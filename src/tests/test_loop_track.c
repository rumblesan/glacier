#include <stdint.h>

#include "tests/minunit.h"

#include "core/types.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/sync_timing_message.h"

void test_loop_track_create() {

  LoopTrack *loop_track = lt_create(0, 1024, 2);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  mu_assert(loop_track->state == LoopTrack_State_Stopped, "Loop Track should start in empty state");

  lt_destroy(loop_track);
}

void test_loop_track_state_changes() {
  uint8_t channels = 2;
  uint32_t frame_count = 64;

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    input_audio[c] = calloc(frame_count, sizeof(SAMPLE));
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  LoopTrack *loop_track = lt_create(0, 2048, channels);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  lt_handle_action(loop_track, LoopTrack_Action_Record);
  mu_assert(loop_track->state == LoopTrack_State_Armed, "Loop Track should be in Armed state");

  SyncTimingMessage sync_message = {SyncControl_Interval_Whole, 0};

  LoopTrackStateChange state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_Started_Recording, "Loop Track should have started recording");
  mu_assert(loop_track->state == LoopTrack_State_Recording, "Loop Track should be in Recording state");

  // Continue syncing
  sync_message.interval = SyncControl_Interval_None;
  sync_message.offset = 0;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_None, "Loop Track should have no state change");
  mu_assert(loop_track->state == LoopTrack_State_Recording, "Loop Track should be in Recording state");

  sync_message.interval = SyncControl_Interval_Half;
  sync_message.offset = 0;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_None, "Loop Track should have no state change");
  mu_assert(loop_track->state == LoopTrack_State_Recording, "Loop Track should be in Recording state");

  lt_handle_action(loop_track, LoopTrack_Action_Record);
  mu_assert(loop_track->state == LoopTrack_State_Concluding, "Loop Track should be in Concluding state");

  uint32_t sync_offset = 16;
  sync_message.interval = SyncControl_Interval_Whole;
  sync_message.offset = sync_offset;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_Finished_Recording, "Loop Track should have no state change");
  mu_assert(loop_track->state == LoopTrack_State_Playing, "Loop Track should be in Playing state");

  mu_assert(lt_is_playing(loop_track), "Loop Track should be playing");

  uint32_t expected_length = ((3 * frame_count) + sync_offset);
  uint32_t recorded_length = lt_length(loop_track);
  uint32_t expected_playback = (frame_count - sync_offset);
  uint32_t playback_length = lt_playhead_pos(loop_track);
  mu_assert(recorded_length == expected_length, "Should have recorded %d not %d", expected_length, recorded_length);
  mu_assert(playback_length == expected_playback, "Should have played back %d not %d", expected_playback, playback_length);

  lt_handle_action(loop_track, LoopTrack_Action_Playback);
  mu_assert(loop_track->state == LoopTrack_State_Stopped, "Loop Track should be in Stopped state");

  lt_handle_action(loop_track, LoopTrack_Action_Playback);
  mu_assert(loop_track->state == LoopTrack_State_Cued, "Loop Track should be in Cued state");

  sync_message.interval = SyncControl_Interval_None;
  sync_message.offset = 0;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_None, "Loop Track should have no state change");
  mu_assert(loop_track->state == LoopTrack_State_Cued, "Loop Track should be in Cued state");

  sync_message.interval = SyncControl_Interval_Whole;
  sync_message.offset = 0;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_Started_Playing, "Loop Track should have started playing");
  mu_assert(loop_track->state == LoopTrack_State_Playing, "Loop Track should be in Playing state");

  lt_destroy(loop_track);
  free(input_audio);
  free(output_audio);
}

void test_loop_track_cancel_recording() {
  uint8_t channels = 2;
  uint32_t frame_count = 64;

  const SAMPLE **input_audio = calloc(channels, sizeof(SAMPLE *));
  SAMPLE **output_audio = calloc(channels, sizeof(SAMPLE *));
  for (uint8_t c = 0; c < channels; c++) {
    input_audio[c] = calloc(frame_count, sizeof(SAMPLE));
    output_audio[c] = calloc(frame_count, sizeof(SAMPLE));
  }

  LoopTrack *loop_track = lt_create(0, 2048, channels);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  lt_handle_action(loop_track, LoopTrack_Action_Record);

  SyncTimingMessage sync_message = {SyncControl_Interval_Whole, 0};

  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);

  // Continue syncing
  sync_message.interval = SyncControl_Interval_Whole;
  sync_message.offset = 0;
  // record 3 * frame_count samples
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  lt_handle_action(loop_track, LoopTrack_Action_Record);

  // Should now be playing and have played back frame_count samples
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(lt_is_playing(loop_track), "Loop Track should be playing");
  mu_assert(lt_playhead_pos(loop_track) == frame_count, "Loop Track playhead should be at %d", frame_count);

  sync_message.interval = SyncControl_Interval_None;
  sync_message.offset = 0;
  // play a bit more then start recording again
  lt_handle_action(loop_track, LoopTrack_Action_Record);
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);

  sync_message.interval = SyncControl_Interval_Whole;
  sync_message.offset = 0;
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(lt_is_recording(loop_track), "Loop Track should be recording");
  mu_assert(lt_recordhead_pos(loop_track) == frame_count, "Loop Track recordhead should be at %d", frame_count);
  mu_assert(lt_playhead_pos(loop_track) == frame_count, "Loop Track playhead should be at %d not %d", frame_count, lt_playhead_pos(loop_track));

  sync_message.interval = SyncControl_Interval_None;
  sync_message.offset = 0;
  lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  lt_handle_action(loop_track, LoopTrack_Action_Playback);
  mu_assert(!lt_is_recording(loop_track), "Loop Track should not be recording");
  mu_assert(lt_is_playing(loop_track), "Loop Track should be playing");
  mu_assert(lt_recordhead_pos(loop_track) == 0, "Loop Track recordhead should be at %d", 0);
  mu_assert(lt_playhead_pos(loop_track) == 2 * frame_count, "Loop Track playhead should be at %d", 2 * frame_count);


  lt_destroy(loop_track);
  free(input_audio);
  free(output_audio);
}

void test_loop_track() {
  mu_run_test(test_loop_track_create);
  mu_run_test(test_loop_track_state_changes);
  mu_run_test(test_loop_track_cancel_recording);
}
