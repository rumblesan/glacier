#include <stdlib.h>
#include <assert.h>

#include "dbg.h"

#include "audio_buffer.h"
#include "types.h"

AudioBuffer *ab_create(unsigned int max_length, unsigned int channels) {
  AudioBuffer *ab = malloc(sizeof(AudioBuffer));
  check_mem(ab);

  SAMPLE *samples = calloc(max_length * channels, sizeof(SAMPLE *));
  check_mem(samples);
  ab->samples = samples;

  ab->playback_head_pos = 0;
  ab->record_head_pos = 0;
  ab->length = 0;

  ab->max_length = max_length * channels;
  ab->channels = channels;

  ab->recording = 0;

  return ab;
error:
  return NULL;
}

void ab_start_recording(AudioBuffer *ab) {
  if (!ab->recording) {
    ab->recording = 1;
  }
}

void ab_stop_recording(AudioBuffer *ab) {
  if (ab->recording) {
    ab->length = ab->record_head_pos;
    ab->record_head_pos = 0;
    ab->recording = 0;
  }
}

void ab_record(AudioBuffer *ab, const SAMPLE *input_samples, unsigned long frame_count) {
  if (ab->recording != 1) {
    return;
  }
  int sample_count = frame_count * ab->channels;
  int stop_recording = 0;

  if (ab->record_head_pos + sample_count >= ab->max_length) {
    stop_recording = 1;
    sample_count = ab->max_length - ab->record_head_pos;
  }

  int bytes = sample_count * sizeof(SAMPLE);
  memcpy(ab->samples + ab->record_head_pos, input_samples, bytes);
  ab->record_head_pos += sample_count;

  if (stop_recording) {
    ab_stop_recording(ab);
  }
}

void ab_playback_mix(AudioBuffer *ab, SAMPLE *output_samples, unsigned long frame_count) {
  if (ab->length <= 0 || ab->recording) { return; }

  int sample_count = frame_count * ab->channels;

  if (ab->playback_head_pos + sample_count >= ab->length) {
    int pos = 0;
    for (int i = 0; i < sample_count; i++) {
      pos = (ab->playback_head_pos + i) % ab->length;
      output_samples[i] += ab->samples[pos];
    }
    ab->playback_head_pos = (ab->playback_head_pos + sample_count) % ab->length;
  } else {
    for (int i = 0; i < sample_count; i++) {
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
