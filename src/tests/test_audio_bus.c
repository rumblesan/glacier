#include <stdint.h>

#include "tests/minunit.h"

#include "core/audio_bus.h"

void test_control_audio_bus_create() {

  uint8_t channels = 1;
  uint8_t first_channel = 1;

  AudioBus *abus = abus_create(channels, first_channel);
  mu_assert(abus != NULL, "Could not create Audio Bus");
  mu_assert(abus->channels == channels, "Incorrect channels number");
  mu_assert(abus->first_channel == first_channel, "Incorrect first channel number");

  abus_destroy(abus);
}

void test_audio_bus() {
  mu_run_test(test_control_audio_bus_create);
}
