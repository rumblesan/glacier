#include <stdint.h>

#include "tests/minunit.h"

#include "core/glacier.h"

void test_glacier_app_create() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  GlacierAudio *glacier = glacier_create(track_count, max_buffer_length, channels);

  glacier_destroy(glacier);
}

void test_glacier_startup() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  uint32_t frame_count = 64;
  uint8_t frames_recorded = 0;
  GlacierAudio *glacier = glacier_create(track_count, max_buffer_length, channels);

  const SAMPLE *input_audio = calloc(frame_count * channels, sizeof(SAMPLE));
  SAMPLE *output_audio = calloc(frame_count * channels, sizeof(SAMPLE));

  ControlMessage *cm = cm_create(0, LoopTrack_Action_Record);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  glacier_handle_command(glacier, cm);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Armed, "Loop track should be Armed");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Recording, "Loop track should be recording");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;

  cm->track_number = 0;
  cm->action = LoopTrack_Action_Record;

  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Concluding, "Loop track should be concluding");
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Playing, "Loop track should be playing");

  mu_assert(glacier->syncer->state == SyncControl_State_Running, "Syncer should be running");
  uint32_t expected_total_recorded = frames_recorded * frame_count;
  mu_assert(glacier->syncer->sync_length == expected_total_recorded, "Syncer should have a total length of %d not %d", expected_total_recorded, glacier->syncer->sync_length);
  mu_assert(lt_length(glacier_track(glacier, 0)) == expected_total_recorded, "Track 0 should have a total length of %d not %d", expected_total_recorded, lt_length(glacier_track(glacier, 0)));

  cm_destroy(cm);
  glacier_destroy(glacier);
}

void test_glacier_multiple_track_record() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  GlacierAudio *glacier = glacier_create(track_count, max_buffer_length, channels);

  const SAMPLE *input_audio = calloc(3000 * channels, sizeof(SAMPLE));
  SAMPLE *output_audio = calloc(3000 * channels, sizeof(SAMPLE));

  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should still be empty");

  // Trigger recording on track 0
  ControlMessage *cm = cm_create(0, LoopTrack_Action_Record);
  glacier_handle_command(glacier, cm);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should still be empty");
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Armed, "Loop track should be Armed");

  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Recording, "Loop track should be recording");
  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  glacier_handle_audio(glacier, input_audio, output_audio, 100);

  // Trigger stopping recording on track 0
  cm->track_number = 0;
  cm->action = LoopTrack_Action_Record;
  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Concluding, "Loop track should be concluding");
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should still be empty");

  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  mu_assert(glacier_track(glacier, 0)->state == LoopTrack_State_Playing, "Loop track should be playing");
  mu_assert(glacier->syncer->state == SyncControl_State_Running, "Syncer should be running");
  mu_assert(glacier->syncer->sync_length == 300, "Syncer should have a total length of %d not %d", 300, glacier->syncer->sync_length);
  mu_assert(lt_length(glacier_track(glacier, 0)) == 300, "Track 0 should have a total length of %d not %d", 300, lt_length(glacier_track(glacier, 0)));
  mu_assert(glacier->syncer->sync_count == 0, "Syncer should have a count of %d not %d", 0, glacier->syncer->sync_count);

  // Let track 0 play a little
  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  mu_assert(glacier->syncer->sync_count == 100, "Syncer should have a count of %d not %d", 100, glacier->syncer->sync_count);

  // Trigger recording on track 1
  cm->track_number = 1;
  cm->action = LoopTrack_Action_Record;
  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Armed, "Loop track 1 should be Armed");
  glacier_handle_audio(glacier, input_audio, output_audio, 75);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Armed, "Loop track 1 should still be Armed");

  // 25 samples past the sync point
  glacier_handle_audio(glacier, input_audio, output_audio, 150);
  mu_assert(glacier->syncer->sync_count == 25, "Syncer should have a count of %d not %d", 25, glacier->syncer->sync_count);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Recording, "Loop track 1 should be Recording");
  mu_assert(lt_recordhead_pos(glacier_track(glacier, 1)) == 25, "Loop track 1 record head pos should be 25");

  glacier_handle_audio(glacier, input_audio, output_audio, 100);
  mu_assert(glacier->syncer->sync_count == 125, "Syncer should have a count of %d not %d", 125, glacier->syncer->sync_count);
  mu_assert(lt_recordhead_pos(glacier_track(glacier, 1)) == 125, "Loop track 1 record head pos should be 125");

  cm->track_number = 1;
  cm->action = LoopTrack_Action_Record;
  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Concluding, "Loop track 1 should be Concluding");

  glacier_handle_audio(glacier, input_audio, output_audio, 50);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Playing, "Loop track 1 should be Playing");
  mu_assert(lt_playhead_pos(glacier_track(glacier, 1)) == 25, "Loop track 1 playback head pos should be 25");
  mu_assert(lt_length(glacier_track(glacier, 1)) == glacier->syncer->half_length, "Track 1 should have the same length as the syncer");

  glacier_handle_audio(glacier, input_audio, output_audio, 150);
  glacier_handle_audio(glacier, input_audio, output_audio, 100);

  cm->track_number = 1;
  cm->action = LoopTrack_Action_Playback;
  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Stopped, "Loop track 1 should be Stopped");

  glacier_handle_audio(glacier, input_audio, output_audio, 150);

  cm->track_number = 1;
  cm->action = LoopTrack_Action_Playback;
  glacier_handle_command(glacier, cm);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Cued, "Loop track 1 should be Cued");

  glacier_handle_audio(glacier, input_audio, output_audio, 60);
  mu_assert(glacier_track(glacier, 1)->state == LoopTrack_State_Playing, "Loop track 1 should be Playing");
  mu_assert(glacier->syncer->sync_count == 35, "Syncer should have a count of %d not %d", 35, glacier->syncer->sync_count);
  mu_assert(lt_playhead_pos(glacier_track(glacier, 1)) == 35, "Loop track 1 playback head pos should be 35 not %d", lt_playhead_pos(glacier_track(glacier, 1)));
  mu_assert(lt_length(glacier_track(glacier, 1)) == glacier->syncer->half_length, "Track 1 should have the same length as the syncer");

  cm_destroy(cm);
  glacier_destroy(glacier);
}

void test_glacier_app() {
  mu_run_test(test_glacier_app_create);
  mu_run_test(test_glacier_startup);
  mu_run_test(test_glacier_multiple_track_record);
}
