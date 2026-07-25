---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: state,database
stage: full
---

# RFD 13: FDB schema design

## Key layout

FoundationDB stores ordered key-value pairs. The world state maps to
key prefixes with JSON values:

| Key prefix | Value | Description |
|------------|-------|-------------|
| `room/{id}` | JSON | Room description, exits, items |
| `npc/{id}` | JSON | NPC state, trust level, knowledge flags |
| `player/{id}` | JSON | Player inventory, location, session flags |
| `obj/{id}` | JSON | Object properties, current location |
| `tick/{session}` | int | Global tick counter per session |
| `session/{id}/player` | JSON | Player-to-session mapping |
| `session/{id}/tick` | int | Per-session tick counter |

## Transaction boundaries

Each tick is a single FDB transaction:

1. Read all keys touched this tick (player location, visible rooms, NPCs)
2. Apply player command effects
3. Run Taskweft planner (outside FDB)
4. Write updated keys

FDB strict serializable isolation guarantees that concurrent ticks on
different shards do not interleave — each tick sees a consistent snapshot.

## See also

- **RFD 11**: State layer — FoundationDB (MVP — stack decision)
- **RFD 6**: Architecture — libh2o, FDB, WebSocket

## Implementation status

- [ ] Key prefix documentation
- [ ] Transaction boundary implementation
- [ ] Range scan patterns for room visibility
