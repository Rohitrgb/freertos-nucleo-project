#include "ringbuf.h"
#include <stddef.h>

/* Stops the compiler reordering the data access past the index update. */
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

static bool is_power_of_two(uint32_t x)
{
    return x != 0U && (x & (x - 1U)) == 0U;
}

bool ringbuf_init(ringbuf_t *rb, uint8_t *storage, uint32_t capacity)
{
    if (rb == NULL || storage == NULL || !is_power_of_two(capacity)) {
        return false;
    }
    rb->buf  = storage;
    rb->mask = capacity - 1U;
    rb->head = 0U;
    rb->tail = 0U;
    return true;
}

uint32_t ringbuf_count(const ringbuf_t *rb)
{
    return rb->head - rb->tail;   /* unsigned maths survives wraparound */
}

bool ringbuf_is_empty(const ringbuf_t *rb)
{
    return rb->head == rb->tail;
}

bool ringbuf_is_full(const ringbuf_t *rb)
{
    return ringbuf_count(rb) == (rb->mask + 1U);
}

bool ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
    if (ringbuf_is_full(rb)) {
        return false;
    }
    rb->buf[rb->head & rb->mask] = byte;
    COMPILER_BARRIER();            /* write the data BEFORE publishing it */
    rb->head = rb->head + 1U;
    return true;
}

bool ringbuf_get(ringbuf_t *rb, uint8_t *byte)
{
    if (byte == NULL || ringbuf_is_empty(rb)) {
        return false;
    }
    *byte = rb->buf[rb->tail & rb->mask];
    COMPILER_BARRIER();            /* read the data BEFORE freeing the slot */
    rb->tail = rb->tail + 1U;
    return true;
}
