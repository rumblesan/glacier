#pragma once

#include "core/loop_track.h"

typedef struct ControlMessage {
  int track_number;
  LoopTrackAction action;
} ControlMessage;

ControlMessage *cm_create(int track_number, LoopTrackAction action);

void cm_destroy(ControlMessage *cm);
