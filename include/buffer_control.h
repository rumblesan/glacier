#ifndef __GLACIER_BUFFER_CONTROL__
#define __GLACIER_BUFFER_CONTROL__

#include "types.h"
#include "audio_buffer.h"

enum command { NoCommand, StartRecording, StopRecording };

typedef struct BufferControl {
  enum command cmd;
} BufferControl;

void bc_start_recording(BufferControl *bc);

void bc_stop_recording(BufferControl *bc);

BufferControl *bc_create();

void bc_destroy(BufferControl *bc);

#endif
