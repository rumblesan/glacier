#pragma once

#include <stdint.h>

typedef enum sync_control_interval {
  SyncControl_Interval_Whole,
  SyncControl_Interval_Half,
  SyncControl_Interval_Quarter,
  SyncControl_Interval_None,
} SyncControlInterval;

typedef struct SyncTimingMessage {

  SyncControlInterval interval;

  uint32_t offset;

} SyncTimingMessage;
