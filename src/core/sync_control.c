#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/sync_timing_message.h"

SyncControl *sc_create(LoopTrack **loop_tracks, uint8_t track_count) {
  SyncControl *sc = malloc(sizeof(SyncControl));
  check_mem(sc);

  check_mem(loop_tracks);
  sc->loop_tracks = loop_tracks;
  sc->track_count = track_count;

  sc->sync_count = 0;
  sc->sync_length = 0;
  sc->half_length = 0;
  sc->quarter_length = 0;

  sc->state = SyncControl_State_Empty;

  return sc;
error:
  return NULL;
}

SyncTimingMessage sc_keep_sync(SyncControl *sc, uint32_t count_increase) {
  if (!sc_is_syncing(sc)) {
    // If syncing isn't running then always return a sync message
    SyncTimingMessage sm = { SyncControl_Interval_Whole, 0 };
    return sm;
  }

  SyncTimingMessage sm = { SyncControl_Interval_None, 0 };
  SyncControlInterval interval = SyncControl_Interval_None;
  uint32_t offset = 0;
  uint32_t new_count = sc->sync_count + count_increase;

  if (sc->sync_count < sc->sync_length && new_count >= sc->sync_length) {
    interval = SyncControl_Interval_Whole;
    offset = sc->sync_length - sc->sync_count;
    sc->sync_count = new_count - sc->sync_length;
  } else if (sc->sync_count < sc->three_quarter_length && new_count >= sc->three_quarter_length) {
    interval = SyncControl_Interval_Quarter;
    offset = sc->three_quarter_length - sc->sync_count;
    sc->sync_count = new_count;
  } else if (sc->sync_count < sc->half_length && new_count >= sc->half_length) {
    interval = SyncControl_Interval_Half;
    offset = sc->half_length - sc->sync_count;
    sc->sync_count = new_count;
  } else if (sc->sync_count < sc->quarter_length && new_count >= sc->quarter_length) {
    interval = SyncControl_Interval_Quarter;
    offset = sc->quarter_length - sc->sync_count;
    sc->sync_count = new_count;
  } else {
    sc->sync_count = new_count;
  }

  sm.interval = interval;
  sm.offset = offset;
  return sm;
}

void _sc_calculate_sync_lengths(SyncControl *sc, uint32_t recorded_length) {
  printf("calculating sync lengths from %d\n", recorded_length);
  sc->sync_length = recorded_length;
  sc->half_length = recorded_length / 2;
  sc->quarter_length = sc->half_length / 2;
  sc->three_quarter_length = sc->half_length + sc->quarter_length;
}

SyncControlState sc_handle_track_change(SyncControl *sc, LoopTrackStateChange track_change, LoopTrack *track) {
  if (track_change == LoopTrack_Change_None) { return sc->state; }
  switch (sc->state) {
    case SyncControl_State_Empty:
      sc->state = SyncControl_State_Running;
      _sc_calculate_sync_lengths(sc, lt_recorded_length(track));
    default: return sc->state;
  }
  return sc->state;
}

SyncControlState sc_buffer_stopped(SyncControl *sc) {
  uint8_t running_loop_tracks = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      // TODO shouldn't occur, but maybe check?
      break;
    case SyncControl_State_Running:
      for (uint8_t i = 0; i < sc->track_count; i++) {
        if (lt_is_playing(sc->loop_tracks[i])) {
          running_loop_tracks += 1;
        }
      }
      if (running_loop_tracks == 0) {
        sc->state = SyncControl_State_Stopped;
      }
      // do nothing
      break;
    case SyncControl_State_Stopped:
      // TODO shouldn't occur, but maybe check?
      break;
  }
  return sc->state;
}

SyncControlState sc_buffer_cleared(SyncControl *sc) {
  uint8_t empty_loop_tracks = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      // TODO shouldn't occur, but maybe check?
      break;
    case SyncControl_State_Running:
      for (uint8_t i = 0; i < sc->track_count; i++) {
        if (lt_is_empty(sc->loop_tracks[i])) {
          empty_loop_tracks += 1;
        }
      }
      if (empty_loop_tracks == sc->track_count) {
        sc->state = SyncControl_State_Empty;
      }
      // do nothing
      break;
    case SyncControl_State_Stopped:
      for (uint8_t i = 0; i < sc->track_count; i++) {
        if (lt_is_empty(sc->loop_tracks[i])) {
          empty_loop_tracks += 1;
        }
      }
      if (empty_loop_tracks == sc->track_count) {
        sc->state = SyncControl_State_Empty;
      }
      // do nothing
      break;
  }
  return sc->state;
}

bool sc_is_syncing(SyncControl *sc) {
  return (sc->state == SyncControl_State_Running);
}

void sc_destroy(SyncControl *sc) {
  check(sc != NULL, "Invalid Sync Control");
  free(sc);
  return;
error:
  log_err("Could not clean up Sync Control");
}
