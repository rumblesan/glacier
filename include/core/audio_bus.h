#pragma once

#include <stdint.h>

typedef enum AudioBusChannelCount {
  AudioBus_Mono,
  AudioBus_Stereo,
} AudioBusChannelCount;

typedef struct AudioBus {
  AudioBusChannelCount channel_count;
  uint8_t first_channel;
} AudioBus;

AudioBus *abus_create(AudioBusChannelCount channel_count, uint8_t first_channel);

void abus_destroy(AudioBus *ab);
