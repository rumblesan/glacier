#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/loop_track.h"
#include "core/audio_buffer.h"

LoopTrack *lt_create(unsigned int buffer_id, AudioBuffer *buffer) {
  LoopTrack *lt = malloc(sizeof(LoopTrack));
  check_mem(lt);

  lt->buffer_id = buffer_id;

  check(buffer != NULL, "Invalid audio buffer given");
  lt->buffer = buffer;

  lt->state = LoopTrack_State_Stopped;

  lt->overdubbing_enabled = false;

  return lt;
error:
  return NULL;
}

// Internal functions
LoopTrackState _lt_stopped_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      if (lt->buffer->length > 0) {
        lt->state = LoopTrack_State_Cued;
      } else {
        lt->state = LoopTrack_State_Stopped;
      }
      break;
    case LoopTrack_Action_Record:
      lt->state = LoopTrack_State_Armed;
      break;
    case LoopTrack_Action_Sync:
      // Nothing to do
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->overdubbing_enabled = !lt->overdubbing_enabled;
      lt->state = LoopTrack_State_Stopped;
      break;
  }
  return lt->state;
}

LoopTrackState _lt_armed_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      if (lt->buffer->length > 0) {
        lt->state = LoopTrack_State_Stopped;
      } else {
        lt->state = LoopTrack_State_Armed;
      }
      break;
    case LoopTrack_Action_Record:
      // Do nothing
      lt->state = LoopTrack_State_Armed;
      break;
    case LoopTrack_Action_Sync:
      lt->state = LoopTrack_State_Recording;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->state = LoopTrack_State_Armed;
      break;
  }
  return lt->state;
}

LoopTrackState _lt_cued_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_Record:
      // TODO undefined currently
      return LoopTrack_State_Error;
      break;
    case LoopTrack_Action_Sync:
      // TODO going to need some sync timing logic here
      lt->state = LoopTrack_State_Playing;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      break;
  }
  return lt->state;
}

LoopTrackState _lt_recording_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_Record:
      lt->state = LoopTrack_State_Concluding;
      break;
    case LoopTrack_Action_Sync:
      // TODO might need some sync timing logic here
      lt->state = LoopTrack_State_Recording;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->state = LoopTrack_State_Recording;
      break;
  }
  return lt->state;
}

LoopTrackState _lt_concluding_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      // TODO Possibly should stop?
      lt->state = LoopTrack_State_Concluding;
      break;
    case LoopTrack_Action_Record:
      // Do nothing. Covers double presses
      lt->state = LoopTrack_State_Concluding;
      break;
    case LoopTrack_Action_Sync:
      // TODO going to need some sync timing logic here
      lt->state = LoopTrack_State_Playing;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->state = LoopTrack_State_Concluding;
      break;
  }
  return lt->state;
}

LoopTrackState _lt_playing_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      // TODO possibly going to need some sync timing logic here?
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_Record:
      if (lt->overdubbing_enabled) {
        lt->state = LoopTrack_State_Overdubbing;
      } else {
        lt->state = LoopTrack_State_Armed;
      }
      break;
    case LoopTrack_Action_Sync:
      // TODO going to need some sync timing logic here
      lt->state = LoopTrack_State_Playing;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->overdubbing_enabled = !lt->overdubbing_enabled;
      lt->state = LoopTrack_State_Playing;
      break;
  }
  return lt->state;
}

LoopTrackState _lt_overdubbing_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      // TODO possibly going to need some sync timing logic here?
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_Record:
      lt->state = LoopTrack_State_Playing;
      break;
    case LoopTrack_Action_Sync:
      // TODO going to need some sync timing logic here
      lt->state = LoopTrack_State_Overdubbing;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can only change overdubbing when stopped or playing
      lt->state = LoopTrack_State_Overdubbing;
      break;
  }
  return lt->state;
}

LoopTrackState lt_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch (lt->state) {
    case LoopTrack_State_Error:
      return LoopTrack_State_Error;
    case LoopTrack_State_Stopped:
      return _lt_stopped_handle_action(lt, action);
    case LoopTrack_State_Armed:
      return _lt_armed_handle_action(lt, action);
    case LoopTrack_State_Cued:
      return _lt_cued_handle_action(lt, action);
    case LoopTrack_State_Recording:
      return _lt_recording_handle_action(lt, action);
    case LoopTrack_State_Concluding:
      return _lt_concluding_handle_action(lt, action);
    case LoopTrack_State_Playing:
      return _lt_playing_handle_action(lt, action);
    case LoopTrack_State_Overdubbing:
      return _lt_overdubbing_handle_action(lt, action);
  }
}

bool lt_is_playing(LoopTrack *lt) {
  return lt->state == LoopTrack_State_Playing;
}

bool lt_is_empty(LoopTrack *lt) {
  return lt->buffer->length <= 0;
}

void lt_handle_audio(LoopTrack *lt, const SAMPLE *input_samples, unsigned long sample_count) {
  switch (lt->state) {
    case LoopTrack_State_Recording:
      ab_record(lt->buffer, input_samples, sample_count);
      break;
    case LoopTrack_State_Overdubbing:
      ab_overdub(lt->buffer, input_samples, sample_count);
      break;
    default:
      break;
  }
}

void lt_playback_mix(LoopTrack *lt, SAMPLE *output_samples, unsigned long sample_count) {
  if (lt->state != LoopTrack_State_Playing) { return ; }
  ab_playback_mix(lt->buffer, output_samples, sample_count);
}

void lt_destroy(LoopTrack *lt) {
  check(lt != NULL, "Invalid Loop Track");
  ab_destroy(lt->buffer);
  free(lt);
  return;
error:
  log_err("Could not clean up Loop Channel");
}
