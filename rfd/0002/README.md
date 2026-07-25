---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: published
discussion: https://github.com/taskweft/crucible-rfd/pull/2
labels: project-definition
stage: mvp
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
must be playable by humans with zero automation dependencies. This
principle — **LLM capable, must not require an LLM** — applies to every
layer of the stack: the server does not distinguish between connection
types, NPC logic never depends on model inference, and evaluation is an
opt-in post-hoc analysis tool.

## Context

CrucibleBench (https://zenodo.org/records/21386663) demonstrates a
proof-of-concept: 13 models, 650 runs, $99.59 total cost. Medieval
town scenario with a trust/investigation objective. The MUD framework
underlying that experiment is being formalized here as Crucible — a
proper game server first, with an optional evaluation harness bolted on.

## Delivery stages

| Stage | Scope | RFDs |
|-------|-------|------|
| **MVP** | Playable website: WebSocket transport, world, tick loop, slash commands, web client, state, planning | 1–5, 7, 11, 12, 15, 16 |
| **Full** | Complete server: WebSocket handler, NPC domains, schema design, bit-crushed wire format | 6, 8, 13, 14 |
| **Someday** | Optional: evaluation scoring, bandwidth optimization | 9, 10 |

## See also

- **RFD 1**: RFD process
- **RFD 3**: Transport — libh2o
- **RFD 4**: World design and narrative
- **RFD 5**: Simulation model — 64 Hz ticked
- **RFD 6**: Architecture — libh2o server, FDB schema, WebSocket (full)
- **RFD 7**: Slash command protocol
- **RFD 8**: Taskweft NPC domain (full)
- **RFD 9**: Evaluation scoring (someday)
- **RFD 10**: Bandwidth allocation (someday)
- **RFD 11**: State layer — FoundationDB
- **RFD 12**: Planning — Taskweft
- **RFD 15**: Web client — Discord-like slash commands
- **RFD 16**: World output format
- **RFD 13**: FDB schema design (full)
- **RFD 14**: Wire format — bit-crushed binary frames (full)

## Implementation status

- [x] Project defined
- [ ] Prototype
- [ ] Human-playable alpha
- [ ] Evaluation harness (optional)
