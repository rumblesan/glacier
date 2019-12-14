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

  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, 1.0));
  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, 2.0));
  blk_write_code(blk, OP_ADD);
  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, 3.0));
  blk_write_code(blk, OP_DIVIDE);
  blk_write_code(blk, OP_NEGATE);
  blk_write_code(blk, OP_EXIT);

  mu_assert(gvm_run(grain_vm, blk) == INTERPRET_OK, "Could not run Grain VM");
  mu_assert(grain_vm->exit_code == -1.0, "Exit code was not 1 it was %f", grain_vm->exit_code);
}

void test_grain_vm() {
  mu_run_test(test_grain_vm_create);
  mu_run_test(test_grain_vm_run);
}
