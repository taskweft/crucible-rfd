---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: architecture
---

# RFD 6: Architecture — libh2o, FDB schema, WebSocket

## Components

### libh2o HTTP/3 + WebSocket server

- Dual-stack listener on port 443 (HTTP/3 QUIC) and 8080 (HTTP/1.1).
- WebSocket upgrade path for slash-command transport.
- Static files and API routes share the same server — no reverse proxy.

### FoundationDB schema

World state layout:

| Key prefix | Value | Description |
|------------|-------|-------------|
| `room/{id}` | JSON | Room description, exits, items |
| `npc/{id}` | JSON | NPC state, trust level, knowledge |
| `player/{id}` | JSON | Player inventory, location, flags |
| `obj/{id}` | JSON | Object properties, location |
| `tick/{id}` | int | Global tick counter per session |

FDB transactions guarantee atomic reads and writes across all keys
for a given tick. No migration overhead — schema is application-enforced.

### WebSocket + slash command handler

- Client connects via WebSocket upgrade.
- Server sends `>> ` prompt on each tick.
- Client sends `/command [args]`.
- Server validates, dispatches to Taskweft planner, applies state changes,
  returns narrative text.

## Open questions

- Session lifecycle: disposable per-run or persistent across connections?
- Tick rate: server-determined or client-requested?

## Implementation status

- [ ] libh2o WebSocket handler
- [ ] FDB schema design doc
- [ ] Command parser and dispatcher
- [ ] Session manager
