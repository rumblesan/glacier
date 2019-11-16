#undef NDEBUG
#pragma once

#include <stdio.h>
#include <dbg.h>
#include <stdlib.h>

/*  Maximum length of last message */
#define MINUNIT_MESSAGE_LEN 1024

extern int minunit_run;
extern int minunit_assert;
extern int minunit_fail;
extern int minunit_status;

/*  Last message */
char minunit_last_message[MINUNIT_MESSAGE_LEN];

#define test_err(M, ...) snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "[FAILURE] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define MU__SAFE_BLOCK(block) do {\
	block\
} while(0)

#define mu_run_suite(suite, name) MU__SAFE_BLOCK(\
	printf("%s tests:\n", name);\
	suite();\
  printf("\n");\
	fflush(stdout);\
)

#define mu_run_test(test) MU__SAFE_BLOCK(\
	minunit_status = 0;\
	test();\
	minunit_run++;\
	if (minunit_status) {\
		minunit_fail++;\
		printf("F");\
		printf("\n%s\n", minunit_last_message);\
	}\
	fflush(stdout);\
)

#define mu_assert(test, message, ...) MU__SAFE_BLOCK(\
	minunit_assert++;\
	if (!(test)) {\
    test_err(message, ##__VA_ARGS__);\
		minunit_status = 1;\
		return;\
	} else {\
		printf(".");\
	}\
)

#define MU_REPORT() MU__SAFE_BLOCK(\
	printf("\n%d tests, %d assertions, %d failures\n", minunit_run, minunit_assert, minunit_fail);\
)
