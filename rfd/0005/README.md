---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: committed
discussion:
labels: simulation
---

# RFD 5: Simulation model — 64 Hz ticked

## Motivation

A fixed-rate tick loop eliminates the confound between wall-clock timing
and simulation determinism. 64 Hz is the canonical game-server tick rate —
fast enough for responsive gameplay and smooth physics, slow enough that
each tick has room for planner invocation, FDB transactions, and LLM
inference over the world state.

## Model

The simulation advances at a fixed 64 ticks per second. Each tick:

1. Read pending player commands from the WebSocket buffer (accumulated
   since the last tick).
2. Apply commands to FoundationDB world state.
3. Run the Taskweft planner over the current state to determine NPC
   actions and scenario progression.
4. Emit narrative output to the player.
5. Increment the tick counter.

A tick that finishes early sleeps the remainder of its 15.625 ms budget.
A tick that exceeds budget queues into the next tick — the simulation
slows down transparently but never drops work or runs catch-up frames
that skip ticks.

This matches how CrucibleBench evaluates models: a model has N ticks
(not wall-clock seconds) to complete the objective. Reproducibility
follows — same seed, same tick count, same state sequence — regardless
of the hardware or LLM latency.

## Why 64 Hz

- **Determinism**: The tick counter is the only clock the simulation
  sees. Wall-clock jitter, network latency, and LLM response time are
  all external noise that the tick loop isolates from world state.
- **Responsiveness**: At 64 Hz input latency is bounded by
  15.6 ms — well under the 100 ms human-perceptible threshold.
- **Budget**: Each 15.6 ms tick gives the planner and the LLM a
  predictable window. Slower planners can run every Nth tick; faster
  ones run every tick.
- **Consistency**: The same 64 Hz rate used by Godot physics, common
  dedicated-game-server loops, and the Elixir VR MMOG server. One rate
  across the stack means no translation layers between tick domains.

## Why not tickless

Tickless (event-driven) evaluation introduces timing artifacts: a model
that churns many quick actions appears differently from one that thinks
longer between actions. Wall-clock time is noisy — GC pauses, network
spikes, and scheduler jitter all add variance. A 64 Hz ticked loop
removes these confounds: every model gets the same budget measured the
same way, in ticks.

## Tick rate comparison

| Approach | Latency bound | Deterministic | Evaluator-friendly | Implementation |
|---|---|---|---|---|
| 64 Hz fixed tick | 15.6 ms | ✅ | ✅ | Simple loop + sleep |
| Request-response | Variable | ❌ | ✅ | Easy but noisy |
| Tickless event | Variable | ❌ | ❌ | Most complex |

## Implementation status

- [x] Ticked (64 Hz) model selected
- [ ] Tick loop in libh2o server runs at 64 Hz
- [ ] Tick counter stored in FDB
- [ ] Over-budget tick transparently queues into next tick
- [ ] Replan trigger fires when state diverges from plan
