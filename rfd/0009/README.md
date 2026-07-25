---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: evaluation
---

# RFD 9: LLM evaluation scoring protocol

## Problem

CrucibleBench's central finding is that model rankings are highly
sensitive to the LLM judge's classifier design. Aggregate reliability
metrics (κ) don't expose which models are affected. A reproducible
scoring protocol must separate algorithmic metrics from classifier
judgment.

## Scoring dimensions

| Dimension | Source | Classifier-dependent |
|-----------|--------|---------------------|
| Goal completion | Binary flag in scenario | No |
| Turn efficiency | Steps-to-complete | No |
| Exploration coverage | Rooms visited / items found | No |
| Dialogue concentration | Turns per NPC | No |
| Trust trajectory | Trust level over time | No |
| Strategic sophistication | Classifier probe | Yes |

Following CrucibleBench, results are reported under two configurations:
a full composite (all six dimensions) and a classifier-minimized subtotal
(first five). Divergence between the two is a diagnostic signal.

## Protocol

- Fixed 50-turn budget per run (per CrucibleBench Phase 1).
- 5+ runs per model for confidence intervals.
- Judge model is independent of the evaluated model (no same-family
  judge).
- Per-model agreement audit reported alongside aggregate scores.

## Implementation status

- [ ] Log format for tick-by-tick state snapshots
- [ ] Score calculator (first five dimensions)
- [ ] Classifier prompt and integration
- [ ] Reporting: leaderboard + per-model agreement table
