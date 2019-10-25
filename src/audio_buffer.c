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

  ab->max_length = max_length;
  ab->channels = channels;

  ab->recording = 0;

  return ab;
error:
  return NULL;
}

void ab_start_recording(AudioBuffer *ab) {
  ab->recording = 1;
}

void ab_stop_recording(AudioBuffer *ab) {
  ab->length = ab->record_head_pos;
  ab->record_head_pos = 0;
  ab->recording = 0;
}

void ab_record(AudioBuffer *ab, const SAMPLE *input_samples, unsigned long sample_count) {
  if (ab->recording != 1) {
    return;
  }
  unsigned int length;
  if (ab->record_head_pos + sample_count >= ab->max_length) {
    length = ab->max_length - ab->record_head_pos;
    memcpy(ab->samples + ab->record_head_pos, input_samples, length * sizeof(SAMPLE));
    ab_stop_recording(ab);
  } else {
    length = sample_count;
    memcpy(ab->samples + ab->record_head_pos, input_samples, length * sizeof(SAMPLE));
    ab->record_head_pos += sample_count;
  }
}

void ab_playback_mix(AudioBuffer *ab, SAMPLE *output_samples, unsigned long sample_count) {
  if (ab->length <= 0 || ab->recording) { return; }

  unsigned int i;
  unsigned int start = ab->playback_head_pos;
  unsigned int stop = start + sample_count;
  unsigned int pos;
  if (stop > ab->length) {
    for (i = 0; i < sample_count; i++) {
      pos = (i + start) % ab->length;
      output_samples[i] = ab->samples[pos];
    }
  } else {
    for (i = 0; i < sample_count; i++) {
      pos = i + start;
      output_samples[i] = ab->samples[pos];
    }
  }
  ab->playback_head_pos = pos;
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
