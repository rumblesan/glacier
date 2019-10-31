#include "tests/minunit.h"

#include "core/sync_control.h"
#include "core/loop_track.h"
#include "core/audio_buffer.h"

void _cleanup(SyncControl *sc, int track_count, AudioBuffer **buffers, LoopTrack **loop_tracks) {
  sc_destroy(sc);

  for (int i = 0; i < track_count; i++) {
    lt_destroy(loop_tracks[i]);
  }
  free(loop_tracks);

  // buffers are free when buffer control is destroyed
  free(buffers);

}

char *test_sync_control_create() {

  int track_count = 3;
  AudioBuffer **buffers = malloc(sizeof(AudioBuffer*) * track_count);
  for (int i = 0; i < track_count; i++) {
    buffers[i] = ab_create(1024, 2);
  }

  LoopTrack **loop_tracks = malloc(sizeof(LoopTrack*) * track_count);
  for (int i = 0; i < track_count; i++) {
    loop_tracks[i] = lt_create(i, buffers[i]);
  }

  SyncControl *sc = sc_create(loop_tracks, track_count);
  mu_assert(sc != NULL, "Could not create Sync Control");

  mu_assert(sc->state == SyncControl_State_Empty, "Sync Control should start in empty state");

  _cleanup(sc, track_count, buffers, loop_tracks);

  return NULL;
}

char *test_syncing() {
  int track_count = 3;
  AudioBuffer **buffers = malloc(sizeof(AudioBuffer*) * track_count);
  for (int i = 0; i < track_count; i++) {
    buffers[i] = ab_create(1024, 2);
  }

  LoopTrack **loop_tracks = malloc(sizeof(LoopTrack*) * track_count);
  for (int i = 0; i < track_count; i++) {
    loop_tracks[i] = lt_create(i, buffers[i]);
  }

  SyncControl *sc = sc_create(loop_tracks, track_count);
  mu_assert(sc != NULL, "Could not create Sync Control");


  unsigned int recorded_length = 100;
  buffers[0]->length = recorded_length;
  SyncControlState after_start = sc_buffer_recorded(sc, recorded_length);
  mu_assert(after_start == SyncControl_State_Running, "Sync Control should be running");

  SyncTimingMessage timing1 = sc_keep_sync(sc, 20);
  mu_assert(timing1.interval == SyncControl_Interval_None, "Sync Control shouldn't sync yet");

  SyncTimingMessage timing2 = sc_keep_sync(sc, 20);
  mu_assert(timing2.interval == SyncControl_Interval_Quarter, "Sync Control should send quarter sync");
  mu_assert(timing2.offset == 15, "Sync Control should have an offset of 15");

  SyncTimingMessage timing3 = sc_keep_sync(sc, 25);
  mu_assert(timing3.interval == SyncControl_Interval_Half, "Sync Control should send half sync");
  mu_assert(timing3.offset == 15, "Sync Control should have an offset of 15");

  SyncTimingMessage timing4 = sc_keep_sync(sc, 5);
  mu_assert(timing4.interval == SyncControl_Interval_None, "Sync Control shouldn't sync");
  mu_assert(timing4.offset == 0, "Sync Control should have an offset of 0");

  SyncTimingMessage timing5 = sc_keep_sync(sc, 20);
  mu_assert(timing5.interval == SyncControl_Interval_Quarter, "Sync Control should send quarter sync");
  mu_assert(timing5.offset == 15, "Sync Control should have an offset of 15");

  SyncTimingMessage timing6 = sc_keep_sync(sc, 7);
  mu_assert(timing6.interval == SyncControl_Interval_None, "Sync Control shouldn't sync here");
  mu_assert(timing6.offset == 0, "Sync Control should have an offset of 0");

  SyncTimingMessage timing7 = sc_keep_sync(sc, 20);
  mu_assert(timing7.interval == SyncControl_Interval_Whole, "Sync Control send whole sync");
  mu_assert(timing7.offset == 17, "Sync Control should have an offset of 17");

  _cleanup(sc, track_count, buffers, loop_tracks);

  return NULL;
}

char *test_sync_control() {
  mu_suite_start();

  mu_run_test(test_sync_control_create);
  mu_run_test(test_syncing);

  return NULL;
}
