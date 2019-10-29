#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "dbg.h"

#include "buffer_control_fsm.h"
#include "audio_buffer.h"

AudioBufferControl *abc_create(AudioBuffer *buffer) {
  AudioBufferControl *abc = malloc(sizeof(AudioBufferControl));
  check_mem(abc);

  check(buffer != NULL, "Invalid audio buffer given");
  abc->buffer = buffer;

  abc->state = AudioBuffer_State_Stopped;

  abc->overdubbing_enabled = false;

  return abc;
error:
  return NULL;
}

// Internal functions
AudioBufferState _abc_stopped_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      if (abc->buffer->length > 0) {
        abc->state = AudioBuffer_State_Cued;
      } else {
        abc->state = AudioBuffer_State_Stopped;
      }
      break;
    case AudioBuffer_Action_Record:
      abc->state = AudioBuffer_State_Armed;
      break;
    case AudioBuffer_Action_Sync:
      // Nothing to do
      abc->state = AudioBuffer_State_Stopped;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->overdubbing_enabled = !abc->overdubbing_enabled;
      abc->state = AudioBuffer_State_Stopped;
      break;
  }
  return abc->state;
}

AudioBufferState _abc_armed_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      if (abc->buffer->length > 0) {
        abc->state = AudioBuffer_State_Stopped;
      } else {
        abc->state = AudioBuffer_State_Armed;
      }
      break;
    case AudioBuffer_Action_Record:
      // Do nothing
      abc->state = AudioBuffer_State_Armed;
      break;
    case AudioBuffer_Action_Sync:
      ab_start_recording(abc->buffer);
      abc->state = AudioBuffer_State_Recording;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->state = AudioBuffer_State_Armed;
      break;
  }
  return abc->state;
}

AudioBufferState _abc_cued_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      abc->state = AudioBuffer_State_Stopped;
      break;
    case AudioBuffer_Action_Record:
      // TODO undefined currently
      return AudioBuffer_State_Error;
      break;
    case AudioBuffer_Action_Sync:
      // TODO going to need some sync timing logic here
      ab_start_playing(abc->buffer);
      abc->state = AudioBuffer_State_Playing;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      break;
  }
  return abc->state;
}

AudioBufferState _abc_recording_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      ab_cancel_recording(abc->buffer);
      abc->state = AudioBuffer_State_Stopped;
      break;
    case AudioBuffer_Action_Record:
      abc->state = AudioBuffer_State_Concluding;
      break;
    case AudioBuffer_Action_Sync:
      // TODO might need some sync timing logic here
      abc->state = AudioBuffer_State_Recording;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->state = AudioBuffer_State_Recording;
      break;
  }
  return abc->state;
}

AudioBufferState _abc_concluding_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      // TODO Possibly should stop?
      abc->state = AudioBuffer_State_Concluding;
      break;
    case AudioBuffer_Action_Record:
      // Do nothing. Covers double presses
      abc->state = AudioBuffer_State_Concluding;
      break;
    case AudioBuffer_Action_Sync:
      // TODO going to need some sync timing logic here
      ab_stop_recording(abc->buffer);
      ab_start_playing(abc->buffer);
      abc->state = AudioBuffer_State_Playing;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->state = AudioBuffer_State_Concluding;
      break;
  }
  return abc->state;
}

AudioBufferState _abc_playing_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      // TODO possibly going to need some sync timing logic here?
      ab_stop_playing(abc->buffer);
      abc->state = AudioBuffer_State_Stopped;
      break;
    case AudioBuffer_Action_Record:
      if (abc->overdubbing_enabled) {
        abc->buffer->overdub = true;
        abc->state = AudioBuffer_State_Overdubbing;
      } else {
        abc->state = AudioBuffer_State_Armed;
      }
      break;
    case AudioBuffer_Action_Sync:
      // TODO going to need some sync timing logic here
      abc->state = AudioBuffer_State_Playing;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->overdubbing_enabled = !abc->overdubbing_enabled;
      abc->state = AudioBuffer_State_Playing;
      break;
  }
  return abc->state;
}

AudioBufferState _abc_overdubbing_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch(action) {
    case AudioBuffer_Action_Playback:
      // TODO possibly going to need some sync timing logic here?
      ab_stop_playing(abc->buffer);
      abc->state = AudioBuffer_State_Stopped;
      break;
    case AudioBuffer_Action_Record:
      abc->buffer->overdub = false;
      abc->state = AudioBuffer_State_Playing;
      break;
    case AudioBuffer_Action_Sync:
      // TODO going to need some sync timing logic here
      abc->state = AudioBuffer_State_Overdubbing;
      break;
    case AudioBuffer_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      abc->state = AudioBuffer_State_Overdubbing;
      break;
  }
  return abc->state;
}

AudioBufferState abc_handle_action(AudioBufferControl *abc, AudioBufferAction action) {
  switch (abc->state) {
    case AudioBuffer_State_Error:
      return AudioBuffer_State_Error;
    case AudioBuffer_State_Stopped:
      return _abc_stopped_handle_action(abc, action);
    case AudioBuffer_State_Armed:
      return _abc_armed_handle_action(abc, action);
    case AudioBuffer_State_Cued:
      return _abc_cued_handle_action(abc, action);
    case AudioBuffer_State_Recording:
      return _abc_recording_handle_action(abc, action);
    case AudioBuffer_State_Concluding:
      return _abc_concluding_handle_action(abc, action);
    case AudioBuffer_State_Playing:
      return _abc_playing_handle_action(abc, action);
    case AudioBuffer_State_Overdubbing:
      return _abc_overdubbing_handle_action(abc, action);
  }
}

void abc_destroy(AudioBufferControl *abc) {
  check(abc != NULL, "Invalid Audio Buffer Control");
  ab_destroy(abc->buffer);
  free(abc);
  return;
error:
  log_err("Could not clean up Audio Buffer Control");
}
