#include "tests/minunit.h"

#include "core/control_message.h"
#include "core/loop_track.h"

char *test_control_message_create() {

  int track_number = 1;
  LoopTrackAction action = LoopTrack_Action_Record;

  ControlMessage *cm = cm_create(track_number, action);
  mu_assert(cm != NULL, "Could not create Control Message");
  mu_assert(cm->track_number == track_number, "Incorrect buffer number");
  mu_assert(cm->action == action, "Incorrect action");

  cm_destroy(cm);
  return NULL;
}

char *test_control_message() {
  mu_suite_start();

  mu_run_test(test_control_message_create);

  return NULL;
}
