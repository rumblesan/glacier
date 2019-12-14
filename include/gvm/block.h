#pragma once

#include <stdint.h>

#include "gvm/value.h"

#define bytecode_t uint8_t

typedef struct {

  uint32_t count;
  uint32_t capacity;

  bytecode_t *code;

  ValueArray *constants;

} GrainVMBlock;

GrainVMBlock *blk_create();

void blk_write_code(GrainVMBlock *blk, bytecode_t code_byte);

uint32_t blk_write_constant(GrainVMBlock *blk, Value constant);

void blk_destroy(GrainVMBlock *blk);
