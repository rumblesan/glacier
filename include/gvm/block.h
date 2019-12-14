#pragma once

#include <stdint.h>

#include "gvm/value.h"

typedef struct {

  uint32_t count;
  uint32_t capacity;

  uint8_t *code;

  ValueArray *constants;

} GrainVMBlock;

GrainVMBlock *blk_create();

void blk_write_code(GrainVMBlock *blk, uint8_t code_byte);

uint32_t blk_write_constant(GrainVMBlock *blk, Value constant);

void blk_destroy(GrainVMBlock *blk);
