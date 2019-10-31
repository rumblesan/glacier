#ifndef __GLACIER_SYNC_CONTROL_FSM__
#define __GLACIER_SYNC_CONTROL_FSM__

#include <stdbool.h>

#include "core/types.h"
#include "core/buffer_control_fsm.h"

typedef enum sync_control_state {
  SyncControl_State_Empty,
  SyncControl_State_Running,
  SyncControl_State_Stopped,
} SyncControlState;

typedef enum sync_control_interval {
  SyncControl_Interval_Whole,
  SyncControl_Interval_Half,
  SyncControl_Interval_Quarter,
  SyncControl_Interval_None,
} SyncControlInterval;

typedef struct SyncTimingMessage {

  SyncControlInterval interval;

  unsigned int offset;

} SyncTimingMessage;

typedef struct SyncControl {

  AudioBufferControl **buffers;

  unsigned int buffer_count;

  unsigned int sync_length;
  unsigned int three_quarter_length;
  unsigned int half_length;
  unsigned int quarter_length;

  unsigned int sync_count;

  SyncControlState state;

} SyncControl;

SyncControl *sc_create(AudioBufferControl **buffers, unsigned int buffer_count);

SyncControlState sc_buffer_recorded(SyncControl *sc, unsigned int record_length);

SyncControlState sc_buffer_stopped(SyncControl *sc);

SyncControlState sc_buffer_cleared(SyncControl *sc);

SyncTimingMessage sc_keep_sync(SyncControl *sc, unsigned int count_increase);

bool sc_is_syncing(SyncControl *sc);

void sc_destroy(SyncControl *abc);

#endif
