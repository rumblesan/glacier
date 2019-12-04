#pragma once

#include <stdbool.h>

#include "core/audio_bus.h"

typedef struct GlacierCfg {
  uint8_t input_bus;
  AudioBusChannelCount input_bus_channels;

  bool audio_passthrough;
} GlacierCfg;

GlacierCfg *cfg_read(char *config_path);

void cfg_destroy(GlacierCfg *cfg);