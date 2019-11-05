#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#include "dbg.h"

#include "core/types.h"
#include "core/glacier.h"
#include "core/control_message.h"
#include "core/loop_track.h"

#define SAMPLE_RATE         (48000)
#define PA_SAMPLE_TYPE      paFloat32
#define FRAMES_PER_BUFFER   (64)


static int glacierAudioCB(
  const void *inputBuffer,
  void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData
) {
  SAMPLE *out = (SAMPLE*)outputBuffer;
  const SAMPLE *in = (const SAMPLE*)inputBuffer;
  GlacierAppState *glacier = (GlacierAppState*)userData;

  (void) timeInfo;
  (void) statusFlags;

  memcpy(out, in, framesPerBuffer * 2 * sizeof(SAMPLE));

  ControlMessage *new_control_message = NULL;

  while (
    ck_ring_dequeue_spsc(
      glacier->control_bus,
      glacier->control_bus_buffer,
      &new_control_message
    ) == true
  ) {
    int track_number = new_control_message->track_number;
    LoopTrackAction action = new_control_message->action;
    printf("received message %d for buffer %d\n", action, track_number);
    glacier_handle_command(
      glacier,
      new_control_message
    );
  }
  glacier_handle_audio(glacier, in, out, framesPerBuffer);

  return paContinue;
}


int input_handler(GlacierAppState *glacier) {
  printf("Hit ENTER to stop program.\n");
  int buffer_num;
  char command;
  while (1) {
    scanf("%d%c", &buffer_num, &command);
    if (buffer_num > glacier->track_count) {
      printf("%d is not a valid buffer number\n", buffer_num);
      continue;
    }
    switch (command) {
      case 'q':
        printf("quitting\n");
        return 0;
      case 'r':
        printf("starting recording in buffer %d\n", buffer_num);
        if (
            ck_ring_enqueue_spsc(
              glacier->control_bus,
              glacier->control_bus_buffer,
              cm_create(buffer_num - 1, LoopTrack_Action_Record)
              ) == false
           ) {
          printf("Could not send message to audio thread\n");
        }
        break;
      case 's':
        printf("stopping recording in buffer %d\n", buffer_num);
        if (
            ck_ring_enqueue_spsc(
              glacier->control_bus,
              glacier->control_bus_buffer,
              cm_create(buffer_num - 1, LoopTrack_Action_Playback)
              ) == false
           ) {
          printf("Could not send message to audio thread\n");
        }
        break;
      default:
        printf("%c unknown command\n", command);
    }
  }
}

/*******************************************************************/
int main(void) {
  PaStreamParameters inputParameters, outputParameters;
  PaStream *stream;
  PaError err;

  err = Pa_Initialize();
  check(err == paNoError, "could not initialize port audio");

  inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
  check (inputParameters.device != paNoDevice, "Error: No default input device.");
  inputParameters.channelCount = 2;       /* stereo input */
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  check (outputParameters.device != paNoDevice, "Error: No default output device.")

  outputParameters.channelCount = 2;       /* stereo output */
  outputParameters.sampleFormat = PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  int record_buffer_count = 3;
  int record_buffer_length = 3;
  int record_buffer_channels = 2;

  GlacierAppState *glacier = glacier_create(
    record_buffer_count,
    record_buffer_length * SAMPLE_RATE,
    record_buffer_channels
  );

  err = Pa_OpenStream(
    &stream,
    &inputParameters,
    &outputParameters,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    0,
    glacierAudioCB,
    glacier
  );
  check(err == paNoError, "could not open stream");

  err = Pa_StartStream( stream );
  check(err == paNoError, "could not start stream");

  input_handler(glacier);

  err = Pa_CloseStream( stream );
  check(err == paNoError, "could not close stream");

  printf("Finished.");
  Pa_Terminate();
  return 0;

error:
  Pa_Terminate();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  return -1;
}
