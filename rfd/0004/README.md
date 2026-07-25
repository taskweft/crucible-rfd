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
player actions, NPC state, and Taskweft scenario plans.

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
- **Objective**: a goal expression the player must satisfy (e.g., reveal
  the antagonist, deliver the package).
- **NPC plans**: per-NPC task networks that advance the scenario in
  response to player actions.
- **Win condition**: state predicate that ends the run (e.g., trust
  level threshold reached, item delivered to target).

### Example: Medieval town

```
Town Square ──n── Market Street ──n── Castle Gate
     │                    │
     s                    s
     │                    │
  Old Well           Blacksmith
```

- NPCs: Watch Captain (Town Square), Merchant (Market Street),
  Blacksmith (Blacksmith), Guard (Castle Gate)
- Objective: identify the smuggler by building trust with NPCs
- Mechanic: each NPC has hidden knowledge revealed through dialogue;
  trust unlocks progressively more sensitive information

## Open questions

- Room count target for a satisfying-but-finite scenario?
- Items: static props or interactive objects with state?
- How is scenario configuration provided to the server?

## Implementation status

- [ ] Room graph data model
- [ ] Scenario format (goals, NPC placements)
- [ ] Example scenario: Medieval town
