#undef NDEBUG
#pragma once

#include <stdio.h>
#include <dbg.h>
#include <stdlib.h>

#define mu_suite_start() char *message = NULL

#define test_err(M, ...) fprintf(stderr, "[FAILURE] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define mu_assert(test, message, ...) if (!(test)) { test_err(message, ##__VA_ARGS__); return message; }
#define mu_run_test(test) log_info("\n-----%s", " " #test); \
    message = test(); tests_run++; if (message) return message;

#define RUN_TESTS(name) int main(int argc, char *argv[]) {\
    argc = 1; \
    printf("----\nRUNNING: %s\n", argv[0]);\
    char *result = name();\
    if (result != 0) {\
        printf("FAILED: %s\n", result);\
    }\
    else {\
        printf("ALL TESTS PASSED\n");\
    }\
    printf("Tests run: %d\n", tests_run);\
    exit(result != 0);\
}

int tests_run;
