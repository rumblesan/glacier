#pragma once

#include <stdint.h>

#include "core/loop_track.h"

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

  TrackUIDisplay **track_info;

} UIDisplayData;

TrackUIDisplay *track_display_create();

UIDisplayData *ui_display_create(uint8_t track_count);
