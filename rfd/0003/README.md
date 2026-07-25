---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: committed
discussion: https://github.com/taskweft/crucible-rfd/pull/3
labels: tech-stack,transport
stage: mvp
---

# RFD 3: Transport — libh2o

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Transport | libh2o (dual-stack HTTP/1.1 + HTTP/3) | Single library for HTTP/1.1, HTTP/3, and WebSocket. Proven in high-throughput production. Direct C API — no NIF bridge. |

libh2o provides the WebSocket transport. The server calls libh2o's C API
directly — no Elixir NIF, no intermediate runtime. The event loop is
libh2o's own `h2o_evloop_t` integrated with the 64Hz tick loop
(RFD 5). All application frames over WebSocket are bit-crushed binary
(RFD 14).

## Decisions

- **Dual-stack HTTP/1.1 + HTTP/3** — WebSocket runs over HTTP/1.1 for
  broad compatibility; HTTP/3 available for low-latency clients.
- **Direct C API** — the server links against libh2o as a C library,
  not via Elixir NIF bindings. Eliminates an FFI boundary on the
  transport hot path.
- **Bit-crushed binary frames** — every byte on the wire carries data,
  not framing overhead. See RFD 14 for the frame layout.

## See also

- **RFD 11**: State layer — FoundationDB
- **RFD 12**: Planning — Taskweft
- **RFD 7**: Slash command protocol (user-facing, not wire format)
- **RFD 14**: Wire format — bit-crushed binary frames
- **STACK_DECISION.md**: Full StarVote scoring (C + libh2o won 28/35)

## Implementation status

- [x] Stack selected
- [ ] libh2o compiled with WebSocket support
- [ ] Binary frame handler
