# Crucible — RFD Sprint PERT Plan

Generated from `taskweft/taskweft` planner.

Calibrated against real taskweft git log velocity (Jul 14–24, 2026):
- **Tiny fix** (1–2 files, <20 lines) — ~0.5h
- **Small feature** (1–5 files, 20–100 lines) — ~0.5–1.5h
- **Medium feature** (5–15 files, 100–500 lines) — ~2–4h
- **Large feature** (10–20 files, 500–2000 lines) — ~4–6h
- **Bulk refactor** (15–40 files, 2000+ lines) — ~6–8h

Single-developer velocity (taskweft repo): peak day 41 commits (Jul 16), typical active day 8–18 commits. Averaging ~6–8 productive hours per working day after accounting for CI, context switches, and upstream NIF bumps.

## Dependency graph

```mermaid
flowchart LR
    r1[RFD 1<br>Process<br>2h] --> r2[RFD 2<br>Project def<br>3h]
    r2 --> r3[RFD 3<br>Tech stack - libh2o<br>16h]
    r2 --> r4[RFD 4<br>World design<br>6h]
    r3 --> r5[RFD 5<br>Ticked model 64Hz<br>4h]
    r3 --> r6[RFD 6<br>Architecture<br>12h]
    r3 --> r7[RFD 7<br>Slash protocol<br>6h]
    r5 --> r10[RFD 10<br>Bandwidth alloc<br>8h]
    r6 --> r8[RFD 8<br>NPC domain<br>12h]
    r6 --> r11[RFD 11<br>State - FDB<br>linear scaling<br>8h]
    r7 --> r14[RFD 14<br>Wire format<br>10h]
    r7 --> r16[RFD 16<br>World output<br>4h]
    r8 --> r9[RFD 9<br>Evaluation<br>8h]
    r8 --> r12[RFD 12<br>Planning - Taskweft<br>8h]
    r11 --> r12
    r11 --> r13[RFD 13<br>FDB schema<br>8h]
    r14 --> r15[RFD 15<br>Web client<br>16h]
    r16 --> r15

    style r3 fill:#f96,stroke:#333
    style r6 fill:#f96,stroke:#333
    style r7 fill:#f96,stroke:#333
    style r8 fill:#f96,stroke:#333
    style r12 fill:#f96,stroke:#333
    style r15 fill:#f96,stroke:#333
```

## Schedule (131H serial / 53H parallel critical path)

```mermaid
gantt
    title RFD Sprint — Critical path: 53H (~7 working days)
    dateFormat  YYYY-MM-DD
    axisFormat  %b %d (%a)

    section Foundation
    RFD 1 — Process               :crit, r1, 2026-07-28, 1d
    RFD 2 — Project definition    :crit, r2, after r1, 1d

    section Core
    RFD 3 — Tech stack (libh2o)   :crit, r3, after r2, 2d

    section Parallel (MVP)
    RFD 4 — World design          :r4, after r2, 1d
    RFD 5 — Ticked model          :r5, after r3, 0.5d
    RFD 7 — Slash protocol        :crit, r7, after r3, 0.75d

    section Architecture
    RFD 6 — Architecture (full)   :crit, r6, after r3, 1.5d

    section State
    RFD 11 — State — FDB (linear-scaling KV)  :r11, after r6, 1d
    RFD 13 — FDB schema — linear-scaling key layout    :r13, after r11, 1d

    section NPC / Planning
    RFD 8 — NPC domain (full)     :crit, r8, after r6, 1.5d
    RFD 12 — Planning — Taskweft  :crit, r12, after r8, 1d

    section Protocol / Client
    RFD 14 — Wire format (full)   :crit, r14, after r7, 1.5d
    RFD 16 — World output format  :crit, r16, after r7, 0.5d
    RFD 15 — Web client           :crit, r15, after r14, 2d

    section Evaluation (someday)
    RFD 9 — Evaluation scoring    :r9, after r8, 1d
    RFD 10 — Bandwidth allocation  :r10, after r5, 1d
```

## Estimates

| RFD | Title | Hours | Stage | Critical | Depends on |
|-----|-------|-------|-------|----------|------------|
| 1 | Process | 2 | mvp | ✓ | — |
| 2 | Project definition | 3 | mvp | ✓ | 1 |
| 3 | Tech stack — libh2o | 16 | mvp | ✓ | 2 |
| 4 | World design and narrative | 6 | mvp | — | 2 |
| 5 | Simulation — 64 Hz ticked | 4 | mvp | — | 3 |
| 6 | Architecture — libh2o, FDB, WS | 12 | full | ✓ | 3 |
| 7 | Slash command protocol | 6 | mvp | ✓ | 3 |
| 8 | NPC domain — Taskweft | 12 | full | ✓ | 6 |
| 9 | Evaluation scoring | 8 | someday | — | 8 |
| 10 | Bandwidth allocation | 8 | someday | — | 5 |
| 11 | State — FDB (linear-scaling KV) | 8 | mvp | — | 6 |
| 12 | Planning — Taskweft | 8 | mvp | ✓ | 8, 11 |
| 13 | FDB schema — linear-scaling key layout | 8 | full | — | 11 |
| 14 | Wire format — bit-crushed | 10 | full | ✓ | 7 |
| 15 | Web client — Discord-like | 16 | mvp | ✓ | 14, 16 |
| 16 | World output format | 4 | mvp | ✓ | 7 |
| | **Total** | **131** | | **53** | |

Critical path (53h): 1 → 2 → 3 → 7 → 14 → 15 (transport/client track).
Second critical path (53h): 1 → 2 → 3 → 6 → 8 → 12 (architecture/NPC/planning track).

Serial 131h is the HTN total order. True parallel schedule is **53h on the critical path** — ~7 working days at 8h/day. Both critical paths converge simultaneously because RFD 12 (planning) and RFD 15 (web client) finish at the same wall-clock offset when their longest dependency branches are followed.

## Calibration notes

All estimates calibrated against taskweft git log commits authored by K. S. Ernest (iFire) Lee between Jul 14–24, 2026:

| Calibrated artifact | taskweft commit | Actual scope | Estimate |
|---|---|---|---|
| Taskweft NIF integration (RFD 12 proxy) | `#152` fold taskweft_mcp | 73 files, 34.6K ins | >8h bulk (monorepo fold, not comparable) |
| Medium feature engineering | `#190` SafeParser fix | 10 files, 562 ins | ~3h |
| Large feature engineering | `#181` Feature/dsl | 18 files, 1721 ins | ~5h |
| Property tests | `#121` loader ref-check | 1 file, 85 ins | ~1h |
| Documentation / CITATION.cff | `#160`–`#161` | 3–4 files, 244 ins | ~2h |
| Schema enforcement | `#130` JSON Schema | 4 files, 253 ins | ~3h |
