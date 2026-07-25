#include "spsc_ring.h"
#include <assert.h>

void spsc_ring_init(spsc_ring_t *r, void **slots, size_t capacity) {
    assert(capacity > 0 && (capacity & (capacity - 1)) == 0);
    r->slots = slots;
    r->mask = capacity - 1;
    atomic_store_explicit(&r->head, 0, memory_order_relaxed);
    atomic_store_explicit(&r->tail, 0, memory_order_relaxed);
}

bool spsc_ring_push(spsc_ring_t *r, void *item) {
    size_t h = atomic_load_explicit(&r->head, memory_order_relaxed);
    size_t t = atomic_load_explicit(&r->tail, memory_order_acquire);
    if (h - t > r->mask) return false;
    r->slots[h & r->mask] = item;
    atomic_store_explicit(&r->head, h + 1, memory_order_release);
    return true;
}

void *spsc_ring_pop(spsc_ring_t *r) {
    size_t t = atomic_load_explicit(&r->tail, memory_order_relaxed);
    size_t h = atomic_load_explicit(&r->head, memory_order_acquire);
    if (t == h) return NULL;
    void *item = r->slots[t & r->mask];
    atomic_store_explicit(&r->tail, t + 1, memory_order_release);
    return item;
}
