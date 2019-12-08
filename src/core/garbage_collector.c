#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "ck_ring.h"

#include "dbg.h"

#include "core/garbage_collector.h"
#include "core/control_message.h"

GarbageCollector *gc_create(ck_ring_t *gc_queue, ck_ring_buffer_t *gc_queue_buffer) {
  GarbageCollector *gc = malloc(sizeof(GarbageCollector));
  check_mem(gc);

  check(gc_queue != NULL, "Invalid message queue");
  check(gc_queue_buffer != NULL, "Invalid message queue buffer");
  gc->queue = gc_queue;
  gc->queue_buffer = gc_queue_buffer;

  gc->running = false;

  return gc;
error:
  return NULL;
}

void *garbage_collector_func(void *_gc) {
  GarbageCollector *gc = _gc;

  struct timespec tim, tim2;
  tim.tv_sec = 5;
  tim.tv_nsec = 0;

  ControlMessage *new_control_message = NULL;

  while(gc->running) {
    while (
      ck_ring_dequeue_spsc(
        gc->queue,
        gc->queue_buffer,
        &new_control_message
      ) == true
    ) {
      cm_destroy(new_control_message);
    }
    sched_yield();
    nanosleep(&tim, &tim2);
  }
  return NULL;
}

bool gc_start(GarbageCollector *gc) {
  gc->running = true;

  debug("Starting garbage collector\n");
  check(!pthread_attr_init(&gc->garbage_thread_attr),
        "Error setting garbage collector thread attributes");
  check(
    !pthread_attr_setdetachstate(
      &gc->garbage_thread_attr, PTHREAD_CREATE_DETACHED
    ),
    "Error setting garbage collector thread detach state"
  );
  check(!pthread_create(&gc->garbage_thread,
                        &gc->garbage_thread_attr,
                        &garbage_collector_func,
                        gc),
        "Error creating garbage collector thread");
  return true;
error:
  return false;
}

void gc_destroy(GarbageCollector *gc) {
  check(gc != NULL, "Invalid Garbage Collector");
  gc->running = false;
  if (gc->garbage_thread != NULL) {
    pthread_cancel(gc->garbage_thread);
  }
  free(gc);
  return;
error:
  log_err("Could not clean up Garbage Collector");
}
