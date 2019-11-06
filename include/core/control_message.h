#pragma once

#include <stdint.h>

#include "core/loop_track.h"

typedef struct ControlMessage {
  uint8_t track_number;
  LoopTrackAction action;
} ControlMessage;

ControlMessage *cm_create(uint8_t track_number, LoopTrackAction action);

void cm_destroy(ControlMessage *cm);
