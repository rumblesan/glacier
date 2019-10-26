#ifndef __GLACIER_STATE__
#define __GLACIER_STATE__

#include "types.h"
#include "buffer_control.h"
#include "audio_buffer.h"
#include "ringbuffer.h"

typedef struct GlacierState {
  int buffer_count;
  int channels;

  RingBuffer *control_bus;

  RingBuffer *garbage_bus;

  BufferControl **controls;
  AudioBuffer **buffers;

} GlacierState;

GlacierState *gs_create(int buffer_count, int max_buffer_length, int channels);

void gs_destroy(GlacierState *gs);

#endif
