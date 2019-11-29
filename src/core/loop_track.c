#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/loop_track.h"
#include "core/audio_buffer.h"

const char *LoopTrackStateStrings[] = {
    "Error", "Stopped", "Armed", "Cued", "Recording", "Concluding", "Playing", "Overdubbing",
};

const char *lt_state_string(LoopTrackState state) {
  return LoopTrackStateStrings[state];
}

LoopTrack *lt_create(uint8_t buffer_id, uint32_t max_length, uint8_t channels) {
  LoopTrack *lt = calloc(1, sizeof(LoopTrack));
  check_mem(lt);

  lt->buffer_id = buffer_id;

  AudioBuffer *buffer = ab_create(max_length, channels);
  check_mem(buffer);
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
      ab_cancel_recording(lt->buffer);
      break;
    case LoopTrack_Action_Record:
      lt->state = LoopTrack_State_Concluding;
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
      ab_stop_playing(lt->buffer);
      lt->state = LoopTrack_State_Stopped;
      break;
    case LoopTrack_Action_Record:
      if (lt->overdubbing_enabled) {
        lt->state = LoopTrack_State_Overdubbing;
      } else {
        lt->state = LoopTrack_State_Armed;
      }
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

uint32_t lt_length(LoopTrack *lc) {
  return lc->buffer->length;
}

uint32_t lt_recordhead_pos(LoopTrack *lc) {
  return lc->buffer->record_head_pos;
}

uint32_t lt_playhead_pos(LoopTrack *lc) {
  return lc->buffer->playback_head_pos;
}

LoopTrackStateChange _lt_handle_concluding(LoopTrack *lt, SyncTimingMessage sync_message, const SAMPLE *input_samples, SAMPLE *output_samples, uint32_t frame_count) {
  if (lt->state != LoopTrack_State_Concluding) {
    return LoopTrack_Change_None;
  }

  if (sync_message.interval == SyncControl_Interval_None) {
    ab_record(lt->buffer, input_samples, frame_count);
    return LoopTrack_Change_None;
  } else {
    ab_record(lt->buffer, input_samples, sync_message.offset);
    ab_finish_recording(lt->buffer);
    ab_playback_mix(lt->buffer, output_samples + sync_message.offset, frame_count - sync_message.offset);
    lt->state = LoopTrack_State_Playing;
    return LoopTrack_Change_Finished_Recording;
  }
}

LoopTrackStateChange lt_handle_audio(LoopTrack *lt, SyncTimingMessage sync_message, const SAMPLE *input_samples, SAMPLE *output_samples, uint32_t frame_count) {
  switch (lt->state) {
    case LoopTrack_State_Error:
      break;
    case LoopTrack_State_Stopped:
      break;
    case LoopTrack_State_Armed:
      if (sync_message.interval == SyncControl_Interval_Whole) {
        ab_record(lt->buffer, input_samples + sync_message.offset, frame_count - sync_message.offset);
        lt->state = LoopTrack_State_Recording;
        return LoopTrack_Change_Started_Recording;
      }
      break;
    case LoopTrack_State_Cued:
      if (sync_message.interval != SyncControl_Interval_None) {
        ab_playback_mix(lt->buffer, output_samples + sync_message.offset, frame_count - sync_message.offset);
        lt->state = LoopTrack_State_Playing;
        return LoopTrack_Change_Started_Playing;
      }
      break;
    case LoopTrack_State_Recording:
      ab_record(lt->buffer, input_samples, frame_count);
      break;
    case LoopTrack_State_Concluding:
      return _lt_handle_concluding(lt, sync_message, input_samples, output_samples, frame_count);
      break;
    case LoopTrack_State_Playing:
      ab_playback_mix(lt->buffer, output_samples, frame_count);
      break;
    case LoopTrack_State_Overdubbing:
      ab_overdub(lt->buffer, input_samples, frame_count);
      break;
    default:
      break;
  }
  return LoopTrack_Change_None;
}

void lt_destroy(LoopTrack *lt) {
  check(lt != NULL, "Invalid Loop Track");
  ab_destroy(lt->buffer);
  free(lt);
  return;
error:
  log_err("Could not clean up Loop Channel");
}
