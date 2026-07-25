You're right — one zone covers all 12 rooms. Middleham is small enough that per-room persistence is fragmentation, not scaling.

**Zonefabric adapted for Crucible:**

```
cr/zone/0                  -> zstd blob: { world_snapshot }
cr/player/{pid}/{field}   -> player_t (point lookups for active players)
cr/tick/{sid}              -> uint64
```

One zone (zone 0) = entire Middleham. The zone blob at tick-commit time contains:
- Room graph (12 rooms, exits, descriptions)
- NPC state (trust, suspicion, talk_count, marked)
- Item locations (which room each item is in)
- Player state (current room, inventory, knowledge)

For a MUD POC with ~30 total entities across 12 rooms, a single zstd-compressed blob per tick is ~200 bytes. At 64Hz, 200 bytes × 64 = 12.8KB/s write to FDB. One key, one transaction, no range scans needed.

Per-entity keys (`cr/player/{pid}/`) exist for point lookups — getting a player's room when they reconnect. The zone blob is the authoritative world state.

This matches Gall's Law: start with the simplest possible model (one zone = whole world), then split into multiple zones only when the world grows beyond what a single zone blob can hold at 64Hz. At 12 rooms that's never going to be a problem for the MUD.