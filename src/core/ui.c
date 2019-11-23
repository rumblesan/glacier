#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "dbg.h"

#include "core/ui.h"

UIInfo *ui_create() {
  UIInfo *ui = malloc(sizeof(UIInfo));
  check_mem(ui);

  return ui;
error:
  return NULL;
}

void ui_destroy(UIInfo *ui) {
  check(ui != NULL, "Invalid UI Structure");
  free(ui);
  return;
error:
  log_err("Could not clean up UI");
}
