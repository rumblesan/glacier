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


void ui_draw(UIInfo *ui, UIDisplayData *uuid) {
  SDL_RenderClear(ui->renderer);

  SDL_Color White = {255, 255, 255};
  SDL_Surface* surfaceMessage = TTF_RenderText_Solid(
    ui->font, "put your text here", White
  );
  SDL_Texture* texture = SDL_CreateTextureFromSurface(
    ui->renderer, surfaceMessage
  );

  int texW = 0;
  int texH = 0;
  SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
  SDL_Rect text_rect = { 0, 0, texW, texH };

  SDL_RenderCopy(ui->renderer, texture, NULL, &text_rect);

  SDL_RenderPresent(ui->renderer);
}

void ui_display(AppState *app) {
  UIInfo *ui = app->ui;

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
      ui_draw(ui, query);
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

