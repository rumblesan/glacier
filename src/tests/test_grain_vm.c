#include "tests/minunit.h"

#include "gvm/grain_vm.h"
#include "gvm/block.h"

void test_grain_vm_create() {
  GrainVM *grain_vm = gvm_create();
  mu_assert(grain_vm != NULL, "Could not create Grain VM");

  gvm_destroy(grain_vm);
}

void test_grain_vm_run() {
  GrainVM *grain_vm = gvm_create();
  mu_assert(grain_vm != NULL, "Could not create Grain VM");
  GrainVMBlock *blk = blk_create();
  mu_assert(blk != NULL, "Could not create Grain VM Block");

  blk_write_code(blk, OP_EXIT);

  mu_assert(gvm_run(grain_vm, blk) == INTERPRET_OK, "Could not run Grain VM");
}

void test_grain_vm() {
  mu_run_test(test_grain_vm_create);
  mu_run_test(test_grain_vm_run);
}
