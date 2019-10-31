#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "dbg.h"

#include "sync_control.h"
#include "buffer_control_fsm.h"

SyncControl *sc_create(AudioBufferControl **buffers, unsigned int buffer_count) {
  SyncControl *sc = malloc(sizeof(SyncControl));
  check_mem(sc);

  check_mem(buffers);
  sc->buffers = buffers;
  sc->buffer_count = buffer_count;

  sc->sync_count = 0;
  sc->sync_length = 0;
  sc->half_length = 0;
  sc->quarter_length = 0;

  sc->state = SyncControl_State_Empty;

  return sc;
error:
  return NULL;
}

SyncControlState sc_buffer_recorded(SyncControl *sc, unsigned int record_length) {
  switch (sc->state) {
    case SyncControl_State_Empty:
      sc->state = SyncControl_State_Running;
      if (sc->sync_length <= 0) {
        sc->sync_length = record_length;
        sc->half_length = record_length / 2;
        sc->quarter_length = record_length / 4;
        sc->three_quarter_length = sc->quarter_length + sc->half_length;
      }
      break;
    case SyncControl_State_Running:
      // do nothing
      break;
    case SyncControl_State_Stopped:
      // TODO shouldn't occur, but maybe check?
      break;
  }
  return sc->state;
}

SyncControlState sc_buffer_stopped(SyncControl *sc) {
  int running_buffers = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      // TODO shouldn't occur, but maybe check?
      break;
    case SyncControl_State_Running:
      for (int i = 0; i < sc->buffer_count; i++) {
        if (abc_is_playing(sc->buffers[i])) {
          running_buffers += 1;
        }
      }
      if (running_buffers == 0) {
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
  int empty_buffers = 0;
  switch (sc->state) {
    case SyncControl_State_Empty:
      // TODO shouldn't occur, but maybe check?
      break;
    case SyncControl_State_Running:
      for (int i = 0; i < sc->buffer_count; i++) {
        if (abc_is_empty(sc->buffers[i])) {
          empty_buffers += 1;
        }
      }
      if (empty_buffers == sc->buffer_count) {
        sc->state = SyncControl_State_Empty;
      }
      // do nothing
      break;
    case SyncControl_State_Stopped:
      for (int i = 0; i < sc->buffer_count; i++) {
        if (abc_is_empty(sc->buffers[i])) {
          empty_buffers += 1;
        }
      }
      if (empty_buffers == sc->buffer_count) {
        sc->state = SyncControl_State_Empty;
      }
      // do nothing
      break;
  }
  return sc->state;
}

SyncTimingMessage sc_keep_sync(SyncControl *sc, unsigned int count_increase) {
  if (!sc_is_syncing(sc)) {
    // If syncing isn't running then always return a sync message
    SyncTimingMessage sm = { SyncControl_Interval_Whole, 0 };
    return sm;
  }

  SyncTimingMessage sm = { SyncControl_Interval_None, 0 };
  SyncControlInterval interval = SyncControl_Interval_None;
  unsigned int offset = 0;
  unsigned int new_count = sc->sync_count + count_increase;

  if (sc->sync_count < sc->sync_length && new_count >= sc->sync_length) {
    interval = SyncControl_Interval_Whole;
    offset = new_count - sc->sync_length;
    sc->sync_count = offset;
  } else if (sc->sync_count < sc->three_quarter_length && new_count >= sc->three_quarter_length) {
    interval = SyncControl_Interval_Quarter;
    offset = new_count - sc->three_quarter_length;
    sc->sync_count = new_count;
  } else if (sc->sync_count < sc->half_length && new_count >= sc->half_length) {
    interval = SyncControl_Interval_Half;
    offset = new_count - sc->half_length;
    sc->sync_count = new_count;
  } else if (sc->sync_count < sc->quarter_length && new_count >= sc->quarter_length) {
    interval = SyncControl_Interval_Quarter;
    offset = new_count - sc->quarter_length;
    sc->sync_count = new_count;
  } else {
    sc->sync_count = new_count;
  }

  sm.interval = interval;
  sm.offset = offset;
  return sm;
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
