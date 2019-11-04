#ifndef __GLACIER_SYNC_TIMING_MESSAGE__
#define __GLACIER_SYNC_TIMING_MESSAGE__

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

#endif
