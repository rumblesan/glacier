#ifndef __GLACIER_CONTROL_MESSAGE__
#define __GLACIER_CONTROL_MESSAGE__

#include "core/loop_track.h"

typedef struct ControlMessage {
  int track_number;
  LoopTrackAction action;
} ControlMessage;

ControlMessage *cm_create(int track_number, LoopTrackAction action);

void cm_destroy(ControlMessage *cm);

#endif
