---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: state,foundationdb
stage: mvp
---

# RFD 11: State — FDB (linear-scaling KV)

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| State | FoundationDB | Ordered key-value store with strict serializable transactions. Schema-free — world state maps naturally to (room, npc, player) keys. Linear throughput with cluster size. |

## Keyspace

One zone (zone 0) covers the entire Middleham world. All 12 rooms
fit in a single zstd-compressed blob committed per tick:

```
cr/zone/0                  -> zstd blob: { world_snapshot }
cr/player/{pid}/{field}   -> player_t (point lookups for active players)
cr/tick/{sid}              -> uint64 (tick counter)
```

### Zone blob structure

The zone blob contains the authoritative world state:

| Field | Content |
|-------|---------|
| Room graph | 12 rooms, exits, descriptions |
| NPC state | Trust, suspicion, talk_count, marked flag |
| Item locations | Which room each item is in |
| Player state | Current room, inventory, knowledge flags |

At 64Hz tick rate, a ~200 byte blob produces 12.8KB/s write to FDB.
One key, one transaction, no range scans needed.

## Zstd compression

All zone state blobs use zstd compression (level 3 default). Compresses
in memory before the FDB set call. Decompressed on read after the FDB
get call. The zone-state keyspace (RFD 0032) describes the compression
wrapper.

## Gall's Law

Start with one zone = whole world. Split into multiple zones only when
the world grows beyond what a single zone blob can hold at 64Hz. For
the MUD POC with 12 rooms and ~30 entities, that never happens.

## See also

- **RFD 13**: FDB schema — linear-scaling key layout
- **RFD 0032**: zstd compression for FDB values
- **RFD 0018**: zonefabric scaling (multi-zone model)

## Implementation status

- [x] Stack selected
- [ ] FDB driver linked
- [ ] Zone blob read/write
- [ ] zstd compression wrapper