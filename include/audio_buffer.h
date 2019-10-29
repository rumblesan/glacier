#ifndef __GLACIER_AUDIO_BUFFER__
#define __GLACIER_AUDIO_BUFFER__

#include <stdbool.h>

#include "types.h"

typedef struct AudioBuffer {
  SAMPLE *samples;

  unsigned int playback_head_pos;
  unsigned int record_head_pos;
  unsigned int length;

  unsigned int max_length;
  unsigned int channels;

  bool recording;
  bool playing;

  bool overdub;
  bool synced;

} AudioBuffer;

AudioBuffer *ab_create(unsigned int max_length, unsigned int channels);

void ab_start_playing(AudioBuffer *ab);

void ab_stop_playing(AudioBuffer *ab);

void ab_start_recording(AudioBuffer *ab);

void ab_stop_recording(AudioBuffer *ab);

void ab_cancel_recording(AudioBuffer *ab);

void ab_record(AudioBuffer *ab, const SAMPLE *input_samples, unsigned long sample_count);

void ab_playback_mix(AudioBuffer *ab, SAMPLE *output_samples, unsigned long sample_count);

void ab_destroy(AudioBuffer *ab);

#endif
