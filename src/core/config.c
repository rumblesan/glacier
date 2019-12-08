#include <stdlib.h>

#include <libconfig.h>

#include "dbg.h"

#include "core/config.h"

GlacierCfg *cfg_read(char *config_path) {
  log_info("Loading config from %s", config_path);

  config_t *cfg = NULL;

  GlacierCfg *glacier_cfg = malloc(sizeof(GlacierCfg));
  check_mem(glacier_cfg);

  glacier_cfg->cfg = malloc(sizeof(config_t));
  cfg = glacier_cfg->cfg;

  config_init(cfg);

  check(config_read_file(cfg, config_path),
    "Could not read config file - %s:%d - %s",
    config_error_file(cfg), config_error_line(cfg), config_error_text(cfg));

  // Channels config
  int input_bus = 0;
  check(config_lookup_int(
    cfg, "input_bus", &input_bus),
    "Could not read input bus setting");
  glacier_cfg->input_bus = (uint8_t)input_bus;

  int input_bus_channels = 0;
  check(config_lookup_int(
    cfg, "input_bus_channels", &input_bus_channels),
    "Could not read input bus channels setting");
  check(
      input_bus_channels == 1 || input_bus_channels == 2,
      "input bus channels must be 1 or 2");

  if (input_bus_channels == 1) {
    glacier_cfg->input_bus_channels = AudioBus_Mono;
  } else if (input_bus_channels == 2) {
    glacier_cfg->input_bus_channels = AudioBus_Stereo;
  }

  int audio_passthrough = 0;
  check(config_lookup_bool(
    cfg, "audio_passthrough", &audio_passthrough),
    "Could not read audio passthrough setting");
  if (audio_passthrough == 0) {
    glacier_cfg->audio_passthrough = false;
  } else {
    glacier_cfg->audio_passthrough = true;
  }

  check(config_lookup_string(
    cfg, "font_filepath", &glacier_cfg->font_filepath),
    "Could not read font filepath setting");

  return glacier_cfg;
 error:
  if (cfg != NULL) config_destroy(cfg);
  if (glacier_cfg != NULL) free(glacier_cfg);
  return NULL;
};

void cfg_destroy(GlacierCfg *gcfg) {
  check(gcfg != NULL, "Invalid config");
  if (gcfg->cfg != NULL) config_destroy(gcfg->cfg);
  free(gcfg);
  return;
error:
  log_err("Could not clean up config");
}
