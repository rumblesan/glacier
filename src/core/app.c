#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ck_ring.h"

#include "dbg.h"

#include "core/app.h"
#include "core/glacier.h"
#include "core/ui.h"
#include "core/config.h"

bool create_ring_buffer(ck_ring_buffer_t **buffer, ck_ring_t **bus, uint16_t size) {
  *bus = malloc(sizeof(ck_ring_t));
  check_mem(*bus);
  *buffer = malloc(sizeof(ck_ring_buffer_t) * size);
  check_mem(*buffer);
  ck_ring_init(*bus, size);
  return true;
error:
  return false;
}

AppState *app_state_create(GlacierAudio *glacier, UIInfo *ui, GlacierCfg *cfg) {

  AppState *as = malloc(sizeof(AppState));
  check_mem(as);

  as->running = true;
  as->audio_passthrough = cfg->audio_passthrough;

  check_mem(glacier);
  as->glacier = glacier;

  as->midi_in = NULL;
  as->midi_active = false;

  check_mem(ui);
  as->ui = ui;

  uint16_t control_bus_size = 1024;

  check(
    create_ring_buffer(&as->midi_control_bus_buffer, &as->midi_control_bus, control_bus_size),
    "Could not create midi control bus ring"
  );
  check(
    create_ring_buffer(&as->control_bus_buffer, &as->control_bus, control_bus_size),
    "Could not create control bus ring"
  );
  check(
    create_ring_buffer(&as->control_bus_garbage_buffer, &as->control_bus_garbage, control_bus_size),
    "Could not create control bus garbage ring"
  );

  check(
    create_ring_buffer(&as->ui_query_bus_buffer, &as->ui_query_bus, control_bus_size),
    "Could not create ui query ring"
  );
  check(
    create_ring_buffer(&as->ui_response_bus_buffer, &as->ui_response_bus, control_bus_size),
    "Could not create ui response ring"
  );

  return as;
error:
  return NULL;
}

void app_state_destroy(AppState *as) {
  check(as != NULL, "Invalid App State");

  // FIXME cleanup everything properlly
  free(as);
  return;
error:
  log_err("Could not clean up App State");
}
