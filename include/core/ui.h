#pragma once

#include <stdint.h>

typedef struct UIInfo {

  uint8_t track_count;

} UIInfo;

UIInfo *ui_create();

void ui_destroy(UIInfo *ui);
