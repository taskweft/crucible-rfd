---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: evaluation
stage: someday
---

# RFD 9: Evaluation scoring

## Problem

When running automated clients (scripted agents, LLMs, or any replay)
through the MUD, you need to answer: did it complete the scenario, and
how well? CrucibleBench's central finding is that composite scoring is
highly sensitive to the judge's classifier design. A reproducible
scoring protocol must separate algorithmic metrics from classifier
judgment.

Evaluation is an opt-in mode. The MUD server does not require a scorer
to run; scoring is a post-hoc analysis tool applied to logged sessions.

## Scoring dimensions

| Dimension | Source | Classifier-dependent |
|-----------|--------|---------------------|
| Goal completion | Binary flag in scenario | No |
| Turn efficiency | Steps-to-complete | No |
| Exploration coverage | Rooms visited / items found | No |
| Dialogue concentration | Turns per NPC | No |
| Trust trajectory | Trust level over time | No |
| Strategic sophistication | Classifier probe | Yes |

Results can be reported under two configurations: a full composite
(all six dimensions) and a classifier-minimized subtotal (first five).
Divergence between the two is a diagnostic signal.

## Protocol

- Fixed 50-turn budget per run.
- 5+ runs per subject for confidence intervals.
- Judge (if used) is independent of the subject — no same-family judge.
- Per-subject agreement audit reported alongside aggregate scores.

## Implementation status

- [ ] Log format for tick-by-tick state snapshots
- [ ] Score calculator (first five dimensions)
- [ ] Classifier prompt and integration (optional)
- [ ] Reporting: leaderboard + per-subject agreement table
