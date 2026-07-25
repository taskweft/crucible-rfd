# crucible-rfd

RFDs for the Crucible project — a playable MUD with deterministic NPCs,
optional automated evaluation, and an LLM-capable evaluation mode.

See `PERT_PLAN.md` for the Gall's Law implementation schedule.

## RFDs

| RFD | Title | State | Stage |
|-----|-------|-------|-------|
| 0001 | Requests for Discussion | published | mvp |
| 0002 | Project definition | published | mvp |
| 0003 | Transport — libh2o | committed | mvp |
| 0004 | World design — Middleham | ideation | mvp |
| 0005 | Simulation model — 64 Hz ticked | committed | mvp |
| 0006 | Architecture — libh2o, FDB, WebSocket | ideation | full |
| 0007 | Slash command protocol | ideation | mvp |
| 0008 | Taskweft NPC domain | ideation | full |
| 0009 | Evaluation scoring | ideation | someday |
| 0010 | Bandwidth allocation | ideation | someday |
| 0011 | State — FDB (linear-scaling KV) | ideation | mvp |
| 0012 | Planning — Taskweft | ideation | mvp |
| 0013 | FDB schema — linear-scaling key layout | ideation | full |
| 0014 | Wire format — bit-crushed binary frames | ideation | full |
| 0015 | Web client — Discord-like slash commands | ideation | mvp |
| 0016 | World output (Elixir DSL) | ideation | mvp |

## Scenarios

| File | Scenario | Description |
|------|----------|-------------|
| `priv/scenarios/middleham.ex` | Middleham | 12 rooms, 4 NPCs, trust/suspicion system, 2 objectives |

Each scenario is a Taskweft DSL domain defining the room graph, NPC
state, items, and objective decompositions.

See `rfd/XXXX/README.md` for each RFD document. Implementation schedule
at `PERT_PLAN.md`.
