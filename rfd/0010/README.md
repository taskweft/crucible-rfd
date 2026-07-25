---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: networking,simulation
stage: someday
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
for 64 Hz muscle-system updates in crowded zones. This guarantees that
the tick budget is never starved by network back-pressure and that
players in dense areas receive smooth, deterministic motion.

## Muscle system

Positional updates are not raw position/rotation/velocity deltas. They
use the **muscle system** formalized in
[`lean-humanoid-rom`](https://github.com/v-sekai-multiplayer-fabric/lean-humanoid-rom):

- Each bone has 3 muscles: **swing1**, **swing2**, **twist** (a scalar in
  [-1000, 1000] mapped to [min_deg, max_deg]).
- A standard humanoid has 15 bones → 45 muscle values per skeleton.
- Muscles are mapped to kusudama joint constraints (swing cones + twist
  range), avoiding gimbal lock and box-shaped limits.
- Between ticks, the receiver reconstructs smooth C³-continuous motion
  via **Quintic Hermite spline** interpolation (integer μm arithmetic,
  proved in `PredictiveBvh/adapters/QuinticHermite.lean`).

## Bandwidth calculus

| Variable | Value | Notes |
|---|---|---|
| Tick rate | 64 Hz | Per RFD 5 |
| Visible entities (crowded) | 50–100 | Players + NPCs in view |
| Muscles per skeleton | 45 | 15 bones × 3 DOF |
| Per-muscle encoding | 11 bits | [-1000, 1000] range → 11 bits |
| Full skeleton state | 45 × 11 + 4 id = ~66 B | Entity ID (4 B) + muscle values |
| Active muscles per tick | ~15 | Typical humanoid motion changes 10–15 muscles/tick |
| Delta per-tick payload | 15 × 11 + 4 = ~25 B | Changed muscles + entity ID |
| Full-state baseline rate | 4 Hz | Every 16 ticks, send full 66 B skeleton |
| Delta rate | 60 Hz | Remaining ticks, send 25 B deltas |
| Effective avg per-tick | (66/16 + 25×15/16) ≈ 27.6 B | Weighted average per entity per tick |
| Per-second bandwidth (50 entities) | 50 × 27.6 × 64 ≈ 88 KB/s ≈ 0.7 Mbps | |
| Per-second bandwidth (100 entities) | 100 × 27.6 × 64 ≈ 177 KB/s ≈ 1.4 Mbps | |
| Protocol overhead (UDP + framing) | ~20 % | |
| **Total reserved** | **~1–2 Mbps** | Per client, downstream |

At 64 Hz with 100 visible humanoids, 1–2 Mbps is sufficient. Pure
position/rotation/velocity deltas would require ~3 × the bandwidth —
the muscle system's compact 11-bit scalar encoding and per-DOF delta
granularity are what make 1–2 Mbps feasible.

## Why lock in a ceiling

- **Determinism**: If the network pipe is saturated by other traffic,
  muscle updates buffer and arrive late. A late update on tick N causes a
  kinematic mismatch on tick N+1, breaking the C³ continuity guarantee
  that the Quintic Hermite interpolation provides.
- **Predictable provisioning**: The game server, FoundationDB throughput,
  and client renderer all need to know the worst-case network load.
  1–2 Mbps per client gives a hard upper bound for capacity planning.
- **QoS classification**: Muscle updates get a dedicated traffic class
  with strict priority over chat, asset loading, and narrative output.
  Everything else yields to this 1–2 Mbps reservation.
- **Fairness in crowded zones**: Without a ceiling, a player in a
  100-player cluster competing with asset streaming from the same server
  sees kinematic stutter. The reservation ensures baseline quality
  regardless of background traffic.

## Protocol sketch

```
Tick N delta snapshot (UDP):
  uint16  sequence              // 2 B  (monotonic, not wall-clock)
  uint8   entity_count          // 1 B
  for each changed entity:
    uint16  entity_id           // 2 B
    uint8   changed_mask        // 1 B  (bitmask of which bones have muscle deltas)
    for each changed bone:
      int16[3] muscle_delta     // 6 B  (swing1, swing2, twist × 11 bits packed)

Tick N full state (every 16th tick, TCP or reliable channel):
  uint16  sequence              // 2 B
  uint8   entity_count          // 1 B
  for each visible entity:
    uint16  entity_id           // 2 B
    uint8[6] muscle_full        // 48 B (45 muscle values × 11 bits packed to 6 bytes × 8)
```

At 25 B/entity × 100 entities × 64 Hz × 1.2 overhead ≈ 1.5 Mbps.
Comfortably within the reservation.

## Receiver reconstruction

The client does not interpolate linearly. It uses the **Quintic Hermite
spline basis** from `lean-spatial-oracle`:

```
Given at tick t₀ (numer=0, denom=δ):
  p₀ = muscle values at t₀
  v₀ = muscle velocity at t₀ (derived from last two deltas)
  a₀ = muscle acceleration at t₀

Given at tick t₁ (numer=δ, denom=δ):
  p₁, v₁, a₁

For any intermediate tick t₀ + k (0 ≤ k < δ):
  t = k / δ                          // normalized
  p(t) = h00·p₀ + h01·p₁ + h10·δ·v₀ + h11·δ·v₁ + h20·δ²·a₀ + h21·δ²·a₁

All in integer μm arithmetic. Basis functions h00–h21 are proved
C³-continuous in Lean.
```

## Open questions

- Should the reservation adapt per-zone (empty rooms need less) or be a
  fixed global ceiling?
- What happens when a client's measured bandwidth exceeds the reservation
  on a metered connection?
- Should the server gracefully degrade entity count (LOD culling) before
  exceeding the reservation?

## Implementation status

- [ ] Bandwidth calculus validated against 64 Hz tick model
- [ ] Muscle delta encoding (per-bone changed-mask + 11-bit muscle deltas)
- [ ] Full-state snapshot sent every 16 ticks (4 Hz)
- [ ] UDP traffic class with strict priority
- [ ] Quintic Hermite receiver reconstruction (C³ continuous)
