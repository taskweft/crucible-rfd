# crucible-rfd

RFDs for the Crucible project — a playable MUD with deterministic NPCs,
optional automated evaluation, and an LLM-capable evaluation mode.

See `PERT_PLAN.md` for the Gall's Law implementation schedule.

## RFDs

| RFD | Title | Stage |
|-----|-------|-------|
| 0001 | Requests for Discussion | mvp |
| 0002 | Project definition | mvp |
| 0003 | Transport — libh2o | mvp |
| 0004 | World design — Middleham | mvp |
| 0005 | Simulation model — 64 Hz ticked | mvp |
| 0006 | Architecture — C server, libh2o, FDB, WebSocket | full |
| 0007 | Slash command protocol | mvp |
| 0008 | Taskweft NPC domain | full |
| 0009 | Evaluation scoring | someday |
| 0010 | Bandwidth allocation | someday |
| 0011 | State — FDB (linear-scaling KV) | mvp |
| 0012 | Planning — Taskweft (C FFI) | mvp |
| 0013 | FDB schema — linear-scaling key layout | full |
| 0014 | Wire format — bit-crushed binary frames | full |
| 0015 | Web client — Discord-like slash commands | mvp |
| 0016 | World output (C structs) | mvp |
| 0017 | CI/CD pipeline | mvp |
| 0018 | Zonefabric scaling (reference) | mvp |
| 0019 | Deployment — Docker + Fly.io | mvp |
| 0020 | Vendored dependencies | mvp |
| 0021 | Actor-lite worker pool (reference) | mvp |
| 0022 | FDB selection (reference) | mvp |
| 0025 | FDB keyspace design (reference) | mvp |
| 0032 | Zstd compression (reference) | mvp |
| 0033 | Slotmap entity storage (reference) | mvp |

## Scenarios

| File | Scenario | Description |
|------|----------|-------------|
| `priv/scenarios/middleham.ex` | Middleham | 12 rooms, 4 NPCs, trust/suspicion system, 2 objectives |

## Implementation

- `h2o-bench-tpcc/` — benchmark code: SPSC ring, worker pool, main.c
- `vendor/h2o/` — vendored libh2o source (git subtree)
- `priv/scenarios/` — scenario definitions (Taskweft DSL)

See `rfd/XXXX/README.md` for each RFD document.
