#include <stdlib.h>
#include <stdbool.h>

#include "dbg.h"

#include "gvm/vm.h"
#include "gvm/block.h"
#include "gvm/value.h"

void vm_reset_stack(VM *vm) {
  vm->stack_top = vm->stack;
}

VM *vm_create() {
  VM *vm = calloc(1, sizeof(VM));
  check_mem(vm);

  vm->block = NULL;
  vm->ip = NULL;

  vm_reset_stack(vm);

  vm->exit_code = 0;

  return vm;
error:
  return NULL;
}

void push(VM *vm, Value value) {
  *vm->stack_top = value;
  vm->stack_top++;
}

Value pop(VM *vm) {
  vm->stack_top--;
  return *vm->stack_top;
}

void runtime_error(VM *vm, const char *msg) {
  vm->exit_code = -1;
  fprintf(stderr, "%s\n", msg);
}

Value peek(VM *vm, int distance) {   
  return vm->stack_top[-1 - distance];
}

bool is_falsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

bool values_equal(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
  }
}

VMResult run(VM *vm) {
  VM _vm = *vm;
#define READ_BYTE() (*_vm.ip++)
#define READ_CONSTANT() (_vm.block->constants->values[READ_BYTE()])

#define BINARY_OP(value_type, op) \
    do { \
      if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
        runtime_error(vm, "Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      float b = AS_NUMBER(pop(vm)); \
      float a = AS_NUMBER(pop(vm)); \
      push(vm, value_type(a op b)); \
    } while (false)

#define UNARY_OP(op) \
    do { \
      float a = pop(vm); \
      push(vm, op a); \
    } while (false)

  for (;;) {
    bytecode_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_NIL:      push(vm, NIL_VAL); break;
      case OP_TRUE:     push(vm, BOOL_VAL(true)); break;
      case OP_FALSE:    push(vm, BOOL_VAL(true)); break;
      case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, +); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(vm, 0))) {
          runtime_error(vm, "Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));
        break;
      case OP_NOT: push(vm, BOOL_VAL(is_falsey(pop(vm)))); break;
      case OP_EQUAL: {
        Value b = pop(vm);
        Value a = pop(vm);
        push(vm, BOOL_VAL(values_equal(a, b)));
        break;
      }
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(vm, _vm.stack[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        _vm.stack[slot] = peek(vm, 0);
        break;
      }
      case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
      case OP_LESSER:   BINARY_OP(BOOL_VAL, <); break;
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(vm, constant);
        break;
      }
      case OP_EXIT: {
        if (!IS_NUMBER(peek(vm, 0))) {
          runtime_error(vm, "Exit code must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        float exit_code = AS_NUMBER(pop(vm));
        vm->exit_code = exit_code;
        return INTERPRET_OK;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
}

VMResult vm_run(VM *vm, VMBlock *block) {
  vm->block = block;
  vm->ip = block->code;
  return run(vm);
}

void vm_destroy(VM *vm) {
  check(vm != NULL, "Invalid VM");
  free(vm);
  return;
error:
  log_err("Could not clean up VM");
}
