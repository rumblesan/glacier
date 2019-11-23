#include <stdint.h>

#include "tests/minunit.h"

#include "core/ui.h"

void test_ui_create() {

  UIInfo *ui = ui_create();
  mu_assert(ui != NULL, "Could not create UI");

  ui_destroy(ui);
}

void test_ui() {
  mu_run_test(test_ui_create);
}
