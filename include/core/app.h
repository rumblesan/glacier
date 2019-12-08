#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ck_ring.h"
#include "portmidi.h"

#include "core/glacier.h"
#include "core/config.h"
#include "core/ui.h"

typedef struct AppState {

  GlacierAudio *glacier;
  UIInfo *ui;

  volatile bool running;

  bool audio_passthrough;

  ck_ring_buffer_t *midi_control_bus_buffer;
  ck_ring_t *midi_control_bus;

  ck_ring_buffer_t *control_bus_buffer;
  ck_ring_t *control_bus;

  ck_ring_buffer_t *control_bus_garbage_buffer;
  ck_ring_t *control_bus_garbage;

  ck_ring_buffer_t *ui_query_bus_buffer;
  ck_ring_t *ui_query_bus;

  ck_ring_buffer_t *ui_response_bus_buffer;
  ck_ring_t *ui_response_bus;

} AppState;

AppState *app_state_create(GlacierAudio *glacier, UIInfo *ui, GlacierCfg *cfg);

void app_state_destroy(AppState *as);
