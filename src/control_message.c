#include <stdlib.h>
#include <assert.h>

#include "dbg.h"

#include "control_message.h"

ControlMessage *cm_create(int buffer_number, enum buffer_command cmd) {
  ControlMessage *cm = malloc(sizeof(ControlMessage));
  check_mem(cm);
  cm->cmd = cmd;

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
