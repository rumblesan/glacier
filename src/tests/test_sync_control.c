#include "tests/minunit.h"

#include "sync_control.h"
#include "buffer_control_fsm.h"
#include "audio_buffer.h"

char *test_sync_control_create() {

  int buffer_count = 3;
  AudioBuffer **buffers = malloc(sizeof(AudioBuffer*) * buffer_count);
  for (int i = 0; i < buffer_count; i++) {
    buffers[i] = ab_create(1024, 2);
  }

  AudioBufferControl **buffer_controls = malloc(sizeof(AudioBufferControl*) * buffer_count);
  for (int i = 0; i < buffer_count; i++) {
    buffer_controls[i] = abc_create(i, buffers[i]);
  }

  SyncControl *sc = sc_create(buffer_controls, buffer_count);
  mu_assert(sc != NULL, "Could not create Sync Control");

  mu_assert(sc->state == SyncControl_State_Empty, "Sync Control should start in empty state");

  sc_destroy(sc);

  for (int i = 0; i < buffer_count; i++) {
    abc_destroy(buffer_controls[i]);
  }
  free(buffer_controls);

  // buffers are free when buffer control is destroyed
  free(buffers);

  return NULL;
}

char *test_sync_control() {
  mu_suite_start();

  mu_run_test(test_sync_control_create);

  return NULL;
}
