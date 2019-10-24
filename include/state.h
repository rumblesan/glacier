#ifndef __GLACIER_STATE__
#define __GLACIER_STATE__

#include "types.h"

typedef struct GlacierState {
  SAMPLE *buffer;
} GlacierState;

GlacierState *gs_create(int samples, int channels);

void gs_destroy(GlacierState *gs);

#endif
