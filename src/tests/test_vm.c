#include "tests/minunit.h"

#include "gvm/vm.h"
#include "gvm/block.h"

void test_vm_create() {
  VM *vm = vm_create();
  mu_assert(vm != NULL, "Could not create VM");

  vm_destroy(vm);
}

void test_vm_run() {
  VM *vm = vm_create();
  mu_assert(vm != NULL, "Could not create VM");
  VMBlock *blk = blk_create();
  mu_assert(blk != NULL, "Could not create VM Block");

  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, NUMBER_VAL(1.0)));
  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, NUMBER_VAL(2.0)));
  blk_write_code(blk, OP_ADD);
  blk_write_code(blk, OP_CONSTANT);
  blk_write_code(blk, blk_write_constant(blk, NUMBER_VAL(3.0)));
  blk_write_code(blk, OP_DIVIDE);
  blk_write_code(blk, OP_NEGATE);
  blk_write_code(blk, OP_EXIT);

  mu_assert(vm_run(vm, blk) == INTERPRET_OK, "Could not run VM");
  mu_assert(vm->exit_code == -1, "Exit code was not 1 it was %d", vm->exit_code);
}

void test_vm() {
  mu_run_test(test_vm_create);
  mu_run_test(test_vm_run);
}
