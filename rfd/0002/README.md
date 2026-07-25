---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: project-definition
---

# RFD 2: Crucible — a playable MUD for evaluating LLMs

## Problem

LLM evaluation benchmarks saturate quickly and correlate weakly with
real-world capability. A persistent text-world (MUD) where models act as
players, navigate a simulated town, build trust with NPCs, and complete
open-ended objectives offers a harder, more realistic signal — but no
reproducible, cost-efficient implementation exists.

## Context

CrucibleBench (https://github.com/CrucibleBench/CrucibleBench_Phase1,
arXiv paper at https://zenodo.org/records/21386663) demonstrates a
proof-of-concept: 13 language models scored over 650 runs in a single-player
MUD at $99.59 total evaluation cost. The central finding is that model
rankings are highly sensitive to judge design.

The MUD scenario: a medieval town where the player must either gain the
Watch's trust or identify which NPC secretly sides with an antagonist faction.

Without the runtime LLM (i.e. as a human-playable game), the server can run
on ~$10/month infrastructure. With an LLM backend, it serves as a reproducible
evaluation harness.

## Proposal

Build Crucible as two operating modes:

1. **Human-playable mode** — a text MUD served via WebSocket/SSH, $10/mo
   hosting (DigitalOcean or Fly.io). Players explore the town, interact with
   NPCs, and complete the trust/investigation objective.
2. **LLM evaluation harness mode** — the same game world serves as an
   evaluation environment. The game engine logs all state transitions and
   actions; LLM-as-player is scored by a separate judge model on trust
   acquisition, information discovery, and objective completion.

## Open questions

- Game engine: custom Python/Elixir, or extend an existing MUD codebase?
- NPC AI: scripted state machines or LLM-driven? (If LLM-driven in eval mode,
   judge contamination risk.)
- Scoring: what dimensions beyond binary objective completion?
- Persistence: per-session disposable worlds or persistent shared state?

## Implementation status

- [ ] Game world design document
- [ ] Tech stack decision
- [ ] Prototype: single room, single NPC, one objective
- [ ] Human-playable alpha
- [ ] LLM evaluation harness
