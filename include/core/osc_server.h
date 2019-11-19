#pragma once

#include "lo/lo.h"

#include "core/app.h"

typedef lo_server_thread OSCServer;

OSCServer osc_start_server(AppState *app);

void osc_stop_server(OSCServer osc_server);
