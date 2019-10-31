#include "tests/minunit.h"

#include "core/glacier.h"

char *test_glacier_app_create() {

  int track_count = 3;
  unsigned int max_buffer_length = 4000;
  int channels = 2;
  GlacierAppState *glacier = glacier_create(track_count, max_buffer_length, channels);

  glacier_destroy(glacier);

  return NULL;
}

char *test_glacier_app() {
  mu_suite_start();

  mu_run_test(test_glacier_app_create);

  return NULL;
}
