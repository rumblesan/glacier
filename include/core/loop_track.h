#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core/types.h"
#include "core/sync_timing_message.h"
#include "core/audio_buffer.h"

typedef enum loop_track_action {
  LoopTrack_Action_Playback,
  LoopTrack_Action_Record,
  LoopTrack_Action_Clear,
  LoopTrack_Action_ToggleOverdubbing,
} LoopTrackAction;

typedef enum loop_track_state {
  LoopTrack_State_Error,
  LoopTrack_State_Stopped,
  LoopTrack_State_Armed,
  LoopTrack_State_Cued,
  LoopTrack_State_Recording,
  LoopTrack_State_Concluding,
  LoopTrack_State_Playing,
  LoopTrack_State_Overdubbing,
} LoopTrackState;

typedef enum loop_track_state_change {
  LoopTrack_Change_None,
  LoopTrack_Change_Started_Playing,
  LoopTrack_Change_Started_Recording,
  LoopTrack_Change_Finished_Recording,
  LoopTrack_Change_Stopped,
  LoopTrack_Change_Cleared,
} LoopTrackStateChange;

typedef struct LoopTrack {

  uint8_t buffer_id;

  AudioBuffer *buffer;

  LoopTrackState state;

  bool overdubbing_enabled;

} LoopTrack;

const char *lt_action_string(LoopTrackAction action);

const char *lt_state_string(LoopTrackState state);

LoopTrack *lt_create(uint8_t buffer_id, uint32_t max_length, uint8_t channels);

LoopTrackState lt_handle_action(LoopTrack *lc, LoopTrackAction action);

bool lt_is_playing(LoopTrack *lc);

bool lt_is_empty(LoopTrack *lc);

uint32_t lt_length(LoopTrack *lc);

uint32_t lt_recordhead_pos(LoopTrack *lc);

uint32_t lt_playhead_pos(LoopTrack *lc);

LoopTrackStateChange lt_handle_audio(LoopTrack *lc, SyncTimingMessage sync_message, const SAMPLE **input_samples, SAMPLE **output_samples, uint32_t frame_count);

void lt_destroy(LoopTrack *lc);
