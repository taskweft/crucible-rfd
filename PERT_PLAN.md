# Crucible — PERT Critical Path Plan

Generated from `taskweft/taskweft` planner (temporal HTN domain).

## Dependencies

- RFD 3 (World design) and RFD 4 (FDB schema) depend on RFD 2 (project def)
- Taskweft NPC domain depends on RFD 3 + RFD 4
- WebSocket handler depends on Taskweft NPC domain
- Human-playable alpha depends on all above
- Eval harness depends on alpha

## Critical path (12 days)

```mermaid
gantt
    title Crucible — Project Plan (Critical path: 12d)
    dateFormat  YYYY-MM-DD
    axisFormat  %b %d (%a)

    section Design
    RFD 3 — World design          :crit, rfd3, 2026-07-27, 2d
    RFD 4 — FDB schema            :crit, rfd4, after rfd3, 3d
    RFD 5 — Taskweft NPC domain   :rfd5, after rfd3, 2d

    section Build
    WebSocket + slash handler     :crit, ws, after rfd4, 4d
    FDB world state layer         :world, after rfd5, 3d

    section Delivery
    Human-playable alpha          :crit, alpha, after ws, 3d
    LLM eval harness              :crit, harness, after alpha, 1d
```

## Stack

- **libh2o** — dual-stack HTTP/1.1 + HTTP/3, WebSocket, slash-command dispatch
- **FoundationDB** — world state persistence
- **Taskweft** — HTN planner for NPC behavior + scenario logic
