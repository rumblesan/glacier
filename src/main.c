#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include "portaudio.h"
#include "portmidi.h"
#include "porttime.h"

#include "dbg.h"

#include "core/types.h"
#include "core/config.h"
#include "core/app.h"
#include "core/garbage_collector.h"
#include "core/audio_io.h"
#include "core/ui.h"
#include "core/ui_coms.h"
#include "core/osc_server.h"
#include "core/glacier.h"
#include "core/control_message.h"
#include "core/loop_track.h"
#include "core/audio_bus.h"

void send_midi_action(AppState *app, uint8_t track_id, LoopTrackAction action) {
  ControlMessage *cm = cm_create(track_id, action);
  printf("sending %s control message to track %d\n", lt_action_string(action), track_id);
  if (
    ck_ring_enqueue_spsc(app->control_bus,app->control_bus_buffer, cm) == false
  ) {
    printf("Could not send message to audio thread\n");
  }
}

void process_midi(PtTimestamp timestamp, void *userData) {
  AppState *app = (AppState*)userData;

  if (!app->midi_active) return;
  PmError result;
  PmEvent buffer;
  int status, data1, data2 = 0;

  do {
    result = Pm_Poll(app->midi_in);
    if (result == TRUE) {
      if (Pm_Read(app->midi_in, &buffer, 1) == pmBufferOverflow) 
        continue;
      /* unless there was overflow, we should have a message now */
      status = Pm_MessageStatus(buffer.message);
      data1 = Pm_MessageData1(buffer.message);
      data2 = Pm_MessageData2(buffer.message);
      debug("received MIDI data %d %d %d\n", status, data1, data2);
      if (status >= 0x90 && status <= 0x9F && data2 > 0) {
        switch (data1) {
          case 1:
            send_midi_action(app, 0, LoopTrack_Action_Record);
            break;
          case 6:
            send_midi_action(app, 0, LoopTrack_Action_Playback);
            break;
          case 2:
            send_midi_action(app, 1, LoopTrack_Action_Record);
            break;
          case 7:
            send_midi_action(app, 1, LoopTrack_Action_Playback);
            break;
          case 3:
            send_midi_action(app, 2, LoopTrack_Action_Record);
            break;
          case 8:
            send_midi_action(app, 2, LoopTrack_Action_Playback);
            break;
          default:
            break;
        }
      }
    }
  } while (result == TRUE);

}

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

  if (app->audio_passthrough) {
    for (uint8_t c = 0; c < 2; c++) {
      memcpy(out[c], in[c], framesPerBuffer * sizeof(SAMPLE));
    }
  } else {
    for (uint8_t c = 0; c < 2; c++) {
      for (uint8_t s = 0; s < framesPerBuffer; s++) {
        out[c][s] = 0;
      }
    }
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
    glacier_handle_command(app->glacier, new_control_message);
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
      app->ui_response_bus,
      app->ui_response_bus_buffer,
      query
    );
  }

  return paContinue;
}

void ui_display(AppState *app) {

  UIDisplayData *uuid = ui_display_create(app->glacier->track_count);
  check(
    ck_ring_enqueue_spsc(app->ui_query_bus, app->ui_query_bus_buffer, uuid),
    "Couldn't start UI querying"
  );
  UIDisplayData *query = NULL;

  SDL_Event e;

  while (app->running){
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        app->running = false;
      }
    }

    if (
      ck_ring_dequeue_spsc(
        app->ui_response_bus,
        app->ui_response_bus_buffer,
        &query
      )
    ) {
      ui_draw(app->ui, query);
      ck_ring_enqueue_spsc(
        app->ui_query_bus,
        app->ui_query_bus_buffer,
        query
      );
    }

  }

error:
  return;
}

/*******************************************************************/
int main (int argc, char *argv[]) {
  // Glacier variables

  GlacierCfg *cfg = NULL;

  GarbageCollector *gc = NULL;
  AudioBus *input_bus = NULL;
  AudioIO *audio_io = NULL;
  UIInfo *ui = NULL;
  GlacierAudio *glacier = NULL;
  AppState *app = NULL;
  OSCServer osc_server = NULL;

  uint32_t sample_rate = 48000;
  uint8_t loop_track_count = 3;
  uint32_t record_buffer_length = 30;
  uint8_t record_buffer_channels = 2;

  // Port MIDI variables
  int midiDeviceId;
  const PmDeviceInfo *midiDeviceInfo = NULL;
  PmError portMIDIErr = pmNoError;

  char *config_path = argv[1];

  cfg = cfg_read(config_path);
  check(cfg != NULL, "Could not read config file");

  input_bus = abus_create(cfg->input_bus_channels, cfg->input_bus);

  audio_io = audio_io_create(sample_rate);

  glacier = glacier_create(
    input_bus,
    loop_track_count,
    record_buffer_length * sample_rate,
    record_buffer_channels
  );

  ui = ui_create("Glacier", "arial.ttf", 24);

  app = app_state_create(glacier, ui, cfg);

  osc_server = osc_start_server(app);

  gc = gc_create(app->control_bus_garbage, app->control_bus_garbage_buffer);
  check(gc_start(gc), "Couldn't start garbage collector");

  check(
    audio_io_run(audio_io, audioCB, app),
    "Could not start Audio IO"
  );

  // Start handling MIDI input
  Pt_Start(1, &process_midi, app); /* start a timer with millisecond accuracy */

	Pm_Initialize();

  midiDeviceId = Pm_GetDefaultInputDeviceID();
  midiDeviceInfo = Pm_GetDeviceInfo(midiDeviceId);
  check(midiDeviceInfo != NULL, "Could not open default input device (%d).", midiDeviceId);
  printf("Opening input device %s %s - %d\n", midiDeviceInfo->interf, midiDeviceInfo->name, midiDeviceId);

  portMIDIErr = Pm_OpenInput(
    &app->midi_in,
    midiDeviceId,
    NULL,
    0,
    NULL,
    NULL
  );
  check(portMIDIErr == pmNoError, "Could not open midi input");
  check(app->midi_in != NULL, "Could not open midi input");
  app->midi_active = true;

  // UI blocks main thread
  ui_display(app);

  audio_io_destroy(audio_io);

  gc_destroy(gc);
  osc_stop_server(osc_server);
  app_state_destroy(app);
  glacier_destroy(glacier);
  ui_destroy(ui);
  abus_destroy(input_bus);
  cfg_destroy(cfg);

  TTF_Quit();
  SDL_Quit();

  printf("Finished.");
  return 0;

error:
  if (audio_io != NULL) { audio_io_destroy(audio_io); }
  if (gc != NULL) { gc_destroy(gc); }
  if (osc_server != NULL) { osc_stop_server(osc_server); }
  if (app != NULL) { app_state_destroy(app); }
  if (glacier != NULL) { glacier_destroy(glacier); }
  if (ui != NULL) { ui_destroy(ui); }
  if (input_bus != NULL) { abus_destroy(input_bus); }
  if (cfg != NULL) { cfg_destroy(cfg); }

  if (portMIDIErr != pmNoError) {
    fprintf( stderr, "An error occured while using the portmidi stream\n");
    fprintf( stderr, "Error number: %d\n", portMIDIErr);
    fprintf( stderr, "Error message: %s\n", Pm_GetErrorText(portMIDIErr));
  }

  TTF_Quit();
  SDL_Quit();

  return -1;
}
