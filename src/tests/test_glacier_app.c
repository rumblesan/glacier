#include <stdint.h>

#include "tests/minunit.h"

#include "core/glacier.h"

char *test_glacier_app_create() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  GlacierAppState *glacier = glacier_create(track_count, max_buffer_length, channels);

  glacier_destroy(glacier);

  return NULL;
}

char *test_glacier_startup() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  uint32_t frame_count = 64;
  uint8_t frames_recorded = 0;
  GlacierAppState *glacier = glacier_create(track_count, max_buffer_length, channels);

  const SAMPLE *input_audio = calloc(frame_count * channels, sizeof(SAMPLE));
  SAMPLE *output_audio = calloc(frame_count * channels, sizeof(SAMPLE));

  ControlMessage *cm = cm_create(0, LoopTrack_Action_Record);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  glacier_handle_command(glacier, cm);
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");

  mu_assert(glacier->loop_tracks[0]->state == LoopTrack_State_Armed, "Loop track should be armed");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;
  mu_assert(glacier->loop_tracks[0]->state == LoopTrack_State_Recording, "Loop track should be recording");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  frames_recorded += 1;

  cm->track_number = 0;
  cm->action = LoopTrack_Action_Record;

  glacier_handle_command(glacier, cm);
  mu_assert(glacier->loop_tracks[0]->state == LoopTrack_State_Concluding, "Loop track should be concluding");
  mu_assert(glacier->syncer->state == SyncControl_State_Empty, "Syncer should be empty");
  glacier_handle_audio(glacier, input_audio, output_audio, frame_count);
  mu_assert(glacier->loop_tracks[0]->state == LoopTrack_State_Playing, "Loop track should be playing");

  mu_assert(glacier->syncer->state == SyncControl_State_Running, "Syncer should be running");
  uint32_t expected_total_recorded = frames_recorded * frame_count;
  mu_assert(glacier->syncer->sync_length == expected_total_recorded, "Syncer should have a total length of %d not %d", expected_total_recorded, glacier->syncer->sync_length);
  mu_assert(glacier->buffers[0]->length == expected_total_recorded, "Track 0 buffer should have a total length of %d not %d", expected_total_recorded, glacier->buffers[0]->length);

  cm_destroy(cm);
  glacier_destroy(glacier);

  return NULL;
}

char *test_glacier_app() {
  mu_suite_start();

  mu_run_test(test_glacier_app_create);
  mu_run_test(test_glacier_startup);

  return NULL;
}
