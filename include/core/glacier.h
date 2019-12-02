#pragma once

#include <stdint.h>

#include "ck_ring.h"

#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/control_message.h"
#include "core/ui_coms.h"

typedef struct GlacierAudio {
  uint8_t track_count;
  uint8_t channels;

  SyncControl *syncer;

  LoopTrack **loop_tracks;

} GlacierAudio;

GlacierAudio *glacier_create(uint8_t track_count, uint32_t max_buffer_length, uint8_t channels);

LoopTrack *glacier_track(GlacierAudio *glacier, uint8_t track_id);

void glacier_handle_command(GlacierAudio *glacier, ControlMessage *msg);

void glacier_handle_audio(GlacierAudio *gs, const SAMPLE **input_samples, SAMPLE **output_samples, uint32_t frame_count);

void glacier_report(GlacierAudio *gs, UIDisplayData *uuid);

void glacier_destroy(GlacierAudio *glacier);
