#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ck_ring.h"

#include "dbg.h"

#include "core/app.h"
#include "core/glacier.h"

AppState *app_state_create(GlacierAudio *glacier) {

  AppState *as = malloc(sizeof(AppState));
  check_mem(as);

  as->running = true;

  check_mem(glacier);
  as->glacier = glacier;

  uint16_t control_bus_size = 1024;

  as->control_bus = malloc(sizeof(ck_ring_t));
  check_mem(as->control_bus);
  as->control_bus_buffer = malloc(
    sizeof(ck_ring_buffer_t) * control_bus_size
  );
  check_mem(as->control_bus_buffer);
  ck_ring_init(as->control_bus, control_bus_size);

  as->control_bus_garbage = malloc(sizeof(ck_ring_t));
  check_mem(as->control_bus_garbage);
  as->control_bus_garbage_buffer = malloc(
    sizeof(ck_ring_buffer_t) * control_bus_size
  );
  check_mem(as->control_bus_garbage_buffer);
  ck_ring_init(as->control_bus_garbage, control_bus_size);

  return as;
error:
  return NULL;
}

void app_state_destroy(AppState *as) {
  check(as != NULL, "Invalid App State");

  free(as);
  return;
error:
  log_err("Could not clean up App State");
}
