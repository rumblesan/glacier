#pragma once

typedef enum sync_control_interval {
  SyncControl_Interval_Whole,
  SyncControl_Interval_Half,
  SyncControl_Interval_Quarter,
  SyncControl_Interval_None,
} SyncControlInterval;

typedef struct SyncTimingMessage {

  SyncControlInterval interval;

  unsigned int offset;

} SyncTimingMessage;
