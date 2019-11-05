#ifndef __GLACIER_APP__
#define __GLACIER_APP__

#include "ck_ring.h"

#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/control_message.h"

typedef struct GlacierAppState {
  int track_count;
  int channels;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  SyncControl *syncer;

  AudioBuffer **buffers;
  LoopTrack **loop_tracks;

} GlacierAppState;

GlacierAppState *glacier_create(int track_count, unsigned int max_buffer_length, int channels);

void glacier_handle_command(GlacierAppState *glacier, ControlMessage *msg);

void glacier_handle_audio(GlacierAppState *gs, const SAMPLE *input_samples, SAMPLE *output_samples, unsigned long frame_count);

void glacier_destroy(GlacierAppState *glacier);

#endif
