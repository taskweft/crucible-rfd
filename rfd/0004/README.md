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
- **Objective**: a goal expression the player must satisfy (e.g., gain
  the Watch captain's trust, identify the marked contact).
- **NPC plans**: per-NPC task networks that advance the scenario in
  response to player actions.
- **Win condition**: state predicate that ends the run.

## Canonical scenario: Middleham

Imported from `CrucibleBench/CrucibleBench_Phase1` (Zenodo DOI
10.5281/zenodo.21386663). The Python reference implementation lives
at `rfd/0004/reference/` for traceability.

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
12 rooms. Three key items (guard_token, tariff_letter, old_map) are
shuffled at seed time for replay variability.

### Objectives

1. **gain_watch_trust**: reach trust ≥ 75 with the captain, talk to
   them ≥ 2 times, and request a recommendation ≥ 1 time.
2. **identify_marked_contact**: determine which NPC (keeper, merchant,
   or peasant) is secretly aligned with the Marked, using clues from
   dialogue and items.

## Taskweft domain

The scenario is defined as a Taskweft DSL domain. Key state variables:

```elixir
@variables %{
  # Player
  player_room: %{type: :ref, init: %{current: "city_gate"}},
  inventory: %{type: :ref, init: %{}},

  # Room graph
  room_exits: %{type: :ref, init: %{
    city_gate: %{north: "main_square"},
    main_square: %{south: "city_gate", north: "guard_barracks",
                   east: "market_street", west: "tavern"},
    guard_barracks: %{south: "main_square", east: "residential_street",
                      north: "guild_court"},
    guild_court: %{south: "guard_barracks", east: "outskirt_road"},
    market_street: %{west: "main_square", north: "merchant_hall",
                     east: "temple_entry"},
    merchant_hall: %{south: "market_street", east: "outskirt_road"},
    tavern: %{east: "main_square", north: "temple_entry"},
    temple_entry: %{west: "market_street", south: "tavern",
                    north: "temple_inner"},
    temple_inner: %{south: "temple_entry"},
    residential_street: %{west: "guard_barracks", north: "temple_entry",
                          east: "outskirt_road"},
    outskirt_road: %{west: "residential_street", south: "forest_rim",
                     north: "merchant_hall", east: "forest_rim"},
    forest_rim: %{north: "outskirt_road", south: "city_gate"}
  }},

  # NPC state
  npc_trust: %{type: :ref, init: %{captain: 58, keeper: 50,
                                    merchant: 52, peasant: 46}},
  npc_suspicion: %{type: :ref, init: %{captain: 22, keeper: 30,
                                        merchant: 28, peasant: 34}},
  npc_talks: %{type: :ref, init: %{captain: 0, keeper: 0,
                                    merchant: 0, peasant: 0}},
  npc_marked: %{type: :ref, init: %{captain: false, keeper: false,
                                     merchant: false, peasant: false}},

  # Objective state
  watch_recommendation_requests: %{type: :int, init: 0},
  clue_count: %{type: :int, init: 0},
  suspect_scores: %{type: :ref, init: %{captain: 0, keeper: 0,
                                         merchant: 0, peasant: 0}},
  objective_complete: %{type: :bool, init: false}
}
```

Player actions as Taskweft actions:

```elixir
@actions %{
  # Move requires an exit from current room in the given direction.
  # Precondition checked by method alternative guard.
  move: %{
    params: [:direction],
    body: [%{pointer_set: "/player_room/current",
             value: %{eval: %{type: "pointer/get",
                             pointer: "/room_exits/{player_room}/{direction}"}}}]
  },

  # Talk increments the NPC's talk counter and applies trust/suspicion
  # deltas based on dialogue content (classified by the DialogueClassifier
  # at runtime; the planner uses the known effect structure).
  talk: %{
    params: [:npc, :intent],
    body: [
      %{pointer_set: "/npc_talks/{npc}",
        value: %{eval: %{type: "math/add",
                        a: %{pointer_get: "/npc_talks/{npc}"}, b: 1}}}
    ]
  },

  take: %{
    params: [:item],
    body: [%{pointer_set: "/inventory/{item}", value: "held"}]
  }
}
```

Goal decomposition for the two objectives:

```elixir
@methods %{
  gain_watch_trust: %{
    params: [],
    alternatives: [
      %{
        name: :build_trust,
        subtasks: [
          [:move_to, "guard_barracks"],
          [:talk, "captain", "polite"],
          [:talk, "captain", "offer_help"],
          [:request_recommendation]
        ]
      }
    ]
  },

  identify_marked: %{
    params: [],
    alternatives: [
      %{
        name: :gather_intel,
        subtasks: [
          [:move_to, "tavern"],
          [:talk, "keeper", "ask_rumors"],
          [:move_to, "market_street"],
          [:talk, "merchant", "ask_trade"],
          [:move_to, "temple_inner"],
          [:talk, "peasant", "ask_witness"],
          [:cross_reference_clues]
        ]
      }
    ]
  }
}
```

## Open questions

- Room count target for a satisfying-but-finite scenario? (Middleham has
  12 — sufficient for PoC, expandable for full release.)
- Items: static props or interactive objects with state? (Middleham uses
  take/give with key-item shuffling at seed time.)
- How is scenario configuration provided to the server? (RFD 8's domain
  encoding — each scenario is a `priv/scenarios/{name}.ex` file compiled
  by the Taskweft DSL loader.)

## Implementation status

- [x] Room graph data model (12 rooms, proven in CrucibleBench Phase 1)
- [x] NPC state (trust/suspicion, 4 NPCs, proven in Phase 1)
- [x] Items (14 items with shuffling, proven in Phase 1)
- [x] Two objectives (gain_watch_trust, identify_marked_contact)
- [ ] Taskweft DSL domain for Middleham scenario
- [ ] Scenario file (`priv/scenarios/middleham.ex`)
- [ ] Server loads scenario domain at session start

## Reference

The Python state-machine implementation from CrucibleBench Phase 1
is archived at `rfd/0004/reference/`. The original repository:
`https://github.com/CrucibleBench/CrucibleBench_Phase1`

Scoring dimensions (also migrated):
- **Goal pursuit**: objective completion weighted by trust/progress
- **Social adaptation**: NPC diversity, repetition penalty, feedback
  response, intent variety
- **World grounding**: valid actions, error penalty, action diversity,
  location diversity, item engagement
- **Strategic sophistication**: exploration breadth, efficiency, NPC
  engagement diversity, objective-relevant progress
