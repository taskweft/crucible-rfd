---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: planning,taskweft
stage: mvp
---

# RFD 12: Planning — Taskweft (C FFI)

## Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Planning | Taskweft | HTN planner with temporal durations. NPC behavior expressed as goal-task networks replanned on each tick. Called via C FFI from the server main loop. |

## Decisions

- **Taskweft over LLM-driven NPCs** — NPC behavior must be deterministic
  and auditable. LLM-driven NPCs would introduce non-determinism and
  conflate the evaluator with the environment. Taskweft HTN plans are
  deterministic, parametrizable, and reproduce identically from the same
  state. LLMs are never required for NPC logic (see RFD 2).
- **C FFI integration** — the server calls the Taskweft planner through
  its C NIF shared object (`taskweft_nif.so`). No Elixir runtime. The
  planner's JSON-LD domain string is passed via `dlopen`/`dlsym` to the
  NIF's entry point. See `STACK_DECISION.md` for the full reasoning.
- **Ticked planning** — the planner runs on each simulation tick (RFD 5),
  replanning only when state diverges from the current plan.
- **Domain per scenario** — each scenario provides its own domain file
  (NPC actions, methods, state variables). The server loads the domain
  at session start.

## Integration

The planner is called via C FFI from the tick loop. Each tick:

1. Serialize current world state as JSON-LD working memory.
2. Call Taskweft's plan entry point via `dlsym`:
   ```c
   typedef int (*plan_fn)(const char *domain_json,
                          const char *state_json,
                          char **plan_out);
   plan_fn plan = (plan_fn)dlsym(taskweft_handle, "taskweft_plan");
   plan(domain_json, state_json, &plan_json);
   ```
3. Parse the plan JSON and execute the highest-priority action.
4. If state diverges from plan, trigger a replan.

The C server links against `taskweft_nif.so` at startup. The shared
object exposes a C ABI that any C program can call — no Elixir,
no NIF VM, no BEAM dependency. The `taskweft_nif` project already
builds as a standalone shared library; this RFD documents how the
Crucible server consumes it.

## See also

- **RFD 8**: Taskweft NPC domain (full stage — detailed domain design)
- **RFD 5**: Simulation model
- **RFD 2**: Project definition (no LLM required principle)
- **STACK_DECISION.md**: Full StarVote scoring (C + libh2o won 28/35)

## Implementation status

- [x] Stack selected
- [ ] Taskweft NIF compiled as standalone shared library
- [ ] `dlopen`/`dlsym` integration in C server main loop
- [ ] Tick-to-planner integration
- [ ] Replan-on-divergence trigger
