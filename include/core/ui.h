#pragma once

#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct UIInfo {

  uint8_t track_count;

  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;

} UIInfo;

UIInfo *ui_create(
  const char *window_name,
  const char *font_file_path,
  uint8_t font_size
);

void ui_destroy(UIInfo *ui);
