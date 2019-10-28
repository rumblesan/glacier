#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "dbg.h"

#include "state_machine.h"
#include "audio_buffer.h"

StateMachine *sm_create(AudioBuffer *buffer) {
  StateMachine *sm = malloc(sizeof(StateMachine));
  check_mem(sm);

  check(buffer != NULL, "Invalid audio buffer passed");
  sm->buffer = buffer;

  sm->recording = false;
  sm->playing = false;

  sm->overdub = false;
  sm->synced = true;

  return sm;
error:
  return NULL;
}

void sm_destroy(StateMachine *sm) {
  check(sm != NULL, "Invalid Audio Buffer");
  ab_destroy(sm->buffer);
  free(sm);
  return;
error:
  log_err("Could not clean up Audio Buffer");
}
