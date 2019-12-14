#include <stdlib.h>

#include "dbg.h"

#include "gvm/value.h"
#include "gvm/memory.h"

ValueArray *val_array_create() {
  ValueArray *va = calloc(1, sizeof(ValueArray));
  check_mem(va);

  va->values = NULL;

  va->count = 0;
  va->capacity = 0;

  return va;
error:
  return NULL;
}

void val_array_write(ValueArray *va, Value value) {
  if (va->capacity < va->count + 1) {
    int oldCapacity = va->capacity;
    va->capacity = GROW_CAPACITY(oldCapacity);
    Value *new_mem = GROW_ARRAY(va->values, Value, oldCapacity, va->capacity);
    check_mem(new_mem);
    va->values = new_mem;
  }

  va->values[va->count] = value;
  va->count++;
  return;
error:
  log_err("Could not resize block code memory");
}

void val_array_destroy(ValueArray *va) {
  check(va != NULL, "Invalid Value Array");
  FREE_ARRAY(Value, va->values, va->capacity);
  free(va);
  return;
error:
  log_err("Could not clean up Value Array");
}
