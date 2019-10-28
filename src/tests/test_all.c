#include <stdio.h>

#include "tests/minunit.h"
#include "tests/test_control_message.h"

int tests_run;

int main(int argc, char *argv[]) {

    printf("\n");
    printf("*****************\n");
    printf("* Running tests *\n");
    printf("*****************\n\n");

    int result = 0;

    tests_run = 0;
    char *control_message_results = test_control_message();
    if (control_message_results != 0) {
        printf("%s\n", control_message_results);
    } else {
        printf("GOL TESTS PASSED\n");
    }
    printf("    Tests run: %d\n", tests_run);
    printf("\n");

    result = result || (control_message_results != 0);

    return result;
}

