#ifndef __GLACIER_BUFFER_STATE_MACHINE__
#define __GLACIER_BUFFER_STATE_MACHINE__

#include "audio_buffer.h"

typedef enum state_machine_actions {
  RecordAction,
  PlaybackAction
} StateMachineAction;

typedef struct StateMachine {

  AudioBuffer *buffer;

  bool recording;
  bool playing;

  bool overdub;
  bool synced;

} StateMachine;

StateMachine *sm_create(AudioBuffer *buffer);

int sm_handle_action(StateMachine *sm, StateMachineAction action);

void sm_destroy(StateMachine *sm);

#endif
