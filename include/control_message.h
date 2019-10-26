#ifndef __GLACIER_CONTROL_MESSAGE__
#define __GLACIER_CONTROL_MESSAGE__

enum buffer_command { StartRecording, StopRecording, StartPlayback, StopPlayback, PausePlayback };

typedef struct ControlMessage {
  int buffer_number;
  enum buffer_command cmd;
} ControlMessage;

ControlMessage *cm_create(int buffer_number, enum buffer_command cmd);

void cm_destroy(ControlMessage *cm);

#endif
