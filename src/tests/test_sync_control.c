#include <stdint.h>

#include "tests/minunit.h"

#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"

void _cleanup(SyncControl *sc, uint8_t track_count, LoopTrack **loop_tracks) {
  sc_destroy(sc);

  for (uint8_t i = 0; i < track_count; i++) {
    lt_destroy(loop_tracks[i]);
  }
  free(loop_tracks);
}

void test_sync_control_create() {

  uint8_t track_count = 3;
  LoopTrack **loop_tracks = malloc(sizeof(LoopTrack*) * track_count);
  for (uint8_t i = 0; i < track_count; i++) {
    loop_tracks[i] = lt_create(i, 1024, 2);
  }

  SyncControl *sc = sc_create(loop_tracks, track_count);
  mu_assert(sc != NULL, "Could not create Sync Control");

  mu_assert(sc->state == SyncControl_State_Empty, "Sync Control should start in empty state");

  _cleanup(sc, track_count, loop_tracks);
}

void test_syncing() {
  uint8_t track_count = 3;
  LoopTrack **loop_tracks = malloc(sizeof(LoopTrack*) * track_count);
  for (uint8_t i = 0; i < track_count; i++) {
    loop_tracks[i] = lt_create(i, 1024, 2);
  }

  SyncControl *sc = sc_create(loop_tracks, track_count);
  mu_assert(sc != NULL, "Could not create Sync Control");

  uint32_t recorded_length = 100;
  loop_tracks[0]->buffer->length = recorded_length;
  SyncControlState after_start = sc_handle_track_change(sc, LoopTrack_Change_Finished_Recording, loop_tracks[0]);

  mu_assert(after_start == SyncControl_State_Running, "Sync Control should be running");
  mu_assert(sc->sync_length == recorded_length, "Sync Control should have recorded %d samples not %d", recorded_length, sc->sync_length);

  SyncTimingMessage timing1 = sc_keep_sync(sc, 20);
  mu_assert(timing1.interval == SyncControl_Interval_None, "Sync Control shouldn't sync yet");

  SyncTimingMessage timing2 = sc_keep_sync(sc, 20);
  mu_assert(timing2.interval == SyncControl_Interval_Quarter, "Sync Control should send quarter sync, not %d", timing2.interval);
  mu_assert(timing2.offset == 5, "Sync Control should have an offset of 5");

  SyncTimingMessage timing3 = sc_keep_sync(sc, 25);
  mu_assert(timing3.interval == SyncControl_Interval_Half, "Sync Control should send half sync");
  mu_assert(timing3.offset == 10, "Sync Control should have an offset of 10");

  SyncTimingMessage timing4 = sc_keep_sync(sc, 5);
  mu_assert(timing4.interval == SyncControl_Interval_None, "Sync Control shouldn't sync");
  mu_assert(timing4.offset == 0, "Sync Control should have an offset of 0");

  SyncTimingMessage timing5 = sc_keep_sync(sc, 20);
  mu_assert(timing5.interval == SyncControl_Interval_Quarter, "Sync Control should send quarter sync");
  mu_assert(timing5.offset == 5, "Sync Control should have an offset of 5");

  SyncTimingMessage timing6 = sc_keep_sync(sc, 7);
  mu_assert(timing6.interval == SyncControl_Interval_None, "Sync Control shouldn't sync here");
  mu_assert(timing6.offset == 0, "Sync Control should have an offset of 0");

  SyncTimingMessage timing7 = sc_keep_sync(sc, 20);
  mu_assert(timing7.interval == SyncControl_Interval_Whole, "Sync Control send whole sync");
  mu_assert(timing7.offset == 3, "Sync Control should have an offset of 3");

  _cleanup(sc, track_count, loop_tracks);
}

void test_sync_control() {
  mu_run_test(test_sync_control_create);
  mu_run_test(test_syncing);
}
