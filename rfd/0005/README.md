---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: committed
discussion:
labels: simulation
---

# RFD 5: Simulation model — ticked

## Model

The game runs a fixed-rate tick loop. Each tick:

1. Read pending player commands from the WebSocket buffer.
2. Apply commands to FoundationDB world state.
3. Run Taskweft planner over the current state to determine NPC actions
   and scenario progression.
4. Emit narrative response to the player.
5. Advance tick counter.

This matches how CrucibleBench evaluates models: a model has N turns
(ticks) to complete the objective. Ticked simulation makes evaluation
reproducible — same seed, same tick count, same state sequence.

## Why not tickless

Tickless (event-driven) evaluation introduces timing artifacts: a model
that churns many quick actions appears differently from one that thinks
longer between actions. Ticked removes this confound — every model gets
the same budget measured the same way.

## Tick rate

- Human-playable: 1 tick per player command (request-response).
- LLM evaluation: 1 tick per LLM action, timed from submission to
  response. A 50-turn evaluation = 50 ticks.

## Implementation status

- [x] Ticked model selected
- [ ] Tick loop in libh2o server
- [ ] Tick counter in FDB
- [ ] Replan trigger when state diverges from plan
