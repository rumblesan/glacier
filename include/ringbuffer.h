#ifndef __BCLIB_LF_RING_BUFFER_H__
#define __BCLIB_LF_RING_BUFFER_H__

typedef struct RingBuffer {
  int head;
  int tail;
  int size;
  void **buffer;
} RingBuffer;

int rb_full(RingBuffer *rbuffer);

int rb_empty(RingBuffer *rbuffer);

RingBuffer *rb_create(int size);

int rb_push(RingBuffer *rbuffer, void *value);

int rb_size(RingBuffer *rbuffer);

void *rb_pop(RingBuffer *rbuffer);

void rb_destroy(RingBuffer *rbuffer);

#endif
