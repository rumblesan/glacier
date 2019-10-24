#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "state.h"
#include "dbg.h"

GlacierState *gs_create(int samples, int channels) {
  GlacierState *gs = malloc(sizeof(GlacierState));
  check_mem(gs);

  SAMPLE *buffer = calloc(samples * channels, sizeof(SAMPLE *));
  check_mem(buffer);
  gs->buffer = buffer;

  return gs;
error:
  return NULL;
}


void gs_destroy(GlacierState *gs) {
  check(gs != NULL, "Invalid Glacier State");
  check(gs->buffer != NULL, "Invalid buffer in Glacier State");
  free(gs->buffer);
  free(gs);
  return;
error:
  log_err("Could not clean up Glacier State");
}
