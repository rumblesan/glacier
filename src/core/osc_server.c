#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "ck_ring.h"
#include "lo/lo.h"

#include "dbg.h"

#include "core/osc_server.h"
#include "core/app.h"
#include "core/glacier.h"
#include "core/control_message.h"

void send_action(AppState *app, uint8_t track_id, LoopTrackAction action) {
  ControlMessage *cm = cm_create(track_id, action);
  if (
    ck_ring_enqueue_spsc(app->osc_control_bus,app->osc_control_bus_buffer, cm) == false
  ) {
    printf("Could not send message to audio thread\n");
  }
}

void error(int num, const char *msg, const char *path) {
  printf("liblo server error %d in path %s: %s\n", num, path, msg);
  fflush(stdout);
}

int quit_handler(
  const char *path, const char *types, lo_arg **argv,
  int argc, void *data, void *user_data
) {
  AppState *app = (AppState*) user_data;
  app->running = false;

  printf("quit OSC message\n");
  fflush(stdout);

  return 0;
}

int record_handler(
  const char *path, const char *types, lo_arg **argv,
  int argc, void *data, void *user_data
) {
  uint8_t track_id = argv[0]->i;
  printf("Record Action for track %d\n", track_id);
  send_action(user_data, track_id, LoopTrack_Action_Record);
  fflush(stdout);
  return 0;
}

int playback_handler(
  const char *path, const char *types, lo_arg **argv,
  int argc, void *data, void *user_data
) {
  uint8_t track_id = argv[0]->i;
  printf("Play Action for track %d\n", track_id);
  send_action(user_data, track_id, LoopTrack_Action_Playback);
  fflush(stdout);
  return 0;
}

OSCServer osc_start_server(AppState *app) {
  printf("Setting up OSC server\n");
  lo_server_thread osc_server = lo_server_thread_new("7770", error);

  lo_server_thread_add_method(osc_server, "/track/action/record", "i", record_handler, app);
  lo_server_thread_add_method(osc_server, "/track/action/playback", "i", playback_handler, app);
  lo_server_thread_add_method(osc_server, "/quit", NULL, quit_handler, app);

  lo_server_thread_start(osc_server);

  return osc_server;

}

void osc_stop_server(OSCServer osc_server) {
  lo_server_thread_free(osc_server);
}
