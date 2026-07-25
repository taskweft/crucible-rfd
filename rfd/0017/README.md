---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: devops,ci
stage: mvp
---

# RFD 0017: CI/CD pipeline

## Build

GitHub Actions workflow at `.github/workflows/ci.yml`.

| Step | What | Why |
|------|------|-----|
| Checkout | `actions/checkout@v5` | Node 24 compat |
| Build h2o | cmake from `vendor/h2o/` | Vendored via git subtree, no network dep |
| FDB client | `foundationdb-clients_7.3.79` deb | C headers + lib for compilation |
| Build demo | cmake from `h2o-bench-tpcc/` | SPSC ring + worker pool + main |

## Runner

`ubuntu-24.04` (2 cores). The build must complete within the 6-hour
runner limit. Current build time: ~5-8 minutes.

## Status checks

The `build` job is a required status check on `main`. PRs cannot merge
through the merge queue unless it passes. Configured via GitHub
repository ruleset `require-pr-and-merge-queue-on-main` and branch
protection requiring `build` context.

## Merge queue

| Setting | Value |
|---------|-------|
| Merge method | MERGE (squash on branch) |
| Grouping | ALLGREEN |
| Min to merge | 1 |
| Check timeout | 60 min |

## See also

- **RFD 0019**: Deployment (Docker + Fly.io)
- **RFD 0003**: Transport — libh2o (build dependency)
- `.github/workflows/ci.yml`
