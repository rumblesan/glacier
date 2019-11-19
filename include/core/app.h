#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ck_ring.h"

#include "core/glacier.h"

typedef struct AppState {

  GlacierAudio *glacier;

  volatile bool running;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  ck_ring_buffer_t *control_bus_garbage_buffer;
  ck_ring_t *control_bus_garbage;

} AppState;

AppState *app_state_create(GlacierAudio *glacier);

void app_state_destroy(AppState *as);
