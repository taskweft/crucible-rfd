# Scaling Middleham — applying zonefabric to the scenario DSL

## The TPC-C lesson

TPC-C scales by W (warehouses). Each warehouse is independent:
no cross-warehouse transactions in the standard mix.  W=10 is ten
isolatable workloads.  FDB sees no conflicts across warehouses.
Linear core scaling.

Zonefabric applies the same pattern: each zone is independent
(200 entities, per-zone FDB transactions, thread-local slotmap).

## The Middleham problem

Middleham is a single scenario: 12 rooms, 4 NPCs, 14 items,
2 win conditions.  At MUD scale (single player, ~30 entities)
there's nothing to scale — it all fits in one FDB key.

The scaling comes from many Middleham-like scenarios running
concurrently, not from making Middleham bigger.

## The DSL as template, not runtime

`priv/scenarios/middleham.ex` is a **scenario template**.
It defines:

```
@variables    →  initial world state (room graph, NPC trust, items)
@actions      →  player capabilities (move, talk, take, look)
@methods      →  win-condition decompositions (trust captain, find spy)
@todo_list    →  terminal goals
```

At runtime, the template is instantiated N times across N workers:

| DSL concept | Runtime mapping |
|-------------|-----------------|
| `@variables` | Slotmap initial state (zone blob) |
| `@actions` | C handler functions in worker thread |
| `@methods` | Planner domain compiled to JSON-LD |
| One scenario | One zone in the zonefabric model |
| N scenarios | N zones × N workers = N× throughput |

## Data flow: DSL → runtime

```
middleham.ex                  (Elixir DSL source)
    │  taskweft_nif compile
    ▼
middleham.jsonld              (RECTGTN domain, planner input)
    │  loaded at server init
    ▼
┌──────────────────────────────────────────────────┐
│  Worker 0            Worker 1         Worker N   │
│  ┌──────────┐       ┌──────────┐     ┌──────────┐│
│  │ scenario │       │ scenario │     │ scenario ││
│  │ instance │       │ instance │     │ instance ││
│  │ slotmap  │       │ slotmap  │     │ slotmap  ││
│  │ planner  │       │ planner  │     │ planner  ││
│  │ FDB txn  │       │ FDB txn  │     │ FDB txn  ││
│  └──────────┘       └──────────┘     └──────────┘│
└──────────────────────────────────────────────────┘
```

## The scaling test

| Scenarios | 1 worker | 2 workers | Ratio |
|-----------|----------|-----------|-------|
| 1 | X ticks/s | X ticks/s | 1.0× |
| 10 | X | 2X | 2.0× |
| 100 | X | 2X | 2.0× |

Each scenario instance is independent (no cross-scenario NPCs,
no shared world state).  FDB key prefixes separate instances
(`cr/scenario/{id}/`).  Workers dispatch via round-robin SPSC.

## What changes in the DSL

The DSL needs one new parameter — a scale factor:

```elixir
@variables %{
  scenario_id: %{type: :int, init: 0},
  ...
}
```

At load time, the server instantiates the domain N times,
each with a different `scenario_id`.  The FDB keyspace
prefixes each scenario's state under its ID.  Workers own
scenario instances, not rooms within a scenario.

No other DSL changes needed.  The actions, methods, and
objectives are identical per instance — just parallelized.
