#pragma once

#include <pthread.h>
#include <stdbool.h>

#include "ck_ring.h"

typedef struct GarbageCollector {

  bool running;

  ck_ring_buffer_t *queue_buffer;
  ck_ring_t *queue;

  pthread_t garbage_thread;
  pthread_attr_t garbage_thread_attr;

} GarbageCollector;

GarbageCollector *gc_create(ck_ring_t *gc_queue, ck_ring_buffer_t *gc_queue_buffer);

bool gc_start(GarbageCollector *gc);

void gc_destroy(GarbageCollector *gc);
