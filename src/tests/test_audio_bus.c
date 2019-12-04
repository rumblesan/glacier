#include <stdint.h>

#include "tests/minunit.h"

#include "core/audio_bus.h"

void test_control_audio_bus_create() {

  AudioBusChannelCount channel_count = AudioBus_Stereo;
  uint8_t first_channel = 1;

  AudioBus *abus = abus_create(channel_count, first_channel);
  mu_assert(abus != NULL, "Could not create Audio Bus");
  mu_assert(abus->channel_count == channel_count, "Incorrect channel count");
  mu_assert(abus->first_channel == first_channel, "Incorrect first channel number");

  abus_destroy(abus);
}

void test_audio_bus() {
  mu_run_test(test_control_audio_bus_create);
}
