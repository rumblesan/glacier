#ifndef __GLACIER_AUDIO_BUFFER__
#define __GLACIER_AUDIO_BUFFER__

#include <stdbool.h>

#include "core/types.h"

typedef struct AudioBuffer {
  SAMPLE *samples;

  unsigned int playback_head_pos;
  unsigned int record_head_pos;
  unsigned int length;

  unsigned int max_length;
  unsigned int channels;

} AudioBuffer;

AudioBuffer *ab_create(unsigned int max_length, unsigned int channels);

bool ab_record(AudioBuffer *ab, const SAMPLE *input_samples, unsigned long sample_count);

bool ab_overdub(AudioBuffer *ab, const SAMPLE *input_samples, unsigned long sample_count);

void ab_playback_mix(AudioBuffer *ab, SAMPLE *output_samples, unsigned long sample_count);

void ab_destroy(AudioBuffer *ab);

#endif
