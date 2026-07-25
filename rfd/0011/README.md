---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: state,foundationdb
stage: mvp
---

# RFD 11: State layer — FoundationDB

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| State | FoundationDB | Ordered key-value store with strict serializable transactions. Schema-free — world state maps naturally to (room, npc, player) keys. |

## Decisions

- **FoundationDB over SQL** — FDB's ordered key-value model maps naturally
  to spatial and inventory state. No schema migrations. Strict serializable
  isolation without coordinator overhead.
- **World state as key-value tuples** — each entity type occupies a key
  prefix, making range scans efficient for tick-wide operations.
- **Tick counter as a monotonic key** — global tick value stored and
  incremented atomically per tick.

## Key prefixes

| Prefix | Content | Description |
|--------|---------|-------------|
| `room/` | Room JSON | Room description, exits, items |
| `npc/`  | NPC JSON | NPC state, trust, knowledge flags |
| `player/` | Player JSON | Inventory, location, flags |
| `obj/`  | Object JSON | Item properties, location |
| `tick/` | int | Global tick counter |

## See also

- **RFD 13**: FDB schema design (full stage — detailed key layout)
- **RFD 5**: Simulation model (tick counter)
- **RFD 3**: Transport (libh2o)

## Implementation status

- [x] Stack selected
- [ ] FDB driver linked
- [ ] Basic key-value read/write
