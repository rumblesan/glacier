#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"

#include "core/loop_track.h"
#include "core/audio_buffer.h"

const char *LoopTrackActionStrings[] = {
    "Playback", "Record", "ToggleOverdub",
};

const char *lt_action_string(LoopTrackAction action) {
  return LoopTrackActionStrings[action];
}

const char *LoopTrackStateStrings[] = {
    "Error", "Stopped", "Armed", "Cued", "Recording", "Concluding", "Playing",
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
  lt->previous_state = LoopTrack_State_Stopped;

  return lt;
error:
  return NULL;
}

static inline void _lt_set_state(LoopTrack *lt, LoopTrackState new_state) {
  if (new_state == LoopTrack_State_Stopped) {
    lt->was_playing_last = false;
  } else if (new_state == LoopTrack_State_Playing) {
    lt->was_playing_last = true;
  }
  lt->previous_state = lt->state;
  lt->state = new_state;
}
// Internal functions
LoopTrackState _lt_stopped_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      if (lt->buffer->length > 0) {
        _lt_set_state(lt, LoopTrack_State_Cued);
      } else {
        _lt_set_state(lt, LoopTrack_State_Stopped);
      }
      break;
    case LoopTrack_Action_Record:
      _lt_set_state(lt, LoopTrack_State_Armed);
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      ab_toggle_overdubbing(lt->buffer);
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
  }
  return lt->state;
}

LoopTrackState _lt_armed_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      if (lt->buffer->length > 0) {
        _lt_set_state(lt, LoopTrack_State_Stopped);
      }
      break;
    case LoopTrack_Action_Record:
      // do nothing
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      ab_toggle_overdubbing(lt->buffer);
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
  }
  return lt->state;
}

LoopTrackState _lt_cued_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
    case LoopTrack_Action_Record:
      _lt_set_state(lt, LoopTrack_State_Armed);
      return LoopTrack_State_Error;
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      ab_toggle_overdubbing(lt->buffer);
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
  }
  return lt->state;
}

LoopTrackState _lt_recording_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      ab_cancel_recording(lt->buffer);
      if (lt->was_playing_last) {
        _lt_set_state(lt, LoopTrack_State_Playing);
      } else {
        _lt_set_state(lt, LoopTrack_State_Stopped);
      }
      break;
    case LoopTrack_Action_Record:
      _lt_set_state(lt, LoopTrack_State_Concluding);
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can't change overdubbing whilst already recording
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
  }
  return lt->state;
}

LoopTrackState _lt_concluding_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      if (lt->was_playing_last) {
        _lt_set_state(lt, LoopTrack_State_Playing);
      } else {
        _lt_set_state(lt, LoopTrack_State_Stopped);
      }
      break;
    case LoopTrack_Action_Record:
      // Do nothing. Covers double presses
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      // Can't change overdubbing whilst already recording
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
  }
  return lt->state;
}

LoopTrackState _lt_playing_handle_action(LoopTrack *lt, LoopTrackAction action) {
  switch(action) {
    case LoopTrack_Action_Playback:
      ab_stop_playing(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
      break;
    case LoopTrack_Action_Record:
      _lt_set_state(lt, LoopTrack_State_Armed);
      break;
    case LoopTrack_Action_ToggleOverdubbing:
      ab_toggle_overdubbing(lt->buffer);
      break;
    case LoopTrack_Action_Clear:
      ab_clear_buffer(lt->buffer);
      _lt_set_state(lt, LoopTrack_State_Stopped);
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
  }
}

bool lt_is_playing(LoopTrack *lt) {
  return lt->state == LoopTrack_State_Playing;
}

bool lt_is_recording(LoopTrack *lt) {
  return lt->state == LoopTrack_State_Recording;
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

LoopTrackStateChange _lt_concluding_handle_audio(LoopTrack *lt, SyncTimingMessage sync_message, const SAMPLE **input_samples, SAMPLE **output_samples, uint32_t frame_count) {

  switch (sync_message.interval) {
    case SyncControl_Interval_None:
      ab_record(lt->buffer, input_samples, frame_count, 0);
      if (lt->buffer->overdub) {
        ab_playback_mix(lt->buffer, output_samples, frame_count, 0);
      }
      return LoopTrack_Change_None;
    case SyncControl_Interval_Quarter:
    case SyncControl_Interval_Half:
      // If this buffer is longer than the sync timing then only
      // finish on a whole interval
      if (lt->buffer->record_head_pos > sync_message.sync_length) {
        ab_record(lt->buffer, input_samples, frame_count, 0);
        if (lt->buffer->overdub) {
          ab_playback_mix(lt->buffer, output_samples, frame_count, 0);
        }
        return LoopTrack_Change_None;
      } else {
        ab_record(lt->buffer, input_samples, sync_message.offset, 0);
        if (lt->buffer->overdub) {
          ab_playback_mix(lt->buffer, output_samples, sync_message.offset, 0);
        }
        ab_finish_recording(lt->buffer);
        ab_tidy(lt->buffer, 5, 1000);
        ab_playback_mix(lt->buffer, output_samples, frame_count - sync_message.offset, sync_message.offset);
        _lt_set_state(lt, LoopTrack_State_Playing);
        return LoopTrack_Change_Finished_Recording;
      }
    case SyncControl_Interval_Whole:
      ab_record(lt->buffer, input_samples, sync_message.offset, 0);
      if (lt->buffer->overdub) {
        ab_playback_mix(lt->buffer, output_samples, sync_message.offset, 0);
      }
      ab_finish_recording(lt->buffer);
      ab_tidy(lt->buffer, 5, 1000);
      ab_playback_mix(lt->buffer, output_samples, frame_count - sync_message.offset, sync_message.offset);
      _lt_set_state(lt, LoopTrack_State_Playing);
      return LoopTrack_Change_Finished_Recording;
  }
}

LoopTrackStateChange lt_handle_audio(LoopTrack *lt, SyncTimingMessage sync_message, const SAMPLE **input_samples, SAMPLE **output_samples, uint32_t frame_count) {
  switch (lt->state) {
    case LoopTrack_State_Error:
      break;
    case LoopTrack_State_Stopped:
      break;
    case LoopTrack_State_Recording:
      ab_record(lt->buffer, input_samples, frame_count, 0);
      // if the buffer is overdubbing then continue playing back
      if (lt->buffer->overdub) {
        ab_playback_mix(lt->buffer, output_samples, frame_count, 0);
      } else {
        ab_scrub_playhead(lt->buffer, frame_count);
      }
      break;
    case LoopTrack_State_Concluding:
      return _lt_concluding_handle_audio(lt, sync_message, input_samples, output_samples, frame_count);
      break;
    case LoopTrack_State_Playing:
      ab_playback_mix(lt->buffer, output_samples, frame_count, 0);
      break;
    case LoopTrack_State_Armed:
      if (sync_message.interval == SyncControl_Interval_Whole) {
        ab_playback_mix(lt->buffer, output_samples, sync_message.offset, 0);
        ab_start_recording(lt->buffer);
        ab_record(lt->buffer, input_samples, frame_count - sync_message.offset, sync_message.offset);
        if (lt->buffer->overdub) {
          ab_playback_mix(lt->buffer, output_samples, frame_count - sync_message.offset, sync_message.offset);
        } else {
          ab_scrub_playhead(lt->buffer, frame_count - sync_message.offset);
        }
        _lt_set_state(lt, LoopTrack_State_Recording);
        return LoopTrack_Change_Started_Recording;
      } else {
        ab_playback_mix(lt->buffer, output_samples, frame_count, 0);
      }
      break;
    case LoopTrack_State_Cued:
      if (sync_message.interval == SyncControl_Interval_Whole) {
        ab_playback_mix(lt->buffer, output_samples, frame_count - sync_message.offset, sync_message.offset);
        _lt_set_state(lt, LoopTrack_State_Playing);
        return LoopTrack_Change_Started_Playing;
      }
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
