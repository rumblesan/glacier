#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "portaudio.h"

typedef struct AudioIO {
  uint32_t sample_rate;

  PaStreamParameters inputParameters;
  PaStreamParameters outputParameters;

  PaStream *audio_stream;

} AudioIO;

void log_pa_error(PaError portAudioErr);

AudioIO *audio_io_create(uint32_t sample_rate);

bool audio_io_run(AudioIO *aio, PaStreamCallback *streamCallback, void *app);

void audio_io_destroy(AudioIO *aio);
