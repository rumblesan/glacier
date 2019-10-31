#include "tests/minunit.h"

#include "core/control_message.h"

char *test_control_message_create() {

  int buffer_number = 1;
  enum buffer_command cmd = StartRecording;

  ControlMessage *cm = cm_create(buffer_number, cmd);
  mu_assert(cm != NULL, "Could not create Control Message");
  mu_assert(cm->buffer_number == buffer_number, "Incorrect buffer number");
  mu_assert(cm->cmd == cmd, "Incorrect command");

  cm_destroy(cm);
  return NULL;
}

char *test_control_message() {
  mu_suite_start();

  mu_run_test(test_control_message_create);

  return NULL;
}
