#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/sync_timing_message.h"

const char *SyncStateStrings[] = {
    "Empty", "Running", "Stopped",
};

const char *sc_state_string(SyncControlState state) {
  return SyncStateStrings[state];
}

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
  sm.sync_length = sc->sync_length;
  return sm;
}

void _sc_calculate_sync_lengths(SyncControl *sc, uint32_t recorded_length) {
  sc->sync_length = recorded_length;
  sc->half_length = recorded_length / 2;
  sc->quarter_length = sc->half_length / 2;
  sc->three_quarter_length = sc->half_length + sc->quarter_length;
}

SyncControlState sc_track_finished_recording(SyncControl *sc, LoopTrack *track) {
  switch (sc->state) {
    case SyncControl_State_Empty:
        sc->state = SyncControl_State_Running;
        _sc_calculate_sync_lengths(sc, lt_length(track));
      break;
    case SyncControl_State_Running:
      break;
    case SyncControl_State_Stopped:
      sc->state = SyncControl_State_Running;
      _sc_calculate_sync_lengths(sc, lt_length(track));
      break;
  }
  return sc->state;
}

SyncControlState sc_track_stopped(SyncControl *sc) {
  uint8_t running_loop_tracks = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      log_err("Track shouldn't stop whilst sync control is empty");
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
      break;
    case SyncControl_State_Stopped:
      log_err("Track shouldn't stop whilst sync control is stopped");
      break;
  }
  return sc->state;
}

SyncControlState sc_track_cleared(SyncControl *sc) {
  uint8_t empty_loop_tracks = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      // do nothing
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
      break;
  }
  return sc->state;
}

SyncControlState sc_track_started(SyncControl *sc, LoopTrack *track) {
  switch (sc->state) {
    case SyncControl_State_Empty:
      sc->state = SyncControl_State_Running;
      sc->sync_count = lt_playhead_pos(track);
      break;
    case SyncControl_State_Running:
      // do nothing
      break;
    case SyncControl_State_Stopped:
      sc->state = SyncControl_State_Running;
      _sc_calculate_sync_lengths(sc, lt_length(track));
      break;
  }
  return sc->state;
}

SyncControlState sc_handle_track_change(SyncControl *sc, LoopTrackStateChange track_change, LoopTrack *track) {
  switch (track_change) {
    case LoopTrack_Change_None:
      return sc->state;
    case LoopTrack_Change_Finished_Recording:
      return sc_track_finished_recording(sc, track);
    case LoopTrack_Change_Stopped:
      return sc_track_stopped(sc);
    case LoopTrack_Change_Cleared:
      return sc_track_cleared(sc);
    case LoopTrack_Change_Started_Playing:
      return sc_track_started(sc, track);
    default: return sc->state;
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
