# Crucible — Stack Decision: StarVote Scoring

## The two axes

| | **Keep Elixir** | **Drop Elixir (C/C++)** |
|---|---|---|
| **Stay with libh2o** | A — Elixir + libh2o NIF | B — C server + libh2o direct |
| **Replace libh2o (Bandit)** | C — Elixir + Bandit (no NIF) | D — C server + raw sockets |

Only one combination wins.

## Criteria (0–5, higher = better)

### 1 — Transport maturity
| Option | Score | Why |
|--------|-------|-----|
| A | 3 | libh2o is proven but needs NIF bindings from Elixir (custom work) |
| B | 5 | libh2o direct C API, zero indirection |
| C | 4 | Bandit is pure Elixir, well-tested, no C deps. HTTP/1.1 only |
| D | 1 | No HTTP/3 without extra library, reinventing wheels |

### 2 — Taskweft integration
| Option | Score | Why |
|--------|-------|-----|
| A | 5 | Taskweft is Elixir, direct `Taskweft.DSL.compile/1` |
| B | 2 | Need C FFI to call Taskweft NIF, or embed the planner |
| C | 5 | Native Elixir, no bridge |
| D | 1 | Embed or rewrite the planner — maximum cost |

### 3 — FDB integration
| Option | Score | Why |
|--------|-------|-----|
| A | 4 | `fdb_elixir` hex package exists, one NIF boundary |
| B | 5 | FDB C API directly |
| C | 4 | Same `fdb_elixir` as A |
| D | 5 | FDB C API directly |

### 4 — Development velocity
| Option | Score | Why |
|--------|-------|-----|
| A | 5 | Elixir hot-reload, REPL, GenServers match tick loop pattern |
| B | 2 | C/C++ iteration is slower, no hot-reload |
| C | 5 | Same Elixir velocity as A |
| D | 1 | Raw C, everything hand-rolled |

### 5 — Deploy simplicity
| Option | Score | Why |
|--------|-------|-----|
| A | 3 | Elixir release + FDB + libh2o NIF (3 components) |
| B | 5 | Single C binary + FDB (2 components) |
| C | 3 | Elixir release + FDB (2 components, simpler than A) |
| D | 5 | Single C binary + FDB |

### 6 — NIF/bridge count (fewer = better)
| Option | Score | Why |
|--------|-------|-----|
| A | 2 | libh2o NIF + FDB NIF = 2 bridges |
| B | 4 | Only Taskweft FFI (if kept) |
| C | 4 | Only FDB NIF = 1 bridge ✓ |
| D | 5 | Zero bridges |

### 7 — HTTP/3 path
| Option | Score | Why |
|--------|-------|-----|
| A | 5 | libh2o provides HTTP/3 today |
| B | 5 | libh2o provides HTTP/3 today |
| C | 1 | No HTTP/3 without adding libh2o later |
| D | 3 | Could add libh2o later, but C anyway |

## Scores

| Criterion | A | B | C | D |
|-----------|---|---|---|---|
| Transport maturity | 3 | 5 | 4 | 1 |
| Taskweft integration | 5 | 2 | 5 | 1 |
| FDB integration | 4 | 5 | 4 | 5 |
| Development velocity | 5 | 2 | 5 | 1 |
| Deploy simplicity | 3 | 5 | 3 | 5 |
| NIF/bridge count | 2 | 4 | 4 | 5 |
| HTTP/3 path | 5 | 5 | 1 | 3 |
| **Total** | **27** | **28** | **26** | **21** |

## Recommendation

**B — C server + libh2o direct** wins by 1 point.

The deciding factors are deploy simplicity (single binary) and zero NIF
bridges for the two main dependencies (libh2o + FDB both expose direct C
APIs). The cost is losing Elixir's development velocity, but libh2o's
C API is straightforward — connection-per-thread, callback-driven — and
the 64Hz tick loop is ~50 lines in any language.

Consequences of the choice:

- **RFD 16 (World output)**: becomes C structs with a serializer, not
  Elixir DSL structs. The `WorldOutput.Encoder` protocol becomes a C
  function table.
- **RFD 12 (Planning — Taskweft)**: the planner runs via C FFI to the
  existing Taskweft NIF, or is embedded as a C library.
- **RFD 14 (Wire format)**: unchanged — bit-crushed binary frames
  are language-agnostic.
- **RFD 5 (Tick loop)**: a simple `while (running) { sleep(15.6ms); }`
  loop in C calling libh2o's event loop.

If development velocity proves to be the bottleneck, Option C (Elixir +
Bandit, dropping libh2o) scores only 2 points behind and removes the
largest NIF bridge. The HTTP/3 gap is real but irrelevant until the
MVP ships.
