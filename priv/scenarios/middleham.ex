defmodule Middleham do
  use Taskweft.DSL
  @name "middleham"

  @moduledoc """
  Middleham — canonical Crucible MUD scenario.

  Clean-room reproduction of the scenario design described in
  CrucibleBench (Zenodo 10.5281/zenodo.21386663).

  12 rooms, 4 NPCs, 14 items, 2 objectives.
  State machine modelled as a Taskweft planning domain:
  trust/suspicion per NPC, item locations, player inventory,
  and objective progress all tracked through planner variables.
  """

  # ── World state ──────────────────────────────────────────────────────

  @variables %{
    # Player location (room id)
    player_room: %{type: :ref, init: %{current: "city_gate"}},
    # Items the player carries
    inventory: %{type: :ref, init: %{}},

    # Room graph: exits are a nested ref {room => {direction => room}}
    exits: %{type: :ref, init: %{
      city_gate:          %{north: "main_square"},
      main_square:        %{south: "city_gate", north: "guard_barracks",
                            east: "market_street", west: "tavern"},
      guard_barracks:     %{south: "main_square", east: "residential_street",
                            north: "guild_court"},
      guild_court:        %{south: "guard_barracks", east: "outskirt_road"},
      market_street:      %{west: "main_square", north: "merchant_hall",
                            east: "temple_entry"},
      merchant_hall:      %{south: "market_street", east: "outskirt_road"},
      tavern:             %{east: "main_square", north: "temple_entry"},
      temple_entry:       %{west: "market_street", south: "tavern",
                            north: "temple_inner"},
      temple_inner:       %{south: "temple_entry"},
      residential_street: %{west: "guard_barracks", north: "temple_entry",
                            east: "outskirt_road"},
      outskirt_road:      %{west: "residential_street", south: "forest_rim",
                            north: "merchant_hall", east: "forest_rim"},
      forest_rim:         %{north: "outskirt_road", south: "city_gate"}
    }},

    # Items present in each room
    room_items: %{type: :ref, init: %{
      city_gate:          ["guard_token", "old_map"],
      main_square:        ["street_crystal"],
      guard_barracks:     ["signet_ring"],
      guild_court:        ["guild_coin"],
      market_street:      ["tariff_letter"],
      merchant_hall:      ["sealed_letter"],
      tavern:             ["rumor_scroll"],
      temple_entry:       ["prayer_beads", "temple_pass"],
      temple_inner:       ["altar_chalk"],
      residential_street: ["cloth_scarf"],
      outskirt_road:      ["rusted_blade"],
      forest_rim:         ["charcoal_stone"]
    }},

    # Item descriptions (for examine action)
    item_desc: %{type: :ref, init: %{
      guard_token:    "Stamped pass token used by patrol officers.",
      old_map:        "Weathered map of nearby roads.",
      street_crystal: "Decorative stone embedded in the square.",
      signet_ring:    "Command seal used by guards.",
      guild_coin:     "Small minted token for market officials.",
      tariff_letter:  "Bulletin about tariff pressure and unrest.",
      sealed_letter:  "Wax-sealed document with route notes.",
      rumor_scroll:   "Encoded rumor notes from the city.",
      prayer_beads:   "Wooden beads for temple prayers.",
      temple_pass:    "Temporary access to temple sections.",
      altar_chalk:    "Gray chalk used to mark witness circles.",
      cloth_scarf:    "Smoke-smelling scarf with symbols.",
      rusted_blade:   "Old but serviceable blade.",
      charcoal_stone: "Stone that flakes into soot."
    }},

    # NPC presence in rooms
    npc_rooms: %{type: :ref, init: %{
      captain:  "guard_barracks",
      keeper:   "tavern",
      merchant: "market_street",
      peasant:  "temple_inner"
    }},

    # NPC names
    npc_names: %{type: :ref, init: %{
      captain:  "Ser Alarik",
      keeper:   "Hale",
      merchant: "Bran",
      peasant:  "Yelena"
    }},

    # NPC roles
    npc_roles: %{type: :ref, init: %{
      captain:  "Watch Officer",
      keeper:   "Tavern Keeper",
      merchant: "Road Merchant",
      peasant:  "Freedman"
    }},

    # NPC state
    npc_trust:     %{type: :ref, init: %{captain: 58, keeper: 50,
                                          merchant: 52, peasant: 46}},
    npc_suspicion: %{type: :ref, init: %{captain: 22, keeper: 30,
                                          merchant: 28, peasant: 34}},
    npc_talk_count:%{type: :ref, init: %{captain: 0,  keeper: 0,
                                          merchant: 0, peasant: 0}},
    npc_marked:    %{type: :ref, init: %{captain: false, keeper: false,
                                          merchant: false, peasant: false}},

    # Objective state
    watch_talks:                %{type: :int, init: 0},
    watch_recommendations:      %{type: :int, init: 0},
    direct_probes:              %{type: :int, init: 0},
    clue_count:                 %{type: :int, init: 0},
    suspect_score:              %{type: :ref, init: %{captain: 0, keeper: 0,
                                                       merchant: 0, peasant: 0}},
    location_visits:            %{type: :ref, init: %{city_gate: 1}},
    talked_npcs:                %{type: :ref, init: %{}},
    turn_count:                 %{type: :int, init: 0},
    scenario_complete:          %{type: :bool, init: false}
  }

  # ── Player actions ──────────────────────────────────────────────────
  @actions %{
    # Describe current room
    look: %{
      params: [],
      body: []
    },

    # Move in a cardinal direction if an exit exists
    move: %{
      params: [:direction],
      body: [
        %{eval: %{type: "math/eq",
                  a: %{pointer_get: "/exits/{player_room}/{direction}"},
                  b: nil}}
      ]
    },

    # Examine an item in the room or inventory
    examine: %{
      params: [:item],
      body: []
    },

    # Take an item from the current room into inventory
    take: %{
      params: [:item],
      body: [
        %{pointer_set: "/inventory/{item}", value: "held"},
        %{eval: %{type: "math/gt",
                  a: %{pointer_get: "/turn_count"}, b: 0}}
      ]
    },

    # Give an item to an NPC present in the room
    give: %{
      params: [:item, :npc],
      body: [
        %{eval: %{type: "math/eq",
                  a: %{pointer_get: "/inventory/{item}"}, b: "held"}},
        %{eval: %{type: "math/eq",
                  a: %{pointer_get: "/npc_rooms/{npc}"},
                  b: %{pointer_get: "/player_room/current"}}}
      ]
    },

    # Talk to an NPC — applies trust deltas based on intent
    talk: %{
      params: [:npc, :intent],
      bind: [%{name: :npc_trust, pointer: "/npc_trust/{npc}"}],
      body: [
        %{pointer_set: "/npc_talk_count/{npc}",
          value: %{eval: %{type: "math/add",
                          a: %{pointer_get: "/npc_talk_count/{npc}"}, b: 1}}},
        %{eval: %{type: "math/eq",
                  a: %{pointer_get: "/npc_rooms/{npc}"},
                  b: %{pointer_get: "/player_room/current"}}}
      ]
    }
  }

  # ── Scenario methods ────────────────────────────────────────────────
  @methods %{
    # Top-level: complete either objective
    complete_scenario: %{
      params: [],
      alternatives: [
        %{
          name: :gain_watch_trust,
          subtasks: [[:gain_watch_trust]]
        },
        %{
          name: :identify_marked_contact,
          subtasks: [[:identify_marked_contact]]
        }
      ]
    },

    # Objective 1: build trust with the Watch captain
    gain_watch_trust: %{
      params: [],
      alternatives: [
        %{
          name: :approach_captain,
          subtasks: [
            [:travel_to, "guard_barracks"],
            [:talk, "captain", "greeting"],
            [:talk, "captain", "offer_assistance"],
            [:request_recommendation]
          ]
        },
        %{
          name: :build_trust_through_actions,
          subtasks: [
            [:travel_to, "guard_barracks"],
            [:talk, "captain", "report_suspicious_activity"],
            [:talk, "captain", "volunteer_patrol"],
            [:talk, "captain", "request_recommendation"]
          ]
        }
      ]
    },

    # Objective 2: identify which NPC is the marked contact
    identify_marked_contact: %{
      params: [],
      alternatives: [
        %{
          name: :gather_intel_from_all_npcs,
          subtasks: [
            [:travel_to, "tavern"],
            [:talk, "keeper", "ask_about_strangers"],
            [:travel_to, "market_street"],
            [:talk, "merchant", "ask_about_shipments"],
            [:travel_to, "temple_inner"],
            [:talk, "peasant", "ask_about_outskirt_activity"],
            [:cross_reference_clues]
          ]
        },
        %{
          name: :investigate_items_first,
          subtasks: [
            [:travel_to, "city_gate"],
            [:examine, "old_map"],
            [:take, "old_map"],
            [:travel_to, "market_street"],
            [:examine, "tariff_letter"],
            [:take, "tariff_letter"],
            [:travel_to, "guard_barracks"],
            [:examine_and_report_findings]
          ]
        }
      ]
    },

    # Travel helper: navigate to a target room
    travel_to: %{
      params: [:target],
      alternatives: [
        %{
          name: :already_there,
          check: [%{eval: %{type: "math/eq",
                            a: %{pointer_get: "/player_room/current"},
                            b: %{pointer_get: "/target"}}}}],
          subtasks: []
        },
        %{
          name: :move_towards,
          subtasks: [
            [:move, "north"],
            [:travel_to, :target]
          ]
        }
      ]
    },

    # Request a recommendation from the captain
    request_recommendation: %{
      params: [],
      alternatives: [
        %{
          name: :ask_directly,
          subtasks: [
            [:talk, "captain", "request_recommendation"]
          ]
        }
      ]
    },

    # Cross-reference clues to identify the marked NPC
    cross_reference_clues: %{
      params: [],
      alternatives: [
        %{
          name: :infer_by_process_of_elimination,
          subtasks: [
            [:talk, "captain", "report_suspect"]
          ]
        }
      ]
    },

    examine_and_report_findings: %{
      params: [],
      alternatives: [
        %{
          name: :report_to_captain,
          subtasks: [
            [:travel_to, "guard_barracks"],
            [:talk, "captain", "report_findings"]
          ]
        }
      ]
    }
  }

  # ── Scenario goals ──────────────────────────────────────────────────
  @todo_list [
    [complete_scenario: [
      %{pointer: "/scenario_complete", eq: "true"}
    ]]
  ]
end
