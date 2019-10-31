#include <stdlib.h>
#include <assert.h>

#include "dbg.h"
#include "ck_ring.h"

#include "state.h"
#include "types.h"
#include "sync_control.h"
#include "buffer_control_fsm.h"
#include "audio_buffer.h"

GlacierState *gs_create(int buffer_count, int max_buffer_length, int channels) {

  GlacierState *gs = malloc(sizeof(GlacierState));
  check_mem(gs);

  gs->buffer_count = buffer_count;
  gs->channels = channels;

  int control_bus_size = 1024;
  gs->control_bus = malloc(sizeof(ck_ring_t));
  check_mem(gs->control_bus);

  gs->control_bus_buffer = malloc(
    sizeof(ck_ring_buffer_t) * control_bus_size
  );
  check_mem(gs->control_bus_buffer);

  ck_ring_init(gs->control_bus, control_bus_size);

  gs->buffers = malloc(sizeof(AudioBuffer*) * buffer_count);
  check_mem(gs->buffers);
  for (int i = 0; i < buffer_count; i++) {
    gs->buffers[i] = ab_create(max_buffer_length, channels);
    check_mem(gs->buffers[i]);
  }
  gs->buffer_controls = malloc(sizeof(AudioBufferControl*) * buffer_count);
  check_mem(gs->buffer_controls);
  for (int i = 0; i < buffer_count; i++) {
    gs->buffer_controls[i] = abc_create(i, gs->buffers[i]);
    check_mem(gs->buffer_controls[i]);
  }

  gs->syncer = sc_create(gs->buffer_controls, buffer_count);
  check_mem(gs->syncer);

  return gs;
error:
  return NULL;
}


void gs_destroy(GlacierState *gs) {
  check(gs != NULL, "Invalid Glacier State");

  check(gs->buffer_controls != NULL, "Invalid Glacier State buffer control list");
  for (int i = 0; i < gs->buffer_count; i++) {
    abc_destroy(gs->buffer_controls[i]);
  }
  free(gs->buffer_controls);
  // buffers are free when buffer control is destroyed
  free(gs->buffers);

  check(gs->syncer != NULL, "Invalid Glacier State Syncer");
  sc_destroy(gs->syncer);

  free(gs);
  return;
error:
  log_err("Could not clean up Glacier State");
}
