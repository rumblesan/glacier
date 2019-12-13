#include <stdlib.h>
#include <stdbool.h>

#include "portmidi.h"
#include "porttime.h"

#include "dbg.h"

#include "core/midi_io.h"
#include "core/control_message.h"

void log_pm_error(PmError portMidiErr) {
  log_err("An error occured while using the portmidi stream");
  log_err("Error number: %d", portMidiErr);
  log_err("Error message: %s", Pm_GetErrorText(portMidiErr));
}

MidiIO *midi_io_create(ck_ring_t *com_queue, ck_ring_buffer_t *com_queue_buffer) {
  MidiIO *mio = malloc(sizeof(MidiIO));
  check_mem(mio);

  mio->active = false;

  mio->com_queue = com_queue;
  mio->com_queue_buffer = com_queue_buffer;

  mio->midi_in = NULL;

  return mio;
error:
  log_pm_error(pmNoError);
  return NULL;
}

void send_midi_action(MidiIO *mio, uint8_t track_id, LoopTrackAction action) {
  ControlMessage *cm = cm_create(track_id, action);
  debug("sending %s control message to track %d", lt_action_string(action), track_id);
  if (
    ck_ring_enqueue_spsc(mio->com_queue, mio->com_queue_buffer, cm) == false
  ) {
    log_err("Could not send message to audio thread");
  }
}

void process_midi(PtTimestamp timestamp, void *userData) {
  MidiIO *mio = (MidiIO*)userData;

  if (!mio->active) return;
  PmError result;
  PmEvent buffer;
  int status, data1, data2 = 0;

  uint8_t active_track = 0;

  do {
    result = Pm_Poll(mio->midi_in);
    if (result == TRUE) {
      if (Pm_Read(mio->midi_in, &buffer, 1) == pmBufferOverflow) 
        continue;
      /* unless there was overflow, we should have a message now */
      status = Pm_MessageStatus(buffer.message);
      data1 = Pm_MessageData1(buffer.message);
      data2 = Pm_MessageData2(buffer.message);
      debug("Received MIDI data %02x %02x %02x", status, data1, data2);
      if (status >= 0x90 && status <= 0x9F && data2 > 0) {
        switch (data1) {
          case 1:
            send_midi_action(mio, 0, LoopTrack_Action_Record);
            break;
          case 6:
            send_midi_action(mio, 0, LoopTrack_Action_Playback);
            break;
          case 2:
            send_midi_action(mio, 1, LoopTrack_Action_Record);
            break;
          case 7:
            send_midi_action(mio, 1, LoopTrack_Action_Playback);
            break;
          case 3:
            send_midi_action(mio, 2, LoopTrack_Action_Record);
            break;
          case 8:
            send_midi_action(mio, 2, LoopTrack_Action_Playback);
            break;
          case 11:
            active_track = 0;
            break;
          case 12:
            active_track = 1;
            break;
          case 13:
            active_track = 2;
            break;
          case 14:
            send_midi_action(mio, active_track, LoopTrack_Action_Record);
            break;
          case 15:
            send_midi_action(mio, active_track, LoopTrack_Action_Playback);
            break;
          case 16:
            send_midi_action(mio, active_track, LoopTrack_Action_Clear);
            break;
          case 17:
            send_midi_action(mio, active_track, LoopTrack_Action_ToggleOverdubbing);
            break;
          default:
            break;
        }
      }
    }
  } while (result == TRUE);

}

bool midi_io_run(MidiIO *mio) {
  PmError portMidiErr = pmNoError;

  Pt_Start(1, &process_midi, mio);

	portMidiErr = Pm_Initialize();
  check(portMidiErr == pmNoError, "Could not initialize port midi");

  mio->input_device_id = Pm_GetDefaultInputDeviceID();
  mio->input_device_info = Pm_GetDeviceInfo(mio->input_device_id);
  check(mio->input_device_id != pmNoDevice, "Could not open default input device (%d).", mio->input_device_id);
  log_info(
    "Opening MIDI input device %s %s - %d",
    mio->input_device_info->interf,
    mio->input_device_info->name,
    mio->input_device_id
  );

  portMidiErr = Pm_OpenInput(
    &mio->midi_in,
    mio->input_device_id,
    NULL,
    0,
    NULL,
    NULL
  );
  check(portMidiErr == pmNoError, "Could not open midi input");
  check(mio->midi_in != NULL, "Could not open midi input");
  mio->active = true;

  return true;
error:
  return false;
}

void midi_io_destroy(MidiIO *mio) {
  PtError portTimeErr = ptNoError;
  check(mio != NULL, "Could not destroy Midi IO");
  mio->active = false;

  portTimeErr = Pt_Stop();
  check(portTimeErr == ptNoError, "Could not stop midi clock timer");

  free(mio);
  return;
error:
  return;
}
