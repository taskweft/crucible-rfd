---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: prediscussion
discussion:
labels: tech-stack
---

# RFD 3: Game engine and hosting

## Problem

Crucible needs a server that serves a text-based interactive world over a
network protocol (WebSocket or SSH). The same server must log every action
and state transition for later LLM evaluation without server-side inference.

## Context

The human-playable mode needs a real-time event loop: player connects, types
commands, server resolves actions against simulated NPCs and world state,
returns narrative text. LLM eval mode replaces the human with a scripted
caller that sends action tokens and receives observation tokens.

## Considered options

### A. Custom Elixir/Phoenix (WebSocket)

Matches the existing multiplayer-fabric infrastructure. Elixir's GenServer
maps naturally to per-player game state. Phoenix Channels for WebSocket
transport. No LLM eval path change needed — same event loop, different caller.

### B. Custom Python/ASGI (WebSocket)

Broader community for text-game tooling (evennia, later chapters of
existing MUD frameworks). Python is the evaluation language for most LLM
benchmark suites — native integration with judge models.

### C. Extend an existing MUD codebase (Evennia, CoffeeMUD, etc.)

Full world-modeling primitives out of the box (rooms, exits, NPCs,
inventory, combat). Risk: carrying decades of design assumptions (MUD
traditions) that don't serve a 60-minute evaluation scenario. Many are
Python 3 compatible.

### D. Static-render with in-memory state (Go or Rust binary)

Single binary, no database, no orchestration — matches the $10/mo constraint.
Each player gets an in-memory game session that lives as long as they're
connected. Trade-off: no persistence between sessions.

## Proposal

Start with **Option D (Go binary, in-memory state, WebSocket)**.

Rationale:
- Lowest operational cost — single binary on a $6/mo DigitalOcean droplet
- No DB, no orchestration, no cache layer. Start the binary, it listens.
- Go's goroutine-per-connection model maps well to per-player sessions
- Matches the user's existing rvk-api (Go/Vue 3) experience
- If persistence or scale is needed later, the interface is clean to swap

The initial world state is plain Go structs loaded from a JSON file shipped
with the binary — the RFD 4 world design document compiles to JSON, not SQL.

## Alternatives rejected

- **Elixir/Phoenix**: stronger alignment with multiplayer-fabric infra, but
  heavier operational cost for a single-process game. Better reserved for
  the zone-backed multiplayer version if Crucible scales.
- **Python/Evennia**: heavy framework with decades of MUD-specific design.
  Wrong abstraction level for a 60-minute evaluation scenario.

## Implementation status

- [ ] Go project scaffold with WebSocket handler
- [ ] World model: Room, Exit, Player, NPC as Go structs
- [ ] JSON world-description format
- [ ] Command parser (verb-noun, simple parser)
- [ ] Human-playable: connect, walk, look, talk
