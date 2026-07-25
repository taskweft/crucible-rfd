---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: networking,simulation
---

# RFD 10: Bandwidth allocation for 64 Hz positional updates

## Problem

RFD 5 defines a fixed 64 Hz tick for the simulation loop. In crowded
areas — where dozens of players and NPCs occupy the same zone — every
tick must broadcast positional updates for every visible entity. Without
a reserved bandwidth ceiling these updates compete with all other
network traffic (chat, inventory, planner output), causing jitter,
dropped updates, or unbounded latency.

We need to lock in a dedicated bandwidth slice of 1–2 Mbps per client
for 64 Hz positional updates in crowded zones. This guarantees that
the tick budget is never starved by network back-pressure and that
players in dense areas receive smooth, deterministic updates.

## Bandwidth calculus

| Variable | Value | Notes |
|---|---|---|
| Tick rate | 64 Hz | Per RFD 5 |
| Visible entities (crowded) | 50–100 | Players + NPCs in view |
| Per-entity payload | 40 B | Compressed pos (12 B) + rot (8 B quat) + vel (12 B) + id (4 B) + header (4 B) |
| Delta encoding savings | ~30 % | Send full state at 4 Hz, deltas at 60 Hz |
| Per-tick payload | 50 × 40 × 0.7 = 1,400 B (low) to 100 × 40 × 0.7 = 2,800 B (high) | |
| Per-second bandwidth | 1,400 × 64 = 90 KB/s ≈ 0.7 Mbps (low) to 2,800 × 64 = 179 KB/s ≈ 1.4 Mbps (high) | |
| Protocol overhead (UDP + application framing) | ~20 % | |
| **Total reserved** | **~1–2 Mbps** | Per client, downstream |

At 64 Hz with 100 visible entities and delta compression, 1–2 Mbps is
sufficient. Doubling the entity count to 200 still fits in 3 Mbps;
the 1–2 Mbps lock-in covers the common crowded-case target.

## Why lock in a ceiling

- **Determinism**: If the network pipe is saturated by other traffic,
  positional updates buffer and arrive late. A late update on tick N
  causes a state mismatch on tick N+1, breaking the deterministic replay
  guarantee that the ticked model exists to provide.
- **Predictable provisioning**: The game server, FoundationDB throughput,
  and client renderer all need to know the worst-case network load.
  1–2 Mbps per client gives a hard upper bound for capacity planning.
- **QoS classification**: Positional updates get a dedicated traffic
  class with strict priority over chat, asset loading, and narrative
  output. Everything else yields to this 1–2 Mbps reservation.
- **Fairness in crowded zones**: Without a ceiling, a player in a
  100-player cluster competing with asset streaming from the same server
  sees positional stutter. The reservation ensures baseline quality
  regardless of background traffic.

## Protocol sketch

```
Tick N snapshot (compact, UDP):
  uint16  sequence         // 2 B
  uint8   entity_count     // 1 B
  for each entity:
    uint16  entity_id      // 2 B
    int16[3] pos_delta     // 6 B (quantized, relative to last full state)
    uint8   rot_index      // 1 B (smallest-three quaternion compression)
    int16[3] vel_delta     // 6 B (quantized)
                         = 15 B/entity + 3 B overhead ≈ 18 B/entity
```

At 18 B/entity × 100 entities × 64 Hz × 1.2 overhead ≈ 1.1 Mbps.
This is comfortably within the 1–2 Mbps reservation.

## Open questions

- Should the reservation adapt per-zone (empty rooms need less) or be a
  fixed global ceiling?
- What happens when a client's measured bandwidth exceeds the reservation
  on a metered connection?
- Should the server gracefully degrade entity count (LOD culling) before
  exceeding the reservation?

## Implementation status

- [ ] Bandwidth calculus validated against 64 Hz tick model
- [ ] UDP traffic class with strict priority in server network stack
- [ ] Delta encoding (full state @ 4 Hz, deltas @ 60 Hz)
- [ ] QoS policy enforced at both server and client
