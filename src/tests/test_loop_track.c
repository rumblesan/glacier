#include <stdint.h>

#include "tests/minunit.h"

#include "core/types.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/sync_timing_message.h"

char *test_loop_track_create() {

  AudioBuffer *buffer = ab_create(1024, 2);
  LoopTrack *loop_track = lt_create(0, buffer);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  mu_assert(loop_track->state == LoopTrack_State_Stopped, "Loop Track should start in empty state");

  lt_destroy(loop_track);

  return NULL;
}

char *test_loop_track_state_changes() {
  uint8_t channels = 2;
  uint32_t frame_count = 64;

  SAMPLE *input_audio = calloc(frame_count * channels, sizeof(SAMPLE));
  SAMPLE *output_audio = calloc(frame_count * channels, sizeof(SAMPLE));

  AudioBuffer *buffer = ab_create(2048, channels);
  LoopTrack *loop_track = lt_create(0, buffer);
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
  uint32_t recorded_length = lt_recorded_length(loop_track);
  uint32_t expected_playback = (frame_count - sync_offset);
  uint32_t playback_length = lt_playback_length(loop_track);
  mu_assert(recorded_length == expected_length, "Should have recorded %d not %d", expected_length, recorded_length)
  mu_assert(playback_length == expected_playback, "Should have played back %d not %d", expected_playback, playback_length)

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

  return NULL;
}

char *test_loop_track_cancel_recording() {
  uint8_t channels = 2;
  uint32_t frame_count = 64;

  SAMPLE *input_audio = calloc(frame_count * channels, sizeof(SAMPLE));
  SAMPLE *output_audio = calloc(frame_count * channels, sizeof(SAMPLE));

  AudioBuffer *buffer = ab_create(2048, channels);
  LoopTrack *loop_track = lt_create(0, buffer);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  lt_handle_action(loop_track, LoopTrack_Action_Record);
  mu_assert(loop_track->state == LoopTrack_State_Armed, "Loop Track should be in Armed state");

  SyncTimingMessage sync_message = {SyncControl_Interval_Whole, 0};

  LoopTrackStateChange state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_Started_Recording, "Loop Track should have started Recording");
  mu_assert(loop_track->state == LoopTrack_State_Recording, "Loop Track should be in Recording state");

  // Continue syncing
  sync_message.interval = SyncControl_Interval_None;
  sync_message.offset = 0;
  state_change = lt_handle_audio(loop_track, sync_message, input_audio, output_audio, frame_count);
  mu_assert(state_change == LoopTrack_Change_None, "Loop Track should not have changed state");
  mu_assert(loop_track->state == LoopTrack_State_Recording, "Loop Track should be in Recording state");

  LoopTrackState loop_state = lt_handle_action(loop_track, LoopTrack_Action_Playback);
  mu_assert(loop_state == LoopTrack_State_Stopped, "Loop Track should be in Stopped state");

  mu_assert(!lt_is_playing(loop_track), "Loop Track should not be playing");

  uint32_t expected_length = 0;
  uint32_t recorded_length = lt_recorded_length(loop_track);
  mu_assert(recorded_length == expected_length, "Should have recorded %d not %d", expected_length, recorded_length)

  lt_handle_action(loop_track, LoopTrack_Action_Playback);
  mu_assert(loop_track->state == LoopTrack_State_Stopped, "Loop Track should remain in Stopped state");

  lt_destroy(loop_track);
  free(input_audio);
  free(output_audio);

  return NULL;
}

char *test_loop_track() {
  mu_suite_start();

  mu_run_test(test_loop_track_create);
  mu_run_test(test_loop_track_state_changes);
  mu_run_test(test_loop_track_cancel_recording);

  return NULL;
}
