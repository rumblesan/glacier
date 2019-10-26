#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#include "dbg.h"

#include "types.h"
#include "state.h"
#include "control_message.h"

/*
** Note that many of the older ISA sound cards on PCs do NOT support
** full duplex audio (simultaneous record and playback).
** And some only support full duplex at lower sample rates.
*/
#define SAMPLE_RATE         (48000)
#define PA_SAMPLE_TYPE      paFloat32
#define FRAMES_PER_BUFFER   (64)


static int gNumNoInputs = 0;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/

static int glacierAudioCB(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData ) {
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    GlacierState *gs = (GlacierState*)userData;

    (void) timeInfo;
    (void) statusFlags;

    memcpy(out, in, framesPerBuffer * 2 * sizeof(SAMPLE));

    ControlMessage *new_control_message;
    int buff_num;
    while (ck_ring_dequeue_spsc(
          gs->control_bus,
          gs->control_bus_buffer,
          &new_control_message
          ) == true) {
      buff_num = new_control_message->buffer_number;
      printf("received message for buffer %d\n", buff_num);
      switch (new_control_message->cmd) {
        case StartRecording:
          printf("Starting recording on %d\n", buff_num);
          ab_start_recording(gs->buffers[buff_num]);
          printf("recording status: %d\n", gs->buffers[buff_num]->recording);
          break;
        case StopRecording:
          printf("Stopping recording on %d\n", buff_num);
          ab_stop_recording(gs->buffers[buff_num]);
          printf("recording status: %d\n", gs->buffers[buff_num]->recording);
          break;
        default:
          printf("unknown command: %d %d\n", new_control_message->buffer_number, new_control_message->cmd);
      }
    }
    for (int i = 0; i < gs->buffer_count; i++) {
      if (gs->buffers[i]->recording) {
        ab_record(gs->buffers[i], in, framesPerBuffer);
      } else {
        if (gs->buffers[i]->length > 0) {
          ab_playback_mix(gs->buffers[i], out, framesPerBuffer);
        }
      }
    }

    return paContinue;
}


int app_loop(GlacierState *gs) {
    printf("Hit ENTER to stop program.\n");
    int buffer_num;
    char command;
    while (1) {
      scanf("%d%c", &buffer_num, &command);
      if (buffer_num > gs->buffer_count) {
        printf("%d is not a valid buffer number\n", buffer_num);
        continue;
      }
      switch (command) {
        case 'q':
          printf("quitting\n");
          return 0;
          break;
        case 'r':
          printf("starting recording in buffer %d\n", buffer_num);
          if (
            ck_ring_enqueue_spsc(
              gs->control_bus,
              gs->control_bus_buffer,
              cm_create(buffer_num - 1, StartRecording)
            ) == false
          ) {
            printf("Could not send message to audio thread\n");
          }
          break;
          break;
        case 's':
          printf("stopping recording in buffer %d\n", buffer_num);
          if (
            ck_ring_enqueue_spsc(
              gs->control_bus,
              gs->control_bus_buffer,
              cm_create(buffer_num - 1, StopRecording)
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

    GlacierState *app_state = gs_create(
        record_buffer_count,
        record_buffer_length * SAMPLE_RATE,
        record_buffer_channels);

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              0, /* paClipOff, */  /* we won't output out of range samples so don't bother clipping them */
              glacierAudioCB,
              app_state );
    check(err == paNoError, "could not open stream");

    err = Pa_StartStream( stream );
    check(err == paNoError, "could not start stream");

    app_loop(app_state);

    err = Pa_CloseStream( stream );
    check(err == paNoError, "could not close stream");

    printf("Finished. gNumNoInputs = %d\n", gNumNoInputs );
    Pa_Terminate();
    return 0;

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}
