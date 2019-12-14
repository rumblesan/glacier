#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core/audio_buffer.h"

typedef struct {

  AudioBuffer *buffer;

  uint32_t count;
  uint32_t length;

} Grain;

typedef struct {
} Trigger;

typedef struct {

  bool running;
  uint32_t count;
  uint32_t length;

} Phasor;

typedef struct {



} GrainState;

GrainState *gs_create();

void gs_destroy(GrainState *gs);
