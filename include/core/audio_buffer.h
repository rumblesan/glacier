#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core/types.h"

typedef struct AudioBuffer {
  SAMPLE **buff1_samples;
  SAMPLE **buff2_samples;

  uint8_t active_sample_buffer;

  uint32_t playback_head_pos;
  uint32_t record_head_pos;
  uint32_t length;
  uint32_t max_length;

  uint8_t channels;

  bool overdub;

} AudioBuffer;

AudioBuffer *ab_create(uint32_t max_length, uint8_t channels);

void ab_start_recording(AudioBuffer *ab);

void ab_finish_recording(AudioBuffer *ab);

void ab_cancel_recording(AudioBuffer *ab);

void ab_clear_buffer(AudioBuffer *ab);

void ab_stop_playing(AudioBuffer *ab);

bool ab_toggle_overdubbing(AudioBuffer *ab);

SAMPLE **ab_get_samples(AudioBuffer *ab);

bool ab_record(AudioBuffer *ab, const SAMPLE **input_samples, uint32_t frame_count, uint32_t offset);

void ab_playback_mix(AudioBuffer *ab, SAMPLE **output_samples, uint32_t frame_count, uint32_t offset);

void ab_scrub_playhead(AudioBuffer *ab, uint32_t frame_count);

void ab_tidy(AudioBuffer *ab, uint8_t crossing_count, uint32_t fade_length);

void ab_destroy(AudioBuffer *ab);
