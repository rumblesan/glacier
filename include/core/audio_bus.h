#pragma once

#include <stdint.h>

typedef struct AudioBus {
  uint8_t channels;
  uint8_t first_channel;
} AudioBus;

AudioBus *abus_create(uint8_t channels, uint8_t first_channel);

void abus_destroy(AudioBus *ab);
