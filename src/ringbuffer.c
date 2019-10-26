#include <stdlib.h>
#include <assert.h>

#include "ringbuffer.h"
#include "dbg.h"

RingBuffer *rb_create(int size) {
  RingBuffer *rb = malloc(sizeof(RingBuffer));
  check_mem(rb);

  // allocate size + 1 so that despite the slightly janky way
  // we check if the buffer is full, we still get ${size} slots
  void **buffer = malloc((size + 1) * sizeof(void *));
  check_mem(buffer);

  rb->buffer = buffer;
  for (int i = 0; i < size; i++) {
    buffer[i] = NULL;
  }
  rb->size = size;
  rb->head = 0;
  rb->tail = 0;

  return rb;
 error:
  return NULL;
}

int next_head(RingBuffer *rbuffer) {
  return (rbuffer->head >= rbuffer->size) ? 0 : rbuffer->head + 1;
}

int next_tail(RingBuffer *rbuffer) {
  return (rbuffer->tail >= rbuffer->size) ? 0 : rbuffer->tail + 1;
}

int rb_full(RingBuffer *rbuffer) {
  int next = next_tail(rbuffer);
  return __sync_bool_compare_and_swap(&(rbuffer->head), next, next);
}

int rb_empty(RingBuffer *rbuffer) {
  return __sync_bool_compare_and_swap(&(rbuffer->head), rbuffer->tail, rbuffer->tail);
}

int rb_size(RingBuffer *rbuffer) {
  int h = rbuffer->head;
  int t = rbuffer->tail;
  return (t >= h) ? t - h : (rbuffer->size - h) + t + 1;
}

int rb_push(RingBuffer *rbuffer, void *value) {
  while (1) {
    if (rb_full(rbuffer)) return -1;
    int next = next_tail(rbuffer);
    int current = rbuffer->tail;
    if (__sync_bool_compare_and_swap(
                                     &(rbuffer->tail),
                                     current,
                                     next)) {
      rbuffer->buffer[next] = value;
      return current;
    }
  }
}

void *rb_pop(RingBuffer *rbuffer) {
  while (1) {
    if (rb_empty(rbuffer)) return NULL;
    int next = next_head(rbuffer);
    int current = rbuffer->head;
    if (__sync_bool_compare_and_swap(
                                     &(rbuffer->head),
                                     current,
                                     next)) {
      return rbuffer->buffer[next];
    }
  }
  return NULL;
}

void rb_destroy(RingBuffer *rbuffer) {
  check(rbuffer != NULL, "Invalid RingBuffer");
  check(rbuffer->buffer != NULL, "Invalid buffer in RingBuffer");
  free(rbuffer->buffer);
  free(rbuffer);
  return;
 error:
  log_err("Could not clean up RingBuffer");
}
