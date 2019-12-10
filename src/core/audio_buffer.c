#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/types.h"
#include "core/audio_buffer.h"

AudioBuffer *ab_create(uint32_t max_length, uint8_t channels) {
  AudioBuffer *ab = calloc(1, sizeof(AudioBuffer));
  check_mem(ab);

  SAMPLE **buff1_samples = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t i = 0; i < channels; i++) {
    buff1_samples[i] = calloc(max_length, sizeof(SAMPLE));
  }
  check_mem(buff1_samples);

  SAMPLE **buff2_samples = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t i = 0; i < channels; i++) {
    buff2_samples[i] = calloc(max_length, sizeof(SAMPLE));
  }
  check_mem(buff2_samples);

  ab->buff1_samples = buff1_samples;
  ab->buff2_samples = buff2_samples;
  ab->active_sample_buffer = 0;

  ab->playback_head_pos = 0;
  ab->record_head_pos = 0;
  ab->length = 0;

  ab->max_length = max_length;
  ab->channels = channels;

  ab->overdub = false;

  return ab;
error:
  return NULL;
}

void ab_finish_recording(AudioBuffer *ab) {
  ab->length = ab->record_head_pos;
  ab->record_head_pos = 0;
  ab->playback_head_pos = 0;
  ab->active_sample_buffer = ab->active_sample_buffer == 1 ? 0 : 1;
}

void ab_cancel_recording(AudioBuffer *ab) {
  // TODO probably need better logic to reset playback position
  ab->record_head_pos = 0;
}

void ab_clear_buffer(AudioBuffer *ab) {
  ab->length = 0;
  ab->record_head_pos = 0;
  ab->playback_head_pos = 0;
}

void ab_stop_playing(AudioBuffer *ab) {
  ab->playback_head_pos = 0;
}

bool ab_toggle_overdubbing(AudioBuffer *ab) {
  ab->overdub = !ab->overdub;
  return ab->overdub;
}

static inline SAMPLE **get_active_buffer(AudioBuffer *ab) {
  return ab->active_sample_buffer == 0 ? ab->buff1_samples : ab->buff2_samples;
}

static inline SAMPLE **get_shadow_buffer(AudioBuffer *ab) {
  return ab->active_sample_buffer == 0 ? ab->buff2_samples : ab->buff1_samples;
}

SAMPLE **ab_get_samples(AudioBuffer *ab) {
  return get_active_buffer(ab);
}

bool ab_record(AudioBuffer *ab, const SAMPLE **input_samples, uint32_t frame_count, uint32_t offset) {

  bool still_recording = true;

  if (ab->record_head_pos + frame_count >= ab->max_length) {
    still_recording = false;
    frame_count = ab->max_length - ab->record_head_pos;
  }

  SAMPLE **active_buffer = get_active_buffer(ab);
  SAMPLE **shadow_buffer = get_shadow_buffer(ab);

  if (ab->overdub) {
    for (uint32_t c = 0; c < ab->channels; c++) {
      for (uint32_t i = 0; i < frame_count; i++) {
        shadow_buffer[c][ab->record_head_pos + i] = input_samples[c][i + offset] + active_buffer[c][ab->record_head_pos + i];
      }
    }
  } else {
    uint32_t bytes = frame_count * sizeof(SAMPLE);
    for (uint32_t c = 0; c < ab->channels; c++) {
      memcpy(shadow_buffer[c] + ab->record_head_pos, input_samples[c] + offset, bytes);
    }
  }
  ab->record_head_pos += frame_count;

  if (!still_recording) {
    ab_finish_recording(ab);
  }

  return still_recording;
}


void ab_playback_mix(AudioBuffer *ab, SAMPLE **output_samples, uint32_t frame_count, uint32_t offset) {
  if (ab->length <= 0) { return; }

  SAMPLE **active_buffer = get_active_buffer(ab);

  if (ab->playback_head_pos + frame_count >= ab->length) {
    uint32_t pos = 0;
    for (uint32_t c = 0; c < ab->channels; c++) {
      for (uint32_t i = 0; i < frame_count; i++) {
        pos = (ab->playback_head_pos + i) % ab->length;
        output_samples[c][i + offset] += active_buffer[c][pos];
      }
    }
    ab->playback_head_pos = (ab->playback_head_pos + frame_count) % ab->length;
  } else {
    for (uint32_t c = 0; c < ab->channels; c++) {
      for (uint32_t i = 0; i < frame_count; i++) {
        output_samples[c][i + offset] += active_buffer[c][ab->playback_head_pos + i];
      }
    }
    ab->playback_head_pos += frame_count;
  }
}

static inline bool zero_crossing(float a, float b) {
    return a*b <= 0.0f;
}

void ab_tidy_zero_crossing(AudioBuffer *ab, uint8_t crossings) {
  SAMPLE **active_buffer = get_active_buffer(ab);

  for (uint8_t c = 0; c < ab->channels; c++) {
    uint8_t crossing_count = 0;
    uint32_t mark = 0;
    // Handle beginning
    for (uint32_t s = 1; s < ab->length; s++) {
      if (zero_crossing(active_buffer[c][s - 1], active_buffer[c][s])) {
        crossing_count += 1;
        if (crossing_count >= crossings) {
          mark = s;
          break;
        }
      }
    }
    for (uint32_t s = 0; s < mark; s++) {
      active_buffer[c][s] = 0.0;
    }

    crossing_count = 0;
    // Handle end
    for (uint32_t s = ab->length; s > 1; s--) {
      if (zero_crossing(active_buffer[c][s - 1], active_buffer[c][s])) {
        crossing_count += 1;
        if (crossing_count >= crossings) {
          mark = s;
          break;
        }
      }
    }
    for (uint32_t s = mark; s < ab->length; s++) {
      active_buffer[c][s] = 0.0;
    }
  }
}

void ab_tidy_fade(AudioBuffer *ab, uint32_t fade_length) {
  SAMPLE **active_buffer = get_active_buffer(ab);

  float delta = 1 / fade_length;
  float mod;
  for (uint8_t c = 0; c < ab->channels; c++) {
    mod = 0;
    for (uint32_t s = 0; s < fade_length; s++) {
      active_buffer[c][s] *= mod;
      mod += delta;
    }
    mod = 1;
    for (uint32_t s = ab->length - fade_length; s < ab->length; s++) {
      active_buffer[c][s] *= mod;
      mod -= delta;
    }
  }
}

void ab_tidy(AudioBuffer *ab, uint8_t crossing_count, uint32_t fade_length) {
  ab_tidy_zero_crossing(ab, crossing_count);
  ab_tidy_fade(ab, fade_length);
}

void ab_destroy(AudioBuffer *ab) {
  check(ab != NULL, "Invalid Audio Buffer");
  check(ab->buff1_samples != NULL, "Invalid samples in Audio Buffer buff1");
  for (uint8_t i = 0; i < ab->channels; i++) {
    free(ab->buff1_samples[i]);
  }
  free(ab->buff1_samples);

  check(ab->buff2_samples != NULL, "Invalid samples in Audio Buffer buff2");
  for (uint8_t i = 0; i < ab->channels; i++) {
    free(ab->buff2_samples[i]);
  }
  free(ab->buff2_samples);
  free(ab);
  return;
error:
  log_err("Could not clean up Audio Buffer");
}
