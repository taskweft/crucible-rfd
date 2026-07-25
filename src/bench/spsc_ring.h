#ifndef SPSC_RING_H
#define SPSC_RING_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

/* Lock-free single-producer single-consumer ring.
 * Capacity must be power of two. No CAS — only atomics. */

typedef struct {
    void **slots;
    size_t mask;
    _Atomic size_t head;
    _Atomic size_t tail;
} spsc_ring_t;

void spsc_ring_init(spsc_ring_t *r, void **slots, size_t capacity);
bool spsc_ring_push(spsc_ring_t *r, void *item);
void *spsc_ring_pop(spsc_ring_t *r);

#endif
