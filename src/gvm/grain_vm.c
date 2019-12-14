#include <stdlib.h>

#include "dbg.h"

#include "gvm/grain_vm.h"

GrainVM *gvm_create() {
  GrainVM *gvm = calloc(1, sizeof(GrainVM));
  check_mem(gvm);

  return gvm;
error:
  return NULL;
}

void gvm_run(GrainVM *gvm, GrainVMBlock *blk) {
}

void gvm_destroy(GrainVM *gvm) {
  check(gvm != NULL, "Invalid Grain VM");
  free(gvm);
  return;
error:
  log_err("Could not clean up Grain VM");
}
