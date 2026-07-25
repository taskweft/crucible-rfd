---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: committed
discussion: https://github.com/taskweft/crucible-rfd/pull/3
labels: tech-stack
---

# RFD 3: Tech stack

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Transport | libh2o (dual-stack HTTP/1.1 + HTTP/3) | Single library for HTTP/1.1, HTTP/3, and WebSocket. Proven in high-throughput production. |
| State | FoundationDB | Ordered key-value store with strict serializable transactions. Schema-free schema — world state maps naturally to (room, npc, player) keys. |
| Planning | Taskweft | HTN planner with ISO 8601 temporal durations. NPC behavior expressed as goal-task networks replanned on each tick. |
| Interface | Discord-style slash commands | Low learning curve. Each command maps to a MUD action (look, move, talk, take, use). |

## Decisions

- **Ticked simulation** — fixed-rate game loop. Taskweft produces temporal
  plans with ISO 8601 durations; the runtime advances the world on each tick
  and replans when state diverges. Matches CrucibleBench's discrete turn
  boundaries.

- **Dual-stack HTTP/1.1 + HTTP/3** — WebSocket runs over HTTP/1.1 for
  broad compatibility; HTTP/3 available for low-latency clients.

- **FoundationDB over SQL** — FDB's ordered key-value model maps naturally
  to spatial and inventory state. No schema migrations. Strict serializable
  isolation without coordinator overhead.

## Implementation status

- [x] Stack selected
- [ ] libh2o compiled with WebSocket support
- [ ] FoundationDB schema design
- [ ] Taskweft NIF integration
