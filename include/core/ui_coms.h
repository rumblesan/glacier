#pragma once

#include <stdint.h>

#include "core/loop_track.h"
#include "core/sync_control.h"

typedef struct TrackUIDisplay {
  uint8_t track_id;

  LoopTrackState state;

  uint8_t channels;

  uint32_t length;
  uint32_t playback_head_pos;
  uint32_t record_head_pos;

} TrackUIDisplay;

typedef struct UIDisplayData {

  uint8_t track_count;

  SyncControlState sync_state;
  uint32_t sync_length;
  uint32_t sync_pos;

  TrackUIDisplay **track_info;

} UIDisplayData;

TrackUIDisplay *track_display_create();

UIDisplayData *ui_display_create(uint8_t track_count);
