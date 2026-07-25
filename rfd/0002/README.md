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
open-ended objectives offers a harder, more realistic signal.

## Context

CrucibleBench (https://zenodo.org/records/21386663) demonstrates a
proof-of-concept: 13 models scored over 650 runs in a single-player MUD
at $99.59 total cost. Medieval town scenario: gain the Watch's trust or
identify which NPC secretly sides with an antagonist faction.

The user-directed tech stack: **libh2o** (HTTP/2, WebSocket, slash-command
dispatch), **FoundationDB** (world state persistence), **Taskweft** (HTN
planner for NPC behavior and scenario logic). Discord-style slash commands
as the player interface.

Without the runtime LLM, the server can run on ~$10/month infrastructure.
With an LLM backend, it serves as a reproducible evaluation harness.

## Proposal

Build Crucible in two modes:

1. **Human-playable** — Discord-style slash commands over WebSocket via
   libh2o. FoundationDB for persistent world state. Taskweft plans NPC
   responses and scenario progression.
2. **LLM evaluation harness** — same world, same stack. The game engine
   logs all state transitions; LLM-as-player is scored by a separate judge.

## Open questions

- FoundationDB schema for world state (rooms, NPCs, player inventory)?
- Taskweft domain for NPC behavior vs. scenario scripting?
- Scoring dimensions beyond binary objective completion?
- Slash command language design for MUD interaction?

## Implementation status

- [ ] Game world design document (RFD 4)
- [ ] FoundationDB schema
- [ ] Taskweft NPC domain prototype
- [ ] libh2o WebSocket + slash command handler
- [ ] Human-playable alpha
- [ ] LLM evaluation harness

