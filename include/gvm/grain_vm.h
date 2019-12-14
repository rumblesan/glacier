#pragma once

#include <stdint.h>

#include "gvm/block.h"
#include "gvm/value.h"

#define GRAINVM_STACK_SIZE 256

typedef enum {
  OP_Constant,
  OP_Exit,
} GrainVMOps;

typedef struct {

  Value stack[GRAINVM_STACK_SIZE];

  bytecode_t *ip;

} GrainVM;

GrainVM *gvm_create();

void gvm_run(GrainVM *gvm, GrainVMBlock *blk);

void gvm_destroy(GrainVM *gvm);

