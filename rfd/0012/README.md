---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: planning,taskweft
stage: mvp
---

# RFD 12: Planning — Taskweft

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Planning | Taskweft | HTN planner with temporal durations. NPC behavior expressed as goal-task networks replanned on each tick. |

## Decisions

- **Taskweft over LLM-driven NPCs** — NPC behavior must be deterministic
  and auditable. LLM-driven NPCs would introduce non-determinism and
  conflate the evaluator with the environment. Taskweft HTN plans are
  deterministic, parametrizable, and reproduce identically from the same
  state. LLMs are never required for NPC logic (see RFD 2).
- **Ticked planning** — the planner runs on each simulation tick (RFD 5),
  replanning only when state diverges from the current plan.
- **Domain per scenario** — each scenario provides its own domain file
  (NPC actions, methods, state variables). The server loads the domain
  at session start.

## Integration

The planner runs as a NIF in the Elixir server process. Each tick:

1. Serialize current world state as the planner's working memory.
2. Call Taskweft to produce the next action sequence.
3. Execute the highest-priority action in the sequence.
4. If state diverges from plan, trigger a replan.

## See also

- **RFD 8**: Taskweft NPC domain (full stage — detailed domain design)
- **RFD 5**: Simulation model
- **RFD 2**: Project definition (no LLM required principle)

## Implementation status

- [x] Stack selected
- [ ] Taskweft NIF compiled
- [ ] Tick-to-planner integration
- [ ] Replan-on-divergence trigger
