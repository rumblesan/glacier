#include <stdlib.h>

#include "dbg.h"

#include "gvm/block.h"
#include "gvm/memory.h"
#include "gvm/value.h"

GrainVMBlock *blk_create() {
  GrainVMBlock *blk = calloc(1, sizeof(GrainVMBlock));
  check_mem(blk);

  blk->code = NULL;

  blk->count = 0;
  blk->capacity = 0;

  blk->constants = val_array_create();
  check_mem(blk->constants);

  return blk;
error:
  return NULL;
}

void blk_write_byte(GrainVMBlock *blk, uint8_t byte) {
  if (blk->capacity < blk->count + 1) {
    int oldCapacity = blk->capacity;
    blk->capacity = GROW_CAPACITY(oldCapacity);
    uint8_t *new_mem = GROW_ARRAY(blk->code, uint8_t, oldCapacity, blk->capacity);
    check_mem(new_mem);
    blk->code = new_mem;
  }

  blk->code[blk->count] = byte;
  blk->count++;
error:
  log_err("Could not resize block code memory");
}

uint32_t blk_write_constant(GrainVMBlock *blk, Value constant) {
  val_array_write(blk->constants, constant);
  return (blk->constants->count - 1);
}

void blk_destroy(GrainVMBlock *blk) {
  check(blk != NULL, "Invalid Grain VM Block");
  FREE_ARRAY(uint8_t, blk->code, blk->capacity);
  val_array_destroy(blk->constants);
  free(blk);
  return;
error:
  log_err("Could not clean up Grain VM Block");
}
