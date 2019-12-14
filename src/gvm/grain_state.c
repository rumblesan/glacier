#include <stdlib.h>

#include "dbg.h"

#include "gvm/grain_state.h"

GrainState *gs_create() {
  GrainState *gs = calloc(1, sizeof(GrainState));
  check_mem(gs);

  return gs;
error:
  return NULL;
}

void gs_destroy(GrainState *gs) {
  check(gs != NULL, "Invalid Grain State");
  free(gs);
  return;
error:
  log_err("Could not clean up Grain State");
}
