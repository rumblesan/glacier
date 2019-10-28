#include "tests/minunit.h"

#include "state_machine.h"
#include "audio_buffer.h"

char *test_state_machine_create() {

  AudioBuffer *buffer = ab_create(1024, 2);

  StateMachine *sm = sm_create(buffer);
  mu_assert(sm != NULL, "Could not create StateMachine");
  mu_assert(sm->buffer != NULL, "Invalid buffer in StateMachine");

  sm_destroy(sm);
  return NULL;
}

char *test_state_machine() {
  mu_suite_start();

  mu_run_test(test_state_machine_create);

  return NULL;
}
