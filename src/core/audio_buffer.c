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

  SAMPLE *samples = calloc(max_length * channels, sizeof(SAMPLE));
  check_mem(samples);
  ab->samples = samples;

  ab->playback_head_pos = 0;
  ab->record_head_pos = 0;
  ab->length = 0;

  ab->max_length = max_length * channels;
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

bool ab_record(AudioBuffer *ab, const SAMPLE *input_samples, uint32_t frame_count) {

  uint32_t sample_count = frame_count * ab->channels;
  bool still_recording = true;

  if (ab->record_head_pos + sample_count >= ab->max_length) {
    still_recording = false;
    sample_count = ab->max_length - ab->record_head_pos;
  }

  uint32_t bytes = sample_count * sizeof(SAMPLE);
  memcpy(ab->samples + ab->record_head_pos, input_samples, bytes);
  ab->record_head_pos += sample_count;

  if (!still_recording) {
    ab_finish_recording(ab);
  }

  return still_recording;
}

bool ab_overdub(AudioBuffer *ab, const SAMPLE *input_samples, uint32_t frame_count) {

  uint32_t sample_count = frame_count * ab->channels;
  bool still_recording = true;

  if (ab->record_head_pos + sample_count >= ab->max_length) {
    still_recording = false;
    sample_count = ab->max_length - ab->record_head_pos;
  }

  for (uint32_t i = 0; i < sample_count; i++) {
    ab->samples[ab->record_head_pos + i] += input_samples[i];
  }
  ab->record_head_pos += sample_count;

  return still_recording;
}

void ab_playback_mix(AudioBuffer *ab, SAMPLE *output_samples, uint32_t frame_count) {
  if (ab->length <= 0) { return; }

  uint32_t sample_count = frame_count * ab->channels;

  if (ab->playback_head_pos + sample_count >= ab->length) {
    uint32_t pos = 0;
    for (uint32_t i = 0; i < sample_count; i++) {
      pos = (ab->playback_head_pos + i) % ab->length;
      output_samples[i] += ab->samples[pos];
    }
    ab->playback_head_pos = (ab->playback_head_pos + sample_count) % ab->length;
  } else {
    for (uint32_t i = 0; i < sample_count; i++) {
      output_samples[i] += ab->samples[ab->playback_head_pos + i];
    }
    ab->playback_head_pos += sample_count;
  }
}

void ab_destroy(AudioBuffer *ab) {
  check(ab != NULL, "Invalid Audio Buffer");
  check(ab->samples != NULL, "Invalid samples in Audio Buffer");
  free(ab->samples);
  free(ab);
  return;
error:
  log_err("Could not clean up Audio Buffer");
}
