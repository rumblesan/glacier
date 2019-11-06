#include <stdio.h>
#include <stdint.h>

#include "tests/minunit.h"
#include "tests/test_all.h"

uint8_t tests_run;

typedef char* (test_suite_func)();
uint8_t run_suite(test_suite_func suite, char *suite_name) {
  printf("%s tests\n", suite_name);
  char *result = (*suite)();
  if (result != 0) {
    printf("%s\n", result);
    return 1;
  } else {
    printf("suite passed\n\n");
    return 0;
  }
}

int main(int argc, char *argv[]) {

    printf("\n*****************");
    printf("\n* Running tests *");
    printf("\n*****************\n\n");

    tests_run = 0;

    uint8_t result = 0;
    result = result || run_suite(&test_control_message, "Control Message");
    result = result || run_suite(&test_sync_control, "Sync Control");
    result = result || run_suite(&test_loop_track, "Loop Track");
    result = result || run_suite(&test_glacier_app, "Glacier App");

    printf("Tests run: %d\n\n", tests_run);

    return result;
}

