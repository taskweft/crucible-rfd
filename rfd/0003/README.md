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
| Transport | libh2o (dual-stack HTTP/1.1 + HTTP/3) | Single library for HTTP/1.1, HTTP/3, and WebSocket. Proven in high-throughput production. |

libh2o provides the WebSocket transport. All application frames over
WebSocket are bit-crushed binary — the protocol does not use text/JSON
on the wire (see **RFD 14** for the binary frame format).

## Decisions

- **Dual-stack HTTP/1.1 + HTTP/3** — WebSocket runs over HTTP/1.1 for
  broad compatibility; HTTP/3 available for low-latency clients.
- **Bit-crushed binary frames** — every byte on the wire carries data,
  not framing overhead. See RFD 14 for the frame layout.

## See also

- **RFD 11**: State layer — FoundationDB
- **RFD 12**: Planning — Taskweft
- **RFD 7**: Slash command protocol (user-facing, not wire format)
- **RFD 14**: Wire format — bit-crushed binary frames

## Implementation status

- [x] Stack selected
- [ ] libh2o compiled with WebSocket support
- [ ] Binary frame handler
