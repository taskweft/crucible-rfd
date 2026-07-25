---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: published
discussion: https://github.com/taskweft/crucible-rfd/pull/1
labels: process
stage: mvp
---

# RFD 1: Requests for Discussion

## Problem

Design decisions for the Crucible project need a structured capture
mechanism. Ad-hoc decisions made in issues, chat, or commits are lost
to future contributors.

## Proposal

Adopt the Oxide RFD process. Each RFD is a Markdown document under
`rfd/XXXX/README.md` with YAML front-matter: authors, state, discussion
link, labels.

States: prediscussion (drafting), ideation (topic only), discussion (PR
open), published (merged consensus), committed (implemented), abandoned.

Lifecycle: reserve number → create from prototype → draft in branch →
open PR with state=discussion → merge with state=published → update to
committed when implementation lands.

## Implementation status

- [x] Process defined (this RFD)
