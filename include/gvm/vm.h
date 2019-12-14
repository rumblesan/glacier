#pragma once

#include <stdint.h>

#include "gvm/block.h"
#include "gvm/value.h"

#define VM_STACK_SIZE 256

typedef enum {
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_CONSTANT,
  OP_NOT,
  OP_EQUAL,
  OP_GREATER,
  OP_LESSER,
  OP_EXIT,
} VMOps;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} VMResult;

typedef struct {

  Value stack[VM_STACK_SIZE];
  Value* stack_top;

  int8_t exit_code;

  VMBlock *block;

  bytecode_t *ip;

} VM;

VM *vm_create();

void vm_push(VM *vm, Value value);

Value vm_pop(VM *vm);

VMResult vm_run(VM *vm, VMBlock *block);

void vm_destroy(VM *vm);

