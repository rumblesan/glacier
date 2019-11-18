#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "dbg.h"

#include "core/glacier.h"
#include "core/types.h"
#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"
#include "core/control_message.h"
#include "core/sync_timing_message.h"

GlacierAudio *glacier_create(uint8_t track_count, uint32_t max_buffer_length, uint8_t channels) {

  GlacierAudio *gs = malloc(sizeof(GlacierAudio));
  check_mem(gs);

  gs->track_count = track_count;
  gs->channels = channels;

  gs->loop_tracks = malloc(sizeof(LoopTrack*) * track_count);
  check_mem(gs->loop_tracks);
  for (uint8_t i = 0; i < track_count; i++) {
    gs->loop_tracks[i] = lt_create(i, max_buffer_length, channels);
    check_mem(gs->loop_tracks[i]);
  }

  gs->syncer = sc_create(gs->loop_tracks, track_count);
  check_mem(gs->syncer);

  return gs;
error:
  return NULL;
}

LoopTrack *glacier_track(GlacierAudio *glacier, uint8_t track_id) {
  if (track_id < 0 || track_id > glacier->track_count) { return NULL; }
  return glacier->loop_tracks[track_id];
}

void glacier_handle_command(GlacierAudio *glacier, ControlMessage *msg) {
  uint8_t track_number = msg->track_number;
  if (track_number >= 0 && track_number < glacier->track_count) {
    lt_handle_action(glacier->loop_tracks[track_number], msg->action);
  }
}

void glacier_handle_audio(GlacierAudio *glacier, const SAMPLE *input_samples, SAMPLE *output_samples, uint32_t frame_count) {
  SyncTimingMessage sync_timer = sc_keep_sync(glacier->syncer, frame_count);
  for (uint8_t i = 0; i < glacier->track_count; i++) {
    LoopTrackStateChange state_change = lt_handle_audio(glacier->loop_tracks[i], sync_timer, input_samples, output_samples, frame_count);
    sc_handle_track_change(glacier->syncer, state_change, glacier->loop_tracks[i]);
  }
}

void glacier_destroy(GlacierAudio *gs) {
  check(gs != NULL, "Invalid Glacier State");

  check(gs->loop_tracks != NULL, "Invalid Glacier State buffer control list");
  for (uint8_t i = 0; i < gs->track_count; i++) {
    lt_destroy(gs->loop_tracks[i]);
  }
  free(gs->loop_tracks);

  check(gs->syncer != NULL, "Invalid Glacier State Syncer");
  sc_destroy(gs->syncer);

  free(gs);
  return;
error:
  log_err("Could not clean up Glacier State");
}
