---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: published
discussion: https://github.com/taskweft/crucible-rfd/pull/2
labels: project-definition
---

# RFD 2: Project definition

## Problem

Standard LLM benchmarks (MMLU, HumanEval) measure isolated knowledge,
not persistent social behavior. A MUD where models act as players,
navigate a town, build NPC trust, and complete open-ended objectives
offers a harder evaluation signal.

## Context

CrucibleBench (https://zenodo.org/records/21386663) demonstrates a
proof-of-concept: 13 models, 650 runs, $99.59 total cost. Medieval
town scenario with a trust/investigation objective. Two operating modes:
human-playable and LLM evaluation harness.

## See also

- **RFD 3**: Tech stack (libh2o, FoundationDB, Taskweft)
- **RFD 4**: World design and narrative
- **RFD 5**: Simulation model (ticked)
- **RFD 6**: Architecture — libh2o, FDB schema, WebSocket
- **RFD 7**: Slash command protocol
- **RFD 8**: Taskweft NPC domain
- **RFD 9**: LLM evaluation scoring protocol

## Implementation status

- [x] Project defined
- [ ] Prototype
- [ ] Human-playable alpha
- [ ] LLM evaluation harness
