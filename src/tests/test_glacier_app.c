#include <stdint.h>

#include "tests/minunit.h"

#include "core/glacier.h"

char *test_glacier_app_create() {

  uint8_t track_count = 3;
  uint32_t max_buffer_length = 4000;
  uint8_t channels = 2;
  GlacierAppState *glacier = glacier_create(track_count, max_buffer_length, channels);

  glacier_destroy(glacier);

  return NULL;
}

char *test_glacier_app() {
  mu_suite_start();

  mu_run_test(test_glacier_app_create);

  return NULL;
}
