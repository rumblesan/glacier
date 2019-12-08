#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "dbg.h"

#include "core/ui.h"
#include "core/ui_coms.h"

UIInfo *ui_create(
  const char *window_name,
  const char *font_file_path,
  uint8_t font_size
) {
  UIInfo *ui = malloc(sizeof(UIInfo));
  check_mem(ui);

  ui->font_size = font_size;

  check(SDL_Init(SDL_INIT_VIDEO) == 0, "could not init SDL");
  check(TTF_Init() == 0, "could not init True Type Font");

  ui->window = SDL_CreateWindow(
    window_name,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480, SDL_WINDOW_SHOWN
  );
  check(ui->window != NULL, "could not create SDL window");

	ui->renderer = SDL_CreateRenderer(
    ui->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );
  check(ui->renderer != NULL, "could not create SDL renderer");

  ui->font = TTF_OpenFont(font_file_path, font_size);
  check(ui->font != NULL, "could not load font: %s", font_file_path);

  return ui;
error:
  ui_destroy(ui);
  return NULL;
}

void draw_track_info(UIInfo *ui, uint8_t track_number, TrackUIDisplay *info) {
  char buf[1024];
  snprintf(buf, sizeof(buf),
      "Track %d %10s - pos/len -> %10d/%10d",
      track_number, lt_state_string(info->state), info->playback_head_pos, info->length);
  SDL_Color col = {255, 255, 255};
  SDL_Surface* surface = TTF_RenderText_Solid(ui->font, buf, col);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
  SDL_Rect text_rect = { 0, track_number * ui->font_size, 0, 0 };
  SDL_QueryTexture(texture, NULL, NULL, &(text_rect.w), &(text_rect.h));
  SDL_RenderCopy(ui->renderer, texture, NULL, &text_rect);

  // cleanup
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

void draw_sync_info(UIInfo *ui, UIDisplayData *uuid) {
  char buf[1024];
  snprintf(buf, sizeof(buf),
      "Syncing %10s - pos/len -> %10d/%10d",
      sc_state_string(uuid->sync_state), uuid->sync_pos, uuid->sync_length);
  SDL_Color col = {255, 255, 255};
  SDL_Surface* surface = TTF_RenderText_Solid(ui->font, buf, col);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
  SDL_Rect text_rect = { 0, (uuid->track_count + 1) * ui->font_size, 0, 0 };
  SDL_QueryTexture(texture, NULL, NULL, &(text_rect.w), &(text_rect.h));
  SDL_RenderCopy(ui->renderer, texture, NULL, &text_rect);

  // cleanup
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

void ui_draw(UIInfo *ui, UIDisplayData *uuid) {
  SDL_RenderClear(ui->renderer);

  for (uint8_t i = 0; i < uuid->track_count; i++) {
    draw_track_info(ui, i, uuid->track_info[i]);
  }
  draw_sync_info(ui, uuid);

  SDL_RenderPresent(ui->renderer);
}

void ui_destroy(UIInfo *ui) {
  check(ui != NULL, "Invalid UI Structure");
  if (ui->font != NULL) {
    TTF_CloseFont(ui->font);
  }
  if (ui->renderer != NULL) {
    SDL_DestroyRenderer(ui->renderer);
  }
  if (ui->window != NULL) {
    SDL_DestroyWindow(ui->window);
  }
  SDL_Quit();
  free(ui);
  return;
error:
  log_err("Could not clean up UI");
}
