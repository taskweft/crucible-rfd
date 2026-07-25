---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: published
discussion: https://github.com/taskweft/crucible-rfd/pull/2
labels: project-definition
---

# RFD 2: Project definition

## Problem

Multi-user dungeons (MUDs) are a compelling game format: text-driven,
turn-structured, open-ended. But most MUDs are built with ad-hoc scripting
or state machines that are hard to configure, audit, or reproduce across
sessions. A well-architected MUD server with deterministic NPC behavior,
a clean protocol, and replayable scenarios serves two audiences:

- **Primary**: human players who want a persistent, reactive game world.
- **Secondary**: automated clients — bots, scripted agents, or LLMs —
  that interact via the same protocol for evaluation or research.

No audience should be treated as an afterthought. The core game loop
must be playable by humans with zero automation dependencies.

## Context

CrucibleBench (https://zenodo.org/records/21386663) demonstrates a
proof-of-concept: 13 models, 650 runs, $99.59 total cost. Medieval
town scenario with a trust/investigation objective. The MUD framework
underlying that experiment is being formalized here as Crucible — a
proper game server first, with an optional evaluation harness bolted on.

## See also

- **RFD 3**: Tech stack (libh2o, FoundationDB, Taskweft)
- **RFD 4**: World design and narrative
- **RFD 5**: Simulation model (ticked)
- **RFD 6**: Architecture — libh2o, FDB schema, WebSocket
- **RFD 7**: Slash command protocol
- **RFD 8**: Taskweft NPC domain
- **RFD 9**: Evaluation scoring

## Implementation status

- [x] Project defined
- [ ] Prototype
- [ ] Human-playable alpha
- [ ] Evaluation harness (optional)
