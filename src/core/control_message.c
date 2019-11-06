#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "dbg.h"

#include "core/control_message.h"
#include "core/loop_track.h"

ControlMessage *cm_create(uint8_t track_number, LoopTrackAction action) {
  ControlMessage *cm = malloc(sizeof(ControlMessage));
  check_mem(cm);
  cm->action = action;
  cm->track_number = track_number;

  return cm;
error:
  return NULL;
}

void cm_destroy(ControlMessage *cm) {
  check(cm != NULL, "Invalid Control Message");
  free(cm);
  return;
error:
  log_err("Could not clean up Control Message");
}
