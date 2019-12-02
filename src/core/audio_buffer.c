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

  SAMPLE **samples = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t i = 0; i < channels; i++) {
    samples[i] = calloc(max_length, sizeof(SAMPLE));
  }
  check_mem(samples);
  ab->samples = samples;

  ab->playback_head_pos = 0;
  ab->record_head_pos = 0;
  ab->length = 0;

  ab->max_length = max_length;
  ab->channels = channels;

  return ab;
error:
  return NULL;
}

void ab_finish_recording(AudioBuffer *ab) {
  ab->length = ab->record_head_pos;
  ab->record_head_pos = 0;
  ab->playback_head_pos = 0;
}

void ab_cancel_recording(AudioBuffer *ab) {
  ab->length = 0;
  ab->record_head_pos = 0;
  ab->playback_head_pos = 0;
}

void ab_stop_playing(AudioBuffer *ab) {
  ab->playback_head_pos = 0;
}

bool ab_record(AudioBuffer *ab, const SAMPLE **input_samples, uint32_t frame_count, uint32_t offset) {

  bool still_recording = true;

  if (ab->record_head_pos + frame_count >= ab->max_length) {
    still_recording = false;
    frame_count = ab->max_length - ab->record_head_pos;
  }

  uint32_t bytes = frame_count * sizeof(SAMPLE);
  for (uint32_t c = 0; c < ab->channels; c++) {
    memcpy(ab->samples[c] + ab->record_head_pos, input_samples[c] + offset, bytes);
  }
  ab->record_head_pos += frame_count;

  if (!still_recording) {
    ab_finish_recording(ab);
  }

  return still_recording;
}

bool ab_overdub(AudioBuffer *ab, const SAMPLE **input_samples, uint32_t frame_count, uint32_t offset) {

  bool still_recording = true;

  if (ab->record_head_pos + frame_count >= ab->max_length) {
    still_recording = false;
    frame_count = ab->max_length - ab->record_head_pos;
  }

  for (uint32_t c = 0; c < ab->channels; c++) {
    for (uint32_t i = 0; i < frame_count; i++) {
      ab->samples[c][ab->record_head_pos + i] += input_samples[c][i + offset];
    }
  }
  ab->record_head_pos += frame_count;

  return still_recording;
}

void ab_playback_mix(AudioBuffer *ab, SAMPLE **output_samples, uint32_t frame_count, uint32_t offset) {
  if (ab->length <= 0) { return; }

  if (ab->playback_head_pos + frame_count >= ab->length) {
    uint32_t pos = 0;
    for (uint32_t c = 0; c < ab->channels; c++) {
      for (uint32_t i = 0; i < frame_count; i++) {
        pos = (ab->playback_head_pos + i) % ab->length;
        output_samples[c][i + offset] += ab->samples[c][pos];
      }
    }
    ab->playback_head_pos = (ab->playback_head_pos + frame_count) % ab->length;
  } else {
    for (uint32_t c = 0; c < ab->channels; c++) {
      for (uint32_t i = 0; i < frame_count; i++) {
        output_samples[c][i + offset] += ab->samples[c][ab->playback_head_pos + i];
      }
    }
    ab->playback_head_pos += frame_count;
  }
}

void ab_destroy(AudioBuffer *ab) {
  check(ab != NULL, "Invalid Audio Buffer");
  check(ab->samples != NULL, "Invalid samples in Audio Buffer");
  for (uint8_t i = 0; i < ab->channels; i++) {
    free(ab->samples[i]);
  }
  free(ab->samples);
  free(ab);
  return;
error:
  log_err("Could not clean up Audio Buffer");
}
