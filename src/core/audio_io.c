#include <stdlib.h>
#include <stdbool.h>

#include "portaudio.h"

#include "dbg.h"

#include "core/audio_io.h"

void log_pa_error(PaError portAudioErr) {
  log_err("An error occured while using the portaudio stream\n");
  log_err("Error number: %d\n", portAudioErr);
  log_err("Error message: %s\n", Pa_GetErrorText(portAudioErr));
}

AudioIO *audio_io_create(uint32_t sample_rate) {
  PaError portAudioErr = paNoError;

  AudioIO *aio = malloc(sizeof(AudioIO));
  check_mem(aio);

  aio->sample_rate = sample_rate;

  portAudioErr = Pa_Initialize();
  check(portAudioErr == paNoError, "Could not initialize Port Audio");

  aio->inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
  check(aio->inputParameters.device != paNoDevice, "No default input device.");
  aio->inputParameters.channelCount = 2;       /* stereo input */
  aio->inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  aio->inputParameters.suggestedLatency = Pa_GetDeviceInfo( aio->inputParameters.device )->defaultLowInputLatency;
  aio->inputParameters.hostApiSpecificStreamInfo = NULL;

  aio->outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  check(aio->outputParameters.device != paNoDevice, "No default output device.")

  aio->outputParameters.channelCount = 2;       /* stereo output */
  aio->outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  aio->outputParameters.suggestedLatency = Pa_GetDeviceInfo( aio->outputParameters.device )->defaultLowOutputLatency;
  aio->outputParameters.hostApiSpecificStreamInfo = NULL;

  return aio;
error:
  log_pa_error(paNoError);
  return NULL;
}

bool audio_io_run(AudioIO *aio, PaStreamCallback *streamCallback, void *app) {
  PaError portAudioErr = paNoError;
  check(aio != NULL, "Invalid Audio IO");

  portAudioErr = Pa_OpenStream(
    &aio->audio_stream,
    &aio->inputParameters,
    &aio->outputParameters,
    aio->sample_rate,
    64,
    0,
    streamCallback,
    app
  );
  check(portAudioErr == paNoError, "Could not open stream");

  portAudioErr = Pa_StartStream( aio->audio_stream );
  check(portAudioErr == paNoError, "Could not start stream");

  return true;
error:
  if (portAudioErr != paNoError) log_pa_error(portAudioErr);
  return false;
}

void audio_io_destroy(AudioIO *aio) {
  PaError portAudioErr = paNoError;
  check(aio != NULL, "Could not destroy Audio IO");

  portAudioErr = Pa_CloseStream( aio->audio_stream );
  check(portAudioErr == paNoError, "Could not close stream");

  portAudioErr = Pa_Terminate();
  check(portAudioErr == paNoError, "Could not terminate Port Audio");

  free(aio);
  return;
error:
  if (portAudioErr != paNoError) log_pa_error(portAudioErr);
  return;
}
