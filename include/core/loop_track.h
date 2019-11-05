#ifndef __GLACIER_LOOP_TRACK__
#define __GLACIER_LOOP_TRACK__

#include "core/types.h"
#include "core/sync_timing_message.h"
#include "core/audio_buffer.h"

typedef enum loop_track_action {
  LoopTrack_Action_Playback,
  LoopTrack_Action_Record,
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

typedef struct LoopTrack {

  unsigned int buffer_id;

  AudioBuffer *buffer;

  LoopTrackState state;

  bool overdubbing_enabled;

} LoopTrack;

LoopTrack *lt_create(unsigned int buffer_id, AudioBuffer *buffer);

LoopTrackState lt_handle_action(LoopTrack *lc, LoopTrackAction action);

bool lt_is_playing(LoopTrack *lc);

bool lt_is_empty(LoopTrack *lc);

unsigned int lt_recorded_length(LoopTrack *lc);

unsigned int lt_playback_length(LoopTrack *lc);

LoopTrackState lt_handle_audio(LoopTrack *lc, SyncTimingMessage sync_message, const SAMPLE *input_samples, SAMPLE *output_samples, unsigned long frame_count);

void lt_destroy(LoopTrack *lc);

#endif
