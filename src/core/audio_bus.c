#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "dbg.h"

#include "core/audio_bus.h"

const AudioBus *abus_create(AudioBusChannelCount channel_count, uint8_t first_channel) {
  AudioBus *abus = malloc(sizeof(AudioBus));
  check_mem(abus);
  abus->channel_count = channel_count;
  abus->first_channel = first_channel;

  return abus;
error:
  return NULL;
}

void abus_destroy(const AudioBus *abus) {
  check(abus != NULL, "Invalid Audio Bus");
  free((void *)abus);
  return;
error:
  log_err("Could not clean up Audio Bus");
}
