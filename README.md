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
| 0017 | TPC-C scaling | mvp |
| 0018 | Zonefabric scaling | mvp |
| 0019 | AssetcDN scaling | mvp |
| 0020 | Cassie scaling | mvp |
| 0021 | Actor-lite worker pool architecture | mvp |
| 0022 | FoundationDB selection over CockroachDB | mvp |
| 0023 | h2o TFB reference implementation reuse | mvp |
| 0024 | Three-layer verification strategy | mvp |
| 0025 | TPC-C keyspace design for FoundationDB | mvp |
| 0026 | Binary value encoding for FDB | mvp |
| 0027 | Async FDB callback chain for TPC-C transactions | mvp |
| 0028 | TPC-C data loader design | mvp |
| 0029 | Benchmark harness endpoint and transaction mix | mvp |
| 0030 | CI/CD pipeline design | mvp |
| 0031 | TechEmpower benchmark implementation | someday |
| 0032 | zstd compression for FDB values | mvp |
| 0033 | Slotmap entity storage | mvp |
| 0034 | Macaroon + eBPF/XDP security fabric | someday |
| 0035 | Plausible-witness-dag for MMO feature ablation | someday |
| 0036 | PERT critical path for zonefabric implementation | mvp |
| 0037 | CrucibleBench MUD ablation | mvp |

## Scenarios

| File | Scenario | Description |
|------|----------|-------------|
| `priv/scenarios/middleham.ex` | Middleham | 12 rooms, 4 NPCs, trust/suspicion system, 2 objectives |

Each scenario is a Taskweft DSL domain defining the room graph, NPC
state, items, and objective decompositions.

See `rfd/XXXX/README.md` for each RFD document.  Implementation schedule
at `PERT_PLAN.md`.  Benchmark code at `h2o-bench-tpcc/`.
