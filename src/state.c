#include <stdlib.h>
#include <assert.h>

#include "dbg.h"

#include "state.h"
#include "types.h"
#include "buffer_control.h"
#include "audio_buffer.h"

GlacierState *gs_create(int buffer_count, int max_buffer_length, int channels) {

  GlacierState *gs = malloc(sizeof(GlacierState));
  check_mem(gs);

  gs->buffer_count = buffer_count;
  gs->channels = channels;

  gs->controls = malloc(sizeof(BufferControl*) * buffer_count);
  check_mem(gs->controls);
  for (int i = 0; i < buffer_count; i++) {
    gs->controls[i] = bc_create();
    check_mem(gs->controls[i]);
  }

  gs->buffers = malloc(sizeof(AudioBuffer*) * buffer_count);
  check_mem(gs->buffers);
  for (int i = 0; i < buffer_count; i++) {
    gs->buffers[i] = ab_create(max_buffer_length, channels);
    check_mem(gs->buffers[i]);
  }

  return gs;
error:
  return NULL;
}


void gs_destroy(GlacierState *gs) {
  check(gs != NULL, "Invalid Glacier State");
  check(gs->buffers != NULL, "Invalid Glacier State buffer list");
  for (int i = 0; i < gs->buffer_count; i++) {
    ab_destroy(gs->buffers[i]);
  }
  free(gs->buffers);
  free(gs);
  return;
error:
  log_err("Could not clean up Glacier State");
}
