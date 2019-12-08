#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "portmidi.h"
#include "ck_ring.h"

typedef struct MidiIO {

  volatile bool active;

  ck_ring_buffer_t *com_queue_buffer;
  ck_ring_t *com_queue;

  PortMidiStream *midi_in;

  int32_t input_device_id;
  const PmDeviceInfo *input_device_info;

} MidiIO;

MidiIO *midi_io_create(ck_ring_t *com_queue, ck_ring_buffer_t *com_queue_buffer);

bool midi_io_run(MidiIO *mio);

void midi_io_destroy(MidiIO *mio);
