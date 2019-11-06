#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core/types.h"
#include "core/loop_track.h"
#include "core/sync_timing_message.h"

typedef enum sync_control_state {
  SyncControl_State_Empty,
  SyncControl_State_Running,
  SyncControl_State_Stopped,
} SyncControlState;

typedef struct SyncControl {

  LoopTrack **loop_tracks;

  uint8_t track_count;

  uint32_t sync_length;
  uint32_t three_quarter_length;
  uint32_t half_length;
  uint32_t quarter_length;

  uint32_t sync_count;

  SyncControlState state;

} SyncControl;

SyncControl *sc_create(LoopTrack **loop_tracks, uint8_t track_count);

SyncControlState sc_check_track_state(SyncControl *sc, LoopTrackState lts, uint32_t record_length);

SyncControlState sc_buffer_recorded(SyncControl *sc, uint32_t record_length);

SyncControlState sc_buffer_stopped(SyncControl *sc);

SyncControlState sc_buffer_cleared(SyncControl *sc);

SyncTimingMessage sc_keep_sync(SyncControl *sc, uint32_t count_increase);

bool sc_is_syncing(SyncControl *sc);

void sc_destroy(SyncControl *abc);
