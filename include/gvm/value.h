#pragma once

#include <stdint.h>

typedef float Value;

typedef struct {

  uint32_t count;
  uint32_t capacity;

  Value *values;

} ValueArray;

ValueArray *val_array_create();              

void val_array_write(ValueArray* va, Value value);

void val_array_destroy(ValueArray* va);   
