#include <stdlib.h>
#include <assert.h>
#include "ck_ring.h"

#include "dbg.h"

#include "core/state.h"
#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"

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
  gs->loop_tracks = malloc(sizeof(LoopTrack*) * buffer_count);
  check_mem(gs->loop_tracks);
  for (int i = 0; i < buffer_count; i++) {
    gs->loop_tracks[i] = lt_create(i, gs->buffers[i]);
    check_mem(gs->loop_tracks[i]);
  }

  gs->syncer = sc_create(gs->loop_tracks, buffer_count);
  check_mem(gs->syncer);

  return gs;
error:
  return NULL;
}


void gs_destroy(GlacierState *gs) {
  check(gs != NULL, "Invalid Glacier State");

  check(gs->loop_tracks != NULL, "Invalid Glacier State buffer control list");
  for (int i = 0; i < gs->buffer_count; i++) {
    lt_destroy(gs->loop_tracks[i]);
  }
  free(gs->loop_tracks);
  // buffers are free when buffer control is destroyed
  free(gs->buffers);

  check(gs->syncer != NULL, "Invalid Glacier State Syncer");
  sc_destroy(gs->syncer);

  free(gs);
  return;
error:
  log_err("Could not clean up Glacier State");
}
