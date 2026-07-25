---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: planning
---

# RFD 8: Taskweft NPC domain

## Problem

NPC behavior in a ticked MUD must be deterministic, auditable, and
parameterizable per scenario. Scripted state machines are too rigid;
LLM-driven NPCs are non-deterministic, expensive, and conflate the
test subject with the environment. Taskweft HTN planning produces
deterministic plans from a declarative domain description — no LLM
involved.

## Domain design

Each NPC is a set of actions and methods in a Taskweft domain:

- **Actions**: primitive operations (change trust level, give information,
  move to room, refuse to talk, reveal allegiance).
- **Methods**: compound behaviors decomposed by scenario phase
  (initial suspicion, trust-building, crisis event, revelation).
- **State**: per-NPC trust level, knowledge flags, suspicion counters,
  allegiance (hidden from the player, tracked in FDB).

The scenario (e.g., "identify the antagonist") is expressed as goals
in the domain's `todo_list`. The planner finds a sequence of NPC actions
that advances the scenario given player actions. Because the planner is
deterministic, the same player inputs always produce the same NPC
responses — essential for fair gameplay and reproducible evaluation.

## Properties

Planner produces the same plan for the same state — every run is
reproducible. Domain parameters (trust thresholds, revelation timing)
are JSON configuration, not code — scenarios are scriptable without
recompiling the server. No model inference, no API calls, no LLM
dependency anywhere in the NPC loop.

## Open questions

- One domain per NPC or one domain for all NPCs with entity parameters?
- How does the planner react when the player takes an unexpected action
  that invalidates the current plan?

## Implementation status

- [ ] NPC action primitives defined
- [ ] Trust and suspicion model
- [ ] Scenario-to-domain encoding
- [ ] Replan trigger integration
