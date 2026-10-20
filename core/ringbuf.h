#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stdint.h>

/* Single-producer / single-consumer byte ring buffer.
 * Safe between ONE producer (e.g. an ISR) and ONE consumer (e.g. a task)
 * on a single-core MCU. Capacity must be a power of two. */
typedef struct {
    uint8_t *buf;
    uint32_t mask;            /* capacity - 1 */
    volatile uint32_t head;   /* written only by the producer */
    volatile uint32_t tail;   /* written only by the consumer */
} ringbuf_t;

bool     ringbuf_init(ringbuf_t *rb, uint8_t *storage, uint32_t capacity);
bool     ringbuf_put(ringbuf_t *rb, uint8_t byte);    /* false if full  */
bool     ringbuf_get(ringbuf_t *rb, uint8_t *byte);   /* false if empty */
uint32_t ringbuf_count(const ringbuf_t *rb);
bool     ringbuf_is_empty(const ringbuf_t *rb);
bool     ringbuf_is_full(const ringbuf_t *rb);

#endif
