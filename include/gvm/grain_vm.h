#pragma once

#include <stdint.h>

#include "gvm/block.h"
#include "gvm/value.h"

#define GRAINVM_STACK_SIZE 256

typedef enum {
  OP_CONSTANT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_EXIT,
} GrainVMOps;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} GrainVMResult;

typedef struct {

  Value stack[GRAINVM_STACK_SIZE];
  Value* stack_top;

  Value exit_code;

  GrainVMBlock *block;

  bytecode_t *ip;

} GrainVM;

GrainVM *gvm_create();

void gvm_push(GrainVM *gvm, Value value);

Value gvm_pop(GrainVM *gvm);

GrainVMResult gvm_run(GrainVM *gvm, GrainVMBlock *block);

void gvm_destroy(GrainVM *gvm);

