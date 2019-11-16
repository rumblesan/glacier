#include <stdint.h>

#include "tests/minunit.h"

#include "core/control_message.h"
#include "core/loop_track.h"

void test_control_message_create() {

  uint8_t track_number = 1;
  LoopTrackAction action = LoopTrack_Action_Record;

  ControlMessage *cm = cm_create(track_number, action);
  mu_assert(cm != NULL, "Could not create Control Message");
  mu_assert(cm->track_number == track_number, "Incorrect buffer number");
  mu_assert(cm->action == action, "Incorrect action");

  cm_destroy(cm);
}

void test_control_message() {
  mu_run_test(test_control_message_create);
}
