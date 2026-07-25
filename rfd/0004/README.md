---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: world-design
stage: mvp
---

# RFD 4: World design and narrative

## Design

A MUD world is a directed graph of rooms. Each room has a description,
exits, items, and NPCs. Narrative is driven by the interaction between
player actions, NPC state, and the Taskweft scenario plan.

### Room structure

```
Room {
  id: string
  description: string          — what the player sees on /look
  exits: {direction => roomId} — n, s, e, w, up, down
  items: [Item]                — objects present in the room
  npcs: [NpcId]                — NPCs present
  tags: [string]               — thematic tags (town, forest, dungeon)
}
```

### Scenario structure

A scenario is a goal-oriented narrative framed as a Taskweft domain:

- **Setting**: room graph with initial item and NPC placements.
- **Objective**: a goal expression the player must satisfy.
- **NPC plans**: per-NPC task networks that advance the scenario in
  response to player actions.
- **Win condition**: state predicate that ends the run.

## Canonical scenario: Middleham

Independent Taskweft DSL reproduction of the scenario design described
in CrucibleBench (Zenodo DOI 10.5281/zenodo.21386663). The canonical
implementation lives at `priv/scenarios/middleham.ex`.

### Room graph

```
                     Guild Court
                         |
                   Guard Barracks
                    /          \
        Residential St     Main Square — Merchant Hall
               |           /    |    \          |
          Temple Entry — Market St  Tavern   Outskirt Road
               |                               /         \
          Temple Inner                    Forest Rim   City Gate
```

12 rooms. The watch captain, the merchant, and the tavern keeper each
know a piece of the smuggler's identity. The peasant freedman watched
the handoff from the forest rim.

### NPCs (4)

| Key | Name | Role | Room | Base trust | Base suspicion |
|-----|------|------|------|-----------|---------------|
| captain | Ser Alarik | Watch Officer | guard_barracks | 58 | 22 |
| keeper | Hale | Tavern Keeper | tavern | 50 | 30 |
| merchant | Bran | Road Merchant | market_street | 52 | 28 |
| peasant | Yelena | Freedman | temple_inner | 46 | 34 |

### Items (14)

Guard token, old map, street crystal, signet ring, guild coin, tariff
letter, sealed letter, rumor scroll, prayer beads, temple pass, altar
chalk, cloth scarf, rusted blade, charcoal stone — scatter across the
12 rooms as defined in `room_items` in the domain file.

### Objectives

1. **gain_watch_trust**: reach trust ≥ 75 with the captain, talk to
   them ≥ 2 times, and request a recommendation ≥ 1 time.
2. **identify_marked_contact**: determine which NPC (keeper, merchant,
   or peasant) is secretly aligned with the Marked, using clues from
   dialogue and items.

## Independent reproduction

The scenario is implemented as a clean-room Taskweft DSL domain at
`priv/scenarios/middleham.ex`. This is the canonical definition —
not a copy of any other codebase. The Zenodo dataset records the
experimental results from the original Python state-machine; the
Taskweft DSL is an independent, functionally equivalent reproduction
of the same scenario design.

### State variables

The domain declares world state as Taskweft variables:

| Variable | Type | Purpose |
|----------|------|---------|
| `player_room` | ref | Current room id |
| `inventory` | ref | Items the player carries |
| `exits` | ref | Room graph: 12 rooms × up to 4 directions |
| `room_items` | ref | Items present in each room |
| `item_desc` | ref | Descriptions for the examine action |
| `npc_rooms` | ref | Which room each NPC occupies |
| `npc_names` | ref | Display names |
| `npc_trust` | ref | Trust level per NPC (0–100) |
| `npc_suspicion` | ref | Suspicion level per NPC (0–100) |
| `npc_talk_count` | ref | Conversation count per NPC |
| `npc_marked` | ref | Whether each NPC is the marked contact |
| `turn_count` | int | Tick counter |

### Player actions

Six actions defined in the domain:

| Action | Precondition | Effect |
|--------|-------------|--------|
| `move(direction)` | Exit exists from current room | Updates `player_room` |
| `look` | — | No state change (triggers narrative) |
| `examine(item)` | Item in room or inventory | No state change (triggers narrative) |
| `take(item)` | Item in room | Adds to `inventory` |
| `give(item, npc)` | Item in inventory, NPC in room | Transfers item (narrative) |
| `talk(npc, intent)` | NPC in room | Increments talk count, triggers trust deltas |

### Objective decomposition

The domain defines two top-level methods, each with multiple
alternative decomposition paths:

- **gain_watch_trust**: approach the captain directly, or build trust
  through actions (reporting suspicious activity, volunteering patrol)
- **identify_marked_contact**: gather intel from all NPCs, or
  investigate items first then cross-reference

Each method alternative triggers different NPC interactions, producing
different trust/suspicion trajectories — the same behavioural
differentiation the original CrucibleBench study measured.

## Open questions

- Room count target for a satisfying-but-finite scenario? (Middleham has
  12 — sufficient for PoC, expandable for full release.)
- How is scenario configuration provided to the server? (RFD 8's domain
  encoding — each scenario is a `priv/scenarios/{name}.ex` file compiled
  by the Taskweft DSL loader.)

## Implementation status

- [x] Room graph data model (12 rooms)
- [x] NPC state (trust/suspicion, 4 NPCs)
- [x] Items (14 with descriptions)
- [x] Two objectives (gain_watch_trust, identify_marked_contact)
- [x] Taskweft DSL domain (`priv/scenarios/middleham.ex`)
- [ ] Server loads scenario domain at session start
- [ ] NPC trust/suspicion delta logic integrated with dialogue classifier
