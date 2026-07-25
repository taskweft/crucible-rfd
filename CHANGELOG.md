# Changelog

## 2026-07-24

- **RFD 4**: Independent Taskweft DSL reproduction of the Middleham
  scenario at `priv/scenarios/middleham.ex`. Clean-room implementation
  (not a copy of the CrucibleBench Python codebase). References
  Zenodo DOI 10.5281/zenodo.21386663 as external citation only.
- **PERT plan**: Restructured around Gall's Law with libh2o linear
  scaling as the starting point. Taskweft planner-validated serial
  schedule (131H total). Parallel critical path 53H across 5 phases.
- **RFD 16**: World output format defined as Elixir DSL
  (`Crucible.WorldOutput` struct + `Encoder` protocol).
- **CITATION.cff**: Backfilled with all dependency references
  (FDB, Taskweft, libh2o, lean-* repos, CrucibleBench dataset).
- **FDB features**: Reframed around linear scaling property.
- **PERT calibration**: All estimates calibrated against taskweft
  git log velocity (Jul 14–24, 2026).

## 2026-07-23

- RFDs 10–16 added (bandwidth allocation, state layer, planning,
  FDB schema, wire format, web client, world output).
- RFD 10: muscle system + Quintic Hermite interpolation.
- RFD 13/14: FDB schema design, bit-crushed binary wire format.

## 2026-07-22

- Initial RFDs 1–9: process, project definition, tech stack (libh2o),
  world design, ticked simulation, architecture, slash protocol,
  NPC domain (Taskweft), evaluation scoring.
- PERT plan with initial critical path.
- CITATION.cff with CrucibleBench dataset reference.
