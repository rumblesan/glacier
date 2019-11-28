#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "dbg.h"

#include "core/ui_coms.h"
#include "core/loop_track.h"

TrackUIDisplay *track_display_create() {
  TrackUIDisplay *tuid = calloc(1, sizeof(TrackUIDisplay));
  check_mem(tuid);
  return tuid;
error:
  return NULL;
}

UIDisplayData *ui_display_create(uint8_t track_count) {

  UIDisplayData *uidd = calloc(1, sizeof(UIDisplayData));
  check_mem(uidd);

  uidd->track_count = track_count;

  uidd->track_info = malloc(sizeof(TrackUIDisplay*) * track_count);
  check_mem(uidd->track_info);
  for (uint8_t i = 0; i < track_count; i++) {
    uidd->track_info[i] = track_display_create();
    check_mem(uidd->track_info[i]);
  }
  return uidd;
error:
  return NULL;
}

