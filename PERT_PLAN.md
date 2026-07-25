# Crucible — RFD Sprint PERT Plan

Generated from `taskweft/taskweft` planner.

## Schedule (58H serial / 48H parallel critical path)

```mermaid
gantt
    title RFD Sprint — Critical path: 48H (6 days)
    dateFormat  YYYY-MM-DD
    axisFormat  %b %d (%a)

    section Core
    RFD 3 — Tech stack            :crit, r3, 2026-07-28, 2d
    RFD 6 — Architecture          :crit, r6, after r3, 1.5d
    RFD 8 — NPC domain            :crit, r8, after r6, 1.5d
    RFD 9 — Scoring protocol      :crit, r9, after r8, 1d

    section Parallel
    RFD 5 — Ticked model          :r5, after r3, 0.5d
    RFD 7 — Slash commands        :r7, after r3, 0.75d
```

## Estimates

| RFD | Hours | Critical |
|-----|-------|----------|
| 3 — Tech stack | 16 | ✓ |
| 5 — Ticked model | 4 | — |
| 6 — Architecture | 12 | ✓ |
| 7 — Slash protocol | 6 | — |
| 8 — NPC domain | 12 | ✓ |
| 9 — Scoring | 8 | ✓ |
| **Total** | **58** | **48** |

RFD 5 and 7 run in parallel with RFD 6 (both depend on RFD 3, neither
depends on RFD 6). Planner output shows 58H serial because HTN produces
a total order; true parallel schedule is 48H on the critical path.
