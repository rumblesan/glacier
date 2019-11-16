#include <stdint.h>

#include "tests/minunit.h"

#include "core/audio_buffer.h"
#include "core/types.h"

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

  const SAMPLE *input_audio = calloc(frame_count * channels, sizeof(SAMPLE));

  ab_record(buffer, input_audio, frame_count);
  ab_record(buffer, input_audio, frame_count);
  ab_finish_recording(buffer);

  uint32_t recorded_frames = 40 * 2;
  mu_assert(buffer->length == recorded_frames, "Audio buffer should have recorded %d frames", recorded_frames);

  ab_destroy(buffer);
}

void test_audio_buffer() {
  mu_run_test(test_audio_buffer_create);
  mu_run_test(test_audio_buffer_record);
}
