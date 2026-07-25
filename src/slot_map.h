#ifndef CRUCIBLE_SLOT_MAP_H
#define CRUCIBLE_SLOT_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Slot map — O(1) entity storage with generation-aware handles.
 *
 * A slot map stores entities in a dense array (slots) for cache-friendly
 * iteration.  Each slot carries a generation counter.  A handle is an
 * index + generation pair.  Accessing a slot with a stale generation
 * returns NULL — dangling handle detection without use-after-free.
 *
 * Use slot maps for everything until they prove insufficient.
 *
 * Layout:
 *   slots[]  — dense array of { entity, generation, occupied }
 *   freelist — linked list of freed slot indices for O(1) insert
 *   count    — number of occupied slots
 *   capacity — allocated slot count (grows by 2x on insert when full)
 */

#define SLOT_MAP_GENERATION_BITS 24
#define SLOT_MAP_INDEX_BITS      40

typedef uint64_t slot_handle_t;

#define SLOT_HANDLE_NULL ((slot_handle_t)0)

static inline slot_handle_t slot_make_handle(uint64_t index, uint32_t generation) {
    return (slot_handle_t)((index) | ((uint64_t)(generation) << SLOT_MAP_INDEX_BITS));
}

static inline uint64_t slot_handle_index(slot_handle_t h) {
    return h & ((1ULL << SLOT_MAP_INDEX_BITS) - 1);
}

static inline uint32_t slot_handle_generation(slot_handle_t h) {
    return (uint32_t)(h >> SLOT_MAP_INDEX_BITS);
}

typedef struct {
    void       *entity;      /* pointer to the stored entity */
    uint32_t    generation;  /* bumped on every free */
    bool        occupied;
} slot_entry_t;

typedef struct {
    slot_entry_t *slots;
    size_t        capacity;
    size_t        count;
    size_t       *freelist;  /* stack of free slot indices */
    size_t        freelist_count;
    size_t        freelist_capacity;
} slot_map_t;

/* Lifetime */
slot_map_t *slot_map_create(size_t initial_capacity);
void        slot_map_destroy(slot_map_t *sm);

/* Insert a heap-allocated entity. Returns a handle. */
slot_handle_t slot_map_insert(slot_map_t *sm, void *entity);

/* Look up by handle. Returns NULL on stale generation or out of bounds. */
void *slot_map_get(const slot_map_t *sm, slot_handle_t handle);

/* Remove. Returns the entity pointer (caller must free). NULL if stale. */
void *slot_map_remove(slot_map_t *sm, slot_handle_t handle);

/* Iterate. Calls fn for each occupied slot. */
void slot_map_foreach(const slot_map_t *sm,
                      void (*fn)(slot_handle_t handle, void *entity, void *ctx),
                      void *ctx);

/* Current count of occupied slots. */
static inline size_t slot_map_count(const slot_map_t *sm) { return sm->count; }

#endif /* CRUCIBLE_SLOT_MAP_H */
