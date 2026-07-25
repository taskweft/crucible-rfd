---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: architecture
stage: full
---

# RFD 6: Architecture — C server, libh2o, FDB, WebSocket

## Stack

| Layer | Choice | Interface |
|-------|--------|-----------|
| Server runtime | C (c17) | `main()` with libh2o event loop integration |
| Transport | libh2o | Direct C API (`h2o_evloop_t`, WebSocket handlers) |
| State | FoundationDB | Direct C API (`fdb_c.h`) |
| Planning | Taskweft | C FFI to the Taskweft NIF shared object |
| Wire format | Bit-crushed binary | C struct serializers (RFD 14) |
| World output | C structs | `WorldOutput` struct with serializer function table (RFD 16) |
| Web client | Static HTML/JS | Served by libh2o, connects via WebSocket |

No Elixir runtime. No NIF bridges on the hot path. Everything is C,
linked directly against libh2o and the FDB C client. See
`STACK_DECISION.md` for the full StarVote scoring.

## Components

### libh2o HTTP/3 + WebSocket server

- Dual-stack listener on port 443 (HTTP/3 QUIC) and 8080 (HTTP/1.1).
- WebSocket upgrade path for slash-command transport.
- Static files and API routes share the same server — no reverse proxy.
- Event loop runs inside the 64Hz tick loop (RFD 5): each tick calls
  `h2o_evloop_run()` to process I/O, then advances the simulation.

### Server main loop

```c
while (running) {
    // Phase 1: drain network I/O
    h2o_evloop_run(evloop, 0);

    // Phase 2: read pending player commands from WebSocket buffers
    for (each buffered command) {
        parse_slash_command(command);
        dispatch_to_planner(command);
    }

    // Phase 3: run FDB transaction for this tick
    fdb_transaction *txn = fdb_database_create_transaction(db);
    apply_state_changes(txn, commands);
    fdb_transaction_commit(txn);

    // Phase 4: run planner if state diverged
    if (replan_triggered) {
        taskweft_plan(domain, state);
    }

    // Phase 5: emit narrative output to clients
    for (each connected client) {
        send_world_output(client, output);
    }

    // Phase 6: sleep remainder of 15.625ms budget
    sleep_until_next_tick();
}
```

### FoundationDB schema

World state layout:

| Key prefix | Value | Description |
|------------|-------|-------------|
| `room/{id}` | JSON | Room description, exits, items |
| `npc/{id}` | JSON | NPC state, trust level, knowledge |
| `player/{id}` | JSON | Player inventory, location, flags |
| `obj/{id}` | JSON | Object properties, location |
| `tick/{id}` | int | Global tick counter per session |

FDB transactions guarantee atomic reads and writes across all keys
for a given tick. No migration overhead — schema is application-enforced.
The C API (`fdb_c.h`) is linked directly.

### WebSocket + slash command handler

- Client connects via WebSocket upgrade through libh2o.
- Server sends `>> ` prompt on each tick (RFD 5).
- Client sends `/command [args]` (RFD 7).
- Server validates, dispatches to Taskweft planner via C FFI, applies
  state changes, returns narrative text.

### Protocol neutrality

The game protocol is text-over-WebSocket at the application layer,
bit-crushed binary on the wire (RFD 7, RFD 14). The server does not
distinguish between human and automated players — see RFD 2.

## See also

- **STACK_DECISION.md**: Full StarVote scoring (C + libh2o won 28/35)
- **RFD 3**: Transport — libh2o (direct C API)
- **RFD 5**: Simulation model — 64 Hz ticked
- **RFD 7**: Slash command protocol
- **RFD 11**: State layer — FoundationDB
- **RFD 12**: Planning — Taskweft (C FFI)
- **RFD 14**: Wire format — bit-crushed binary frames

## Implementation status

- [ ] libh2o WebSocket handler with C event loop
- [ ] FDB C client linked and connected
- [ ] Command parser and dispatcher in C
- [ ] Session manager (connection pool)
- [ ] Tick loop integration with libh2o event loop
