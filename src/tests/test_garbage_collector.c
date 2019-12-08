#include <stdint.h>

#include "tests/minunit.h"

#include "core/garbage_collector.h"

void test_garbage_collector_create() {

  ck_ring_t *queue = malloc(sizeof(ck_ring_t));
  ck_ring_buffer_t *buffer = malloc(sizeof(ck_ring_buffer_t) * 100);
  ck_ring_init(queue, 100);

  GarbageCollector *gc = gc_create(queue, buffer);

  mu_assert(gc != NULL, "Could not create Garbage Collector");

  gc_destroy(gc);
}

void test_garbage_collector() {
  mu_run_test(test_garbage_collector_create);
}
