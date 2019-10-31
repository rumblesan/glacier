#ifndef __GLACIER_STATE__
#define __GLACIER_STATE__

#include "ck_ring.h"

#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"

typedef struct GlacierState {
  int buffer_count;
  int channels;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  SyncControl *syncer;

  AudioBuffer **buffers;
  LoopTrack ** loop_tracks;

} GlacierState;

GlacierState *gs_create(int buffer_count, int max_buffer_length, int channels);

void gs_destroy(GlacierState *gs);

#endif
