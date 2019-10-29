#ifndef __GLACIER_BUFFER_CONTROL_FSM__
#define __GLACIER_BUFFER_CONTROL_FSM__

#include "audio_buffer.h"

typedef enum audio_buffer_actions {
  AudioBuffer_Action_Playback,
  AudioBuffer_Action_Record,
  AudioBuffer_Action_Sync,
  AudioBuffer_Action_ToggleOverdubbing,
} AudioBufferAction;

typedef enum audio_buffer_state {
  AudioBuffer_State_Error,
  AudioBuffer_State_Stopped,
  AudioBuffer_State_Armed,
  AudioBuffer_State_Cued,
  AudioBuffer_State_Recording,
  AudioBuffer_State_Concluding,
  AudioBuffer_State_Playing,
  AudioBuffer_State_Overdubbing,
} AudioBufferState;

typedef struct AudioBufferControl {

  AudioBuffer *buffer;

  AudioBufferState state;

  bool overdubbing_enabled;

} AudioBufferControl;

AudioBufferControl *abc_create(AudioBuffer *buffer);

AudioBufferState abc_handle_action(AudioBufferControl *abc, AudioBufferAction action);

void abc_destroy(AudioBufferControl *abc);

#endif
