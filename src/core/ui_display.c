#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include "ck_ring.h"

#include "dbg.h"

#include "core/app.h"
#include "core/ui.h"
#include "core/ui_coms.h"
#include "core/glacier.h"
#include "core/control_message.h"
#include "core/loop_track.h"

void draw_track_info(UIInfo *ui, uint8_t track_number, TrackUIDisplay *info) {
  char buf[1024];
  snprintf(buf, sizeof(buf),
      "Track %d %d - pos/len -> %10d/%10d",
      track_number, info->state, info->playback_head_pos, info->length);
  SDL_Color col = {255, 255, 255};
  SDL_Surface* surface = TTF_RenderText_Solid(ui->font, buf, col);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
  SDL_Rect text_rect = { 0, track_number * ui->font_size, 0, 0 };
  SDL_QueryTexture(texture, NULL, NULL, &(text_rect.w), &(text_rect.h));
  SDL_RenderCopy(ui->renderer, texture, NULL, &text_rect);
}

void ui_draw(AppState *app, UIDisplayData *uuid) {
  UIInfo *ui = app->ui;
  SDL_RenderClear(ui->renderer);

  for (uint8_t i = 0; i < app->glacier->track_count; i++) {
    draw_track_info(ui, i, uuid->track_info[i]);
  }

  SDL_RenderPresent(ui->renderer);
}

void ui_display(AppState *app) {

  UIDisplayData *uuid = ui_display_create(app->glacier->track_count);
  check(
    ck_ring_enqueue_spsc(app->ui_query_bus, app->ui_query_bus_buffer, uuid),
    "Couldn't start UI querying"
  );
  UIDisplayData *query = NULL;


  SDL_Event e;

  while (app->running){
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        app->running = false;
      }
      if (e.type == SDL_KEYDOWN){
        app->running = false;
      }
      if (e.type == SDL_MOUSEBUTTONDOWN){
        app->running = false;
      }
    }

    if (
      ck_ring_dequeue_spsc(
        app->ui_query_bus,
        app->ui_query_bus_buffer,
        &query
      )
    ) {
      ui_draw(app, query);
      ck_ring_enqueue_spsc(
        app->ui_query_bus,
        app->ui_query_bus_buffer,
        query
      );
    }

  }

error:
  return;
}

