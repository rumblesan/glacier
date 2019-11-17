#pragma once

#include <stdint.h>

#include "ck_ring.h"

#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/control_message.h"

typedef struct GlacierAppState {
  uint8_t track_count;
  uint8_t channels;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  SyncControl *syncer;

  LoopTrack **loop_tracks;

} GlacierAppState;

GlacierAppState *glacier_create(uint8_t track_count, uint32_t max_buffer_length, uint8_t channels);

LoopTrack *glacier_track(GlacierAppState *glacier, uint8_t track_id);

void glacier_handle_command(GlacierAppState *glacier, ControlMessage *msg);

void glacier_handle_audio(GlacierAppState *gs, const SAMPLE *input_samples, SAMPLE *output_samples, uint32_t frame_count);

void glacier_destroy(GlacierAppState *glacier);
