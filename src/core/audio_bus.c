#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "dbg.h"

#include "core/audio_bus.h"

AudioBus *abus_create(uint8_t channels, uint8_t first_channel) {
  AudioBus *abus = malloc(sizeof(AudioBus));
  check_mem(abus);
  abus->channels = channels;
  abus->first_channel = first_channel;

  return abus;
error:
  return NULL;
}

void abus_destroy(AudioBus *abus) {
  check(abus != NULL, "Invalid Audio Bus");
  free(abus);
  return;
error:
  log_err("Could not clean up Audio Bus");
}
