---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: published
discussion:
labels: process
---

# RFD 1: Requests for Discussion

## Problem Statement

Design decisions for the crucible MUD project need a structured capture
mechanism. Ad-hoc decisions made in issues, chat, or commits are lost to
future contributors.

## Proposal

Adopt the Oxide Computer Company RFD (Request for Discussion) process.

Each RFD is a Markdown document under `rfd/XXXX/README.md` with YAML
front-matter:

```yaml
---
authors: Name <email>
state: prediscussion | ideation | discussion | published | committed | abandoned
discussion:
labels:
---
```

**States:** prediscussion (drafting), ideation (topic only), discussion (PR
open), published (merged consensus), committed (implemented), abandoned.

**Lifecycle:** reserve number → create `rfd/XXXX/README.md` from prototype →
draft in `rfd-XXXX` branch → open PR with state=discussion → merge with
state=published → update to committed when implementation lands.

## Implementation status

- [x] Process defined (this RFD)
- [ ] CI to render RFD index
- [ ] README with link to RFD listing
