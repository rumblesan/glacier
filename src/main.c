#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "portaudio.h"

#include "dbg.h"

#include "core/types.h"
#include "core/app.h"
#include "core/ui.h"
#include "core/ui_coms.h"
#include "core/ui_display.h"
#include "core/osc_server.h"
#include "core/glacier.h"
#include "core/control_message.h"
#include "core/audio_bus.h"
#include "core/loop_track.h"

#define SAMPLE_RATE         (48000)
#define FRAMES_PER_BUFFER   (64)


static int audioCB(
  const void *inputBuffer,
  void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData
) {
  SAMPLE **out = (SAMPLE**)outputBuffer;
  const SAMPLE **in = (const SAMPLE**)inputBuffer;
  AppState *app = (AppState*)userData;

  (void) timeInfo;
  (void) statusFlags;

  for (uint8_t c = 0; c < 2; c++) {
    memcpy(out[c], in[c], framesPerBuffer * sizeof(SAMPLE));
  }

  ControlMessage *new_control_message = NULL;
  UIDisplayData *query = NULL;

  while (
    ck_ring_dequeue_spsc(
      app->control_bus,
      app->control_bus_buffer,
      &new_control_message
    ) == true
  ) {
    glacier_handle_command(
      app->glacier,
      new_control_message
    );
    ck_ring_enqueue_spsc(
      app->control_bus_garbage,
      app->control_bus_garbage_buffer,
      new_control_message
    );
  }
  glacier_handle_audio(app->glacier, in, out, framesPerBuffer);

  if (
    ck_ring_dequeue_spsc(
      app->ui_query_bus,
      app->ui_query_bus_buffer,
      &query
    )
  ) {
    glacier_report(app->glacier, query);
    ck_ring_enqueue_spsc(
      app->ui_query_bus,
      app->ui_query_bus_buffer,
      query
    );
  }

  return paContinue;
}


void *garbage_collector(void *_app) {
  AppState *app = _app;

  struct timespec tim, tim2;
  tim.tv_sec = 5;
  tim.tv_nsec = 0;

  ControlMessage *new_control_message = NULL;

  while(app->running) {
    while (
      ck_ring_dequeue_spsc(
        app->control_bus,
        app->control_bus_buffer,
        &new_control_message
      ) == true
    ) {
      cm_destroy(new_control_message);
    }
    sched_yield();
    nanosleep(&tim, &tim2);
  }
  return NULL;
}

/*******************************************************************/
int main(void) {
  PaStreamParameters inputParameters, outputParameters;
  PaStream *stream;
  PaError err;

  OSCServer osc_server = NULL;

  pthread_t garbage_thread;
  pthread_attr_t garbage_thread_attr;

  err = Pa_Initialize();
  check(err == paNoError, "could not initialize port audio");

  inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
  check (inputParameters.device != paNoDevice, "Error: No default input device.");
  inputParameters.channelCount = 2;       /* stereo input */
  inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  check (outputParameters.device != paNoDevice, "Error: No default output device.")

  outputParameters.channelCount = 2;       /* stereo output */
  outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  uint8_t loop_track_count = 3;
  uint32_t record_buffer_length = 30;
  uint8_t record_buffer_channels = 2;

  AudioBus *input_bus = abus_create(2, 0);

  GlacierAudio *glacier = glacier_create(
    input_bus,
    loop_track_count,
    record_buffer_length * SAMPLE_RATE,
    record_buffer_channels
  );

  UIInfo *ui = ui_create("Glacier", "arial.ttf", 24);

  AppState *app = app_state_create(glacier, ui);

  osc_server = osc_start_server(app);

  debug("Starting garbage collector\n");
  check(!pthread_attr_init(&garbage_thread_attr),
        "Error setting reader thread attributes");
  check(!pthread_attr_setdetachstate(&garbage_thread_attr, PTHREAD_CREATE_DETACHED),
        "Error setting reader thread detach state");
  check(!pthread_create(&garbage_thread,
                        &garbage_thread_attr,
                        &garbage_collector,
                        app),
        "Error creating file reader thread");

  err = Pa_OpenStream(
    &stream,
    &inputParameters,
    &outputParameters,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    0,
    audioCB,
    app
  );
  check(err == paNoError, "could not open stream");

  err = Pa_StartStream( stream );
  check(err == paNoError, "could not start stream");

  ui_display(app);

  err = Pa_CloseStream( stream );
  check(err == paNoError, "could not close stream");

  printf("Finished.");
  osc_stop_server(osc_server);
  Pa_Terminate();
  TTF_Quit();
  SDL_Quit();
  return 0;

error:
  osc_stop_server(osc_server);
  Pa_Terminate();
  TTF_Quit();
  SDL_Quit();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  return -1;
}
