#include <stdlib.h>

#include "dbg.h"

#include "gvm/grain_vm.h"
#include "gvm/block.h"
#include "gvm/value.h"

GrainVM *gvm_create() {
  GrainVM *vm = calloc(1, sizeof(GrainVM));
  check_mem(vm);

  vm->block = NULL;
  vm->ip = NULL;

  return vm;
error:
  return NULL;
}

static GrainVMResult run(GrainVM *gvm) {
#define READ_BYTE() (*vm.ip++)
  GrainVM vm = *gvm;
  for (;;) {
    bytecode_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_EXIT: {
        return INTERPRET_OK;
      }
    }
  }

#undef READ_BYTE
}

GrainVMResult gvm_run(GrainVM *vm, GrainVMBlock *block) {
  vm->block = block;
  vm->ip = block->code;
  return run(vm);
}

void gvm_destroy(GrainVM *gvm) {
  check(gvm != NULL, "Invalid Grain VM");
  free(gvm);
  return;
error:
  log_err("Could not clean up Grain VM");
}
