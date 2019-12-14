#include "tests/minunit.h"

#include "gvm/grain_vm.h"

void test_grain_vm_create() {
  GrainVM *grain_vm = gvm_create();
  mu_assert(grain_vm != NULL, "Could not create Grain VM");

  gvm_destroy(grain_vm);
}

void test_grain_vm() {
  mu_run_test(test_grain_vm_create);
}
