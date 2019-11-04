#include "tests/minunit.h"

#include "core/loop_track.h"
#include "core/audio_buffer.h"

char *test_loop_track_create() {

  AudioBuffer *buffer = ab_create(1024, 2);

  LoopTrack *loop_track = lt_create(0, buffer);
  mu_assert(loop_track != NULL, "Could not create Loop Track");

  mu_assert(loop_track->state == LoopTrack_State_Stopped, "Loop Track should start in empty state");

  lt_destroy(loop_track);

  ab_destroy(buffer);

  return NULL;
}

char *test_loop_track() {
  mu_suite_start();

  mu_run_test(test_loop_track_create);

  return NULL;
}
