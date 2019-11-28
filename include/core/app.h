#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ck_ring.h"

#include "core/glacier.h"
#include "core/ui.h"

typedef struct AppState {

  GlacierAudio *glacier;
  UIInfo *ui;

  volatile bool running;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  ck_ring_buffer_t *control_bus_garbage_buffer;
  ck_ring_t *control_bus_garbage;

  ck_ring_buffer_t *ui_query_bus_buffer;
  ck_ring_t *ui_query_bus;

  ck_ring_buffer_t *ui_response_bus_buffer;
  ck_ring_t *ui_response_bus;

} AppState;

AppState *app_state_create(GlacierAudio *glacier, UIInfo *ui);

void app_state_destroy(AppState *as);
