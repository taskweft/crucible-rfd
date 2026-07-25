#ifndef CRUCIBLE_SHRED_H
#define CRUCIBLE_SHRED_H

/*
 * Sharded entity store with FoundationDB consensus.
 *
 * A "shred" is a shard × entity-id pair.  Each worker thread owns
 * one shard — its own slot map, its own SPSC dispatch ring, its own
 * FDB database handle.  The handle encodes (shard << 48 | slot << 24 | gen)
 * for O(1) routing: extract shard from handle, direct-index into the
 * owning thread's slot array.  No cross-thread locks on the hot path.
 *
 * Linear scaling: N workers × N FDB cluster nodes = N² throughput
 * (worker-local slot maps are memory-bound; FDB transactions are
 *  network-bound; they scale independently).
 *
 * Two backends (no custom database code):
 *   1. fdb    — primary.  Each tick commits mutations through an FDB
 *               strict-serializable transaction.  Linear scaling with
 *               cluster size.
 *   2. local  — single-process fallback for dev and testing.  Same
 *               API, no network, no persistence.  Tick-versioned
 *               snapshots for sequential consistency.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ── Handle layout ──────────────────────────────────────────────────
 *  63      48 47            24 23             0
 * ┌──────────┬────────────────┬──────────────────┐
 * │  shard   │  slot index    │   generation     │
 * │  16 bits  │  24 bits       │   24 bits        │
 * └──────────┴────────────────┴──────────────────┘
 */
#define SHRED_SHARD_BITS      16
#define SHRED_INDEX_BITS      24
#define SHRED_GEN_BITS        24
#define SHRED_MAX_SHARDS      (1ULL << SHRED_SHARD_BITS)
#define SHRED_MAX_SLOTS       (1ULL << SHRED_INDEX_BITS)

typedef uint64_t shred_handle_t;
#define SHRED_HANDLE_NULL ((shred_handle_t)0)

static inline shred_handle_t shred_make(uint16_t shard, uint32_t slot, uint32_t gen) {
    return ((shred_handle_t)shard << 48)
         | ((shred_handle_t)(slot & (SHRED_MAX_SLOTS - 1)) << 24)
         | (gen & 0xFFFFFF);
}
static inline uint16_t shred_shard(shred_handle_t h) { return (uint16_t)(h >> 48); }
static inline uint32_t shred_slot(shred_handle_t h)  { return (uint32_t)((h >> 24) & (SHRED_MAX_SLOTS - 1)); }
static inline uint32_t shred_gen(shred_handle_t h)   { return (uint32_t)(h & 0xFFFFFF); }

/* ── Slot — 16-byte inline entity ──────────────────────────────────── */

typedef struct {
    uint64_t data[2];       /* inline entity, no heap alloc */
    uint32_t generation;
    uint16_t shard;
    uint16_t flags;         /* bit 0 = occupied */
} shred_slot_t;

/* ── Per-worker shard ──────────────────────────────────────────────── */

typedef struct {
    shred_slot_t *slots;
    size_t        capacity;
    size_t        count;
    size_t       *freelist;
    size_t        freelist_len;
    uint16_t      shard_id;
} shred_shard_t;

/* ── Mutation batch — collected during one tick ────────────────────── */

typedef struct {
    shred_handle_t *inserts;
    uint64_t       *insert_data;
    size_t          insert_count;
    shred_handle_t *removes;
    size_t          remove_count;
    shred_handle_t *updates;
    uint64_t       *update_data;
    size_t          update_count;
    uint64_t        tick_id;
} shred_tick_t;

/* ── Backend (FDB or local) ────────────────────────────────────────── */

typedef enum { SHRED_BACKEND_FDB, SHRED_BACKEND_LOCAL } shred_backend_kind_t;

typedef struct shred_backend_t {
    shred_backend_kind_t kind;
    int (*commit)(struct shred_backend_t *b, shred_tick_t *tick);
    /* FDB config — pass NULL config for local fallback */
    struct { const char *cluster_file; } fdb;
} shred_backend_t;

/* ── Top-level shred map ───────────────────────────────────────────── */

typedef struct {
    shred_shard_t  *shards;
    uint16_t        shard_count;
    shred_backend_t backend;
} shred_map_t;

/* ── API ───────────────────────────────────────────────────────────── */

/* Create.  fdb_config = NULL for local fallback. */
shred_map_t *shred_create(uint16_t shard_count, size_t slots_per_shard,
                           const char *fdb_cluster_file);
void         shred_destroy(shred_map_t *m);

/* CRUD — buffers locally, nothing hits backend until shred_tick. */
shred_handle_t shred_insert(shred_map_t *m, const void *entity);
void *shred_get(const shred_map_t *m, shred_handle_t h);
bool  shred_remove(shred_map_t *m, shred_handle_t h, void *out);
bool  shred_update(shred_map_t *m, shred_handle_t h, const void *entity);

/* Commit all buffered mutations through the backend.  Called every tick. */
int shred_tick(shred_map_t *m, uint64_t tick_id);

/* Iterate. */
typedef void (*shred_foreach_fn)(uint16_t shard, const void *entity, void *ctx);
void shred_foreach(const shred_map_t *m, shred_foreach_fn fn, void *ctx);

/* Counts. */
size_t shred_count(const shred_map_t *m, uint16_t shard);
size_t shred_total(const shred_map_t *m);

#endif /* CRUCIBLE_SHRED_H */
