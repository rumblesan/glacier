#pragma once

#include <stdbool.h>

#include <libconfig.h>

#include "core/audio_bus.h"

typedef struct GlacierCfg {
  uint8_t input_bus;
  AudioBusChannelCount input_bus_channels;

  const char *font_filepath;
  uint8_t font_size;

  config_t *cfg;

  bool audio_passthrough;
  bool midi_enabled;
  bool osc_enabled;
} GlacierCfg;

GlacierCfg *cfg_read(char *config_path);

void cfg_destroy(GlacierCfg *cfg);
