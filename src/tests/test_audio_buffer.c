#include <stdint.h>
#include <float.h>
#include <math.h>

#include "tests/minunit.h"

#include "core/audio_buffer.h"
#include "core/types.h"

bool almostEqual(float a, float b) {
    return fabs(a - b) <= FLT_EPSILON;
}

void test_audio_buffer_create() {

  uint32_t max_length = 1000;
  uint8_t channels = 2;
  AudioBuffer *buffer = ab_create(max_length, channels);

  mu_assert(buffer != NULL, "Could not create Audio Buffer");
  mu_assert(buffer->max_length == max_length, "Expected max length to be %d not %d", max_length, buffer->max_length);
  mu_assert(buffer->channels == channels, "Should have correct channel number");

  ab_destroy(buffer);
}

void test_audio_buffer_record() {
  uint32_t max_length = 1000;
  uint8_t channels = 2;
  uint32_t frame_count = 40;
  AudioBuffer *buffer = ab_create(max_length, channels);

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE *audio;
  for (uint8_t c = 0; c < channels; c++) {
    audio = calloc(frame_count, sizeof(SAMPLE));
    for (uint32_t s = 0; s < frame_count; s++) {
      audio[s] = 0.75;
    }
    input_audio[c] = audio;
  }

  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  ab_record(buffer, input_audio, frame_count, 0);
  ab_record(buffer, input_audio, frame_count, 0);
  ab_finish_recording(buffer);

  SAMPLE **buffer_samples = ab_get_samples(buffer);
  uint32_t internal_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      if (!almostEqual(buffer_samples[c][s], 0.75)) {
        internal_error_samps += 1;
      }
    }
  }
  mu_assert(internal_error_samps == 0, "All recorded samples should equal 0.75");

  ab_playback_mix(buffer, output_audio, frame_count, 0);
  uint32_t ext_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      if (!almostEqual(output_audio[c][s], 0.75)) {
        ext_error_samps += 1;
      }
    }
  }
  mu_assert(ext_error_samps == 0, "All output samples should equal 0.75");

  uint32_t recorded_frames = 40 * 2;
  mu_assert(buffer->length == recorded_frames, "Audio buffer should have recorded %d frames", recorded_frames);

  ab_destroy(buffer);
  for (uint8_t c = 0; c < channels; c++) {
    free((void *)input_audio[c]);
  }
  free(input_audio);
}


void test_audio_buffer_record_overdub() {
  uint32_t max_length = 1000;
  uint8_t channels = 2;
  uint32_t frame_count = 40;
  AudioBuffer *buffer = ab_create(max_length, channels);

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE *audio;
  for (uint8_t c = 0; c < channels; c++) {
    audio = calloc(frame_count, sizeof(SAMPLE));
    for (uint32_t s = 0; s < frame_count; s++) {
      audio[s] = 0.3;
    }
    input_audio[c] = audio;
  }

  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  ab_record(buffer, input_audio, frame_count, 0);
  ab_record(buffer, input_audio, frame_count, 0);
  ab_finish_recording(buffer);

  ab_toggle_overdubbing(buffer);
  ab_record(buffer, input_audio, frame_count, 0);
  ab_record(buffer, input_audio, frame_count, 0);

  SAMPLE **buffer_samples = ab_get_samples(buffer);
  uint32_t internal_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      if (!almostEqual(buffer_samples[c][s], 0.3)) {
        internal_error_samps += 1;
      }
    }
  }
  mu_assert(internal_error_samps == 0, "All recorded samples should be equal to 0.3");

  ab_playback_mix(buffer, output_audio, frame_count, 0);
  uint32_t ext_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      // Playback should only have the unoverdubbed audio
      if (!almostEqual(output_audio[c][s], 0.3)) {
        ext_error_samps += 1;
      }
    }
  }
  mu_assert(ext_error_samps == 0, "All output samples should be greater than zero");

  uint32_t recorded_frames = 40 * 2;
  mu_assert(buffer->length == recorded_frames, "Audio buffer should have recorded %d frames", recorded_frames);

  ab_destroy(buffer);
  for (uint8_t c = 0; c < channels; c++) {
    free((void *)input_audio[c]);
  }
  free(input_audio);
}

void test_audio_buffer_cancel_recording() {
  uint32_t max_length = 1000;
  uint8_t channels = 2;
  uint32_t frame_count = 40;
  AudioBuffer *buffer = ab_create(max_length, channels);

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE *audio;
  for (uint8_t c = 0; c < channels; c++) {
    audio = calloc(frame_count, sizeof(SAMPLE));
    for (uint32_t s = 0; s < frame_count; s++) {
      audio[s] = 0.75;
    }
    input_audio[c] = audio;
  }

  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  ab_record(buffer, input_audio, frame_count, 0);
  ab_record(buffer, input_audio, frame_count, 0);
  ab_cancel_recording(buffer);

  SAMPLE **buffer_samples = ab_get_samples(buffer);
  uint32_t internal_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      if (!almostEqual(buffer_samples[c][s], 0)) {
        internal_error_samps += 1;
      }
    }
  }
  mu_assert(internal_error_samps == 0, "All recorded samples should be zero");

  ab_playback_mix(buffer, output_audio, frame_count, 0);
  uint32_t ext_error_samps = 0;
  for (uint8_t c = 0; c < channels; c++) {
    for (uint32_t s = 0; s < frame_count; s++) {
      if (!almostEqual(output_audio[c][s], 0)) {
        ext_error_samps += 1;
      }
    }
  }
  mu_assert(ext_error_samps == 0, "All output samples should be zero");

  mu_assert(buffer->length == 0, "Audio buffer should have recorded 0 frames");

  ab_destroy(buffer);
  for (uint8_t c = 0; c < channels; c++) {
    free((void *)input_audio[c]);
  }
  free(input_audio);
}

void test_audio_buffer_clearing() {
  uint32_t max_length = 1000;
  uint8_t channels = 2;
  uint32_t frame_count = 40;
  AudioBuffer *buffer = ab_create(max_length, channels);

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE *audio;
  for (uint8_t c = 0; c < channels; c++) {
    audio = calloc(frame_count, sizeof(SAMPLE));
    for (uint32_t s = 0; s < frame_count; s++) {
      audio[s] = 0.75;
    }
    input_audio[c] = audio;
  }

  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  ab_record(buffer, input_audio, frame_count, 0);
  ab_record(buffer, input_audio, frame_count, 0);
  ab_finish_recording(buffer);
  ab_clear_buffer(buffer);

  mu_assert(buffer->playback_head_pos == 0, "Playback head should be reset");
  mu_assert(buffer->record_head_pos == 0, "Record head should be reset");
  mu_assert(buffer->length == 0, "Length head should be reset");

  ab_destroy(buffer);
  for (uint8_t c = 0; c < channels; c++) {
    free((void *)input_audio[c]);
  }
  free(input_audio);
}

void test_audio_buffer() {
  mu_run_test(test_audio_buffer_create);
  mu_run_test(test_audio_buffer_record);
  mu_run_test(test_audio_buffer_record_overdub);
  mu_run_test(test_audio_buffer_cancel_recording);
  mu_run_test(test_audio_buffer_clearing);
}
