#include <stdlib.h>
#include <assert.h>

#include "dbg.h"

#include "buffer_control.h"

BufferControl *bc_create() {
  BufferControl *bc = malloc(sizeof(BufferControl));
  check_mem(bc);
  bc->cmd = NoCommand;

  return bc;
error:
  return NULL;
}

void bc_start_recording(BufferControl *bc) {
  bc->cmd = StartRecording;
}

void bc_stop_recording(BufferControl *bc) {
  bc->cmd = StopRecording;
}

void bc_destroy(BufferControl *bc) {
  check(bc != NULL, "Invalid Buffer Control");
  free(bc);
  return;
error:
  log_err("Could not clean up Buffer Control");
}
