#pragma once

#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "core/ui_coms.h"

typedef struct UIInfo {

  uint8_t track_count;
  uint8_t font_size;

  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;

} UIInfo;

UIInfo *ui_create(
  const char *window_name,
  const char *font_file_path,
  uint8_t font_size
);

void ui_draw(UIInfo *ui, UIDisplayData *uuid);

void ui_destroy(UIInfo *ui);
