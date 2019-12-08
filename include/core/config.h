#pragma once

#include <stdbool.h>

#include <libconfig.h>

#include "core/audio_bus.h"

typedef struct GlacierCfg {
  uint8_t input_bus;
  AudioBusChannelCount input_bus_channels;

  const char *font_filepath;

  config_t *cfg;

  bool audio_passthrough;
} GlacierCfg;

GlacierCfg *cfg_read(char *config_path);

void cfg_destroy(GlacierCfg *cfg);
