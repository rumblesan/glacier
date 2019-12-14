#include <stdlib.h>

#include "dbg.h"

#include "gvm/block.h"
#include "gvm/memory.h"
#include "gvm/value.h"

VMBlock *blk_create() {
  VMBlock *blk = calloc(1, sizeof(VMBlock));
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

void blk_write_code(VMBlock *blk, bytecode_t byte) {
  if (blk->capacity < blk->count + 1) {
    int oldCapacity = blk->capacity;
    blk->capacity = GROW_CAPACITY(oldCapacity);
    bytecode_t *new_mem = GROW_ARRAY(blk->code, bytecode_t, oldCapacity, blk->capacity);
    check_mem(new_mem);
    blk->code = new_mem;
  }

  blk->code[blk->count] = byte;
  blk->count++;
  return;
error:
  log_err("Could not resize block code memory");
}

uint32_t blk_write_constant(VMBlock *blk, Value constant) {
  val_array_write(blk->constants, constant);
  return (blk->constants->count - 1);
}

void blk_destroy(VMBlock *blk) {
  check(blk != NULL, "Invalid VM Block");
  FREE_ARRAY(bytecode_t, blk->code, blk->capacity);
  val_array_destroy(blk->constants);
  free(blk);
  return;
error:
  log_err("Could not clean up VM Block");
}
