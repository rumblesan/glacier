#ifndef __GLACIER_STATE__
#define __GLACIER_STATE__

#include "ck_ring.h"

#include "types.h"
#include "audio_buffer.h"

typedef struct GlacierState {
  int buffer_count;
  int channels;

  void *control_bus_buffer;
  ck_ring_t *control_bus;

  AudioBuffer **buffers;

} GlacierState;

GlacierState *gs_create(int buffer_count, int max_buffer_length, int channels);

void gs_destroy(GlacierState *gs);

#endif
