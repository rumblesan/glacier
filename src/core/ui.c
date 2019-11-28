#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "dbg.h"

#include "core/ui.h"

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
