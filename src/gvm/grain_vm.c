#include <stdlib.h>
#include <stdbool.h>

#include "dbg.h"

#include "gvm/grain_vm.h"
#include "gvm/block.h"
#include "gvm/value.h"

void gvm_reset_stack(GrainVM *gvm) {
  gvm->stack_top = gvm->stack;
}

GrainVM *gvm_create() {
  GrainVM *vm = calloc(1, sizeof(GrainVM));
  check_mem(vm);

  vm->block = NULL;
  vm->ip = NULL;

  gvm_reset_stack(vm);

  vm->exit_code = 0;

  return vm;
error:
  return NULL;
}

void push(GrainVM *gvm, Value value) {
  *gvm->stack_top = value;
  gvm->stack_top++;
}

Value pop(GrainVM *gvm) {
  gvm->stack_top--;
  return *gvm->stack_top;
}

void runtime_error(GrainVM *gvm, const char *msg) {
  gvm->exit_code = -1;
  fprintf(stderr, "%s\n", msg);
}

Value peek(GrainVM *vm, int distance) {   
  return vm->stack_top[-1 - distance];
}   

GrainVMResult run(GrainVM *gvm) {
  GrainVM vm = *gvm;
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.block->constants->values[READ_BYTE()])

#define BINARY_OP(value_type, op) \
    do { \
      if (!IS_NUMBER(peek(gvm, 0)) || !IS_NUMBER(peek(gvm, 1))) { \
        runtime_error(gvm, "Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      float b = AS_NUMBER(pop(gvm)); \
      float a = AS_NUMBER(pop(gvm)); \
      push(gvm, value_type(a op b)); \
    } while (false)

#define UNARY_OP(op) \
    do { \
      float a = pop(gvm); \
      push(gvm, op a); \
    } while (false)

  for (;;) {
    bytecode_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, +); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(gvm, 0))) {
          runtime_error(gvm, "Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(gvm, NUMBER_VAL(-AS_NUMBER(pop(gvm))));
        break;
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(gvm, constant);
        break;
      }
      case OP_EXIT: {
        if (!IS_NUMBER(peek(gvm, 0))) {
          runtime_error(gvm, "Exit code must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        float exit_code = AS_NUMBER(pop(gvm));
        gvm->exit_code = exit_code;
        return INTERPRET_OK;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
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
