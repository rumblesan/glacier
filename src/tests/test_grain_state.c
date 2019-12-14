#include "tests/minunit.h"

#include "gvm/grain_state.h"

void test_grain_state_create() {
  GrainState *grain_state = gs_create();
  mu_assert(grain_state != NULL, "Could not create Grain State");

  gs_destroy(grain_state);
}

void test_grain_state() {
  mu_run_test(test_grain_state_create);
}
