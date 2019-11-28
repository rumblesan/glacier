#include <stdio.h>
#include <stdint.h>

#include "tests/minunit.h"
#include "tests/test_all.h"

int minunit_run = 0;
int minunit_assert = 0;
int minunit_fail = 0;
int minunit_status = 0;

int main(int argc, char *argv[]) {

    printf("\n*****************");
    printf("\n* Running tests *");
    printf("\n*****************\n\n");

    mu_run_suite(test_audio_buffer, "Audio Buffer");
    mu_run_suite(test_control_message, "Control Message");
    mu_run_suite(test_sync_control, "Sync Control");
    mu_run_suite(test_loop_track, "Loop Track");
    mu_run_suite(test_glacier_app, "Glacier App");

    MU_REPORT();
    return minunit_fail > 0;
}

