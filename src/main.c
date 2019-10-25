#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#include "dbg.h"

#include "types.h"
#include "state.h"

/*
** Note that many of the older ISA sound cards on PCs do NOT support
** full duplex audio (simultaneous record and playback).
** And some only support full duplex at lower sample rates.
*/
#define SAMPLE_RATE         (44100)
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

    memcpy(outputBuffer, inputBuffer, framesPerBuffer * sizeof(SAMPLE));

    for (int i = 0; i < gs->buffer_count; i++) {
      switch (gs->controls[i]->cmd) {
        case StartRecording:
          ab_start_recording(gs->buffers[i]);
          gs->controls[i]->cmd = NoCommand;
          break;
        case StopRecording:
          ab_stop_recording(gs->buffers[i]);
          gs->controls[i]->cmd = NoCommand;
          break;
        default:
          break;
      }
      if (gs->buffers[i]->recording) {
        ab_record(gs->buffers[i], in, framesPerBuffer);
      } else {
        ab_playback_mix(gs->buffers[i], out, framesPerBuffer);
      }
    }

    return paContinue;
}


int input_loop(GlacierState *gs) {
    printf("Hit ENTER to stop program.\n");
    int c;
    while (1) {
      c = getchar();
      switch (c) {
        case 'q':
          printf("quitting\n");
          return 0;
          break;
        case 'r':
          bc_start_recording(gs->controls[0]);
          break;
        case 's':
          bc_stop_recording(gs->controls[0]);
          break;
        default:
          printf("%c\n", c);
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

    int buffer_count = 3;

    GlacierState *app_state = gs_create(buffer_count, 132300, 2);

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

    input_loop(app_state);

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
