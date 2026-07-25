---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: protocol,output
stage: mvp
---

# RFD 16: World output format

## Problem

The server emits narrative text on every tick — room descriptions, NPC
dialogue, system messages. The client needs to distinguish message
source and type to render them correctly (tags, colors, layout).

## Format

Each world output is a structured message with source and body:

| Field | Type | Description |
|-------|------|-------------|
| source | enum | `world`, `npc`, `system`, `error` |
| source_name | string | NPC name or empty (for world/system) |
| body | string | Narrative text |
| tags | [string] | Optional: `room_desc`, `dialogue`, `combat`, `event` |

### Examples

```json
{"source": "world", "body": "You are in the Town Square. A fountain burbles.",
 "tags": ["room_desc"]}

{"source": "npc", "source_name": "Watch Captain",
 "body": "\"Keep your eyes open, stranger.\"",
 "tags": ["dialogue"]}

{"source": "system", "body": "You pick up the rusty key.",
 "tags": ["event"]}

{"source": "error", "body": "Unknown command. Try /help."}
```

## Wire encoding

Over the bit-crushed WebSocket (RFD 14), each output frame:

```
uint8   source          — 0=world, 1=npc, 2=system, 3=error
uint8   name_len        — length of source_name (0 for world/system/error)
uint8[name_len] name    — UTF-8 NPC name
uint16  body_len        — length of body
uint8[body_len] body    — UTF-8 narrative text
uint8   tag_count       — number of tags
for each tag:
  uint8 tag_len
  uint8[tag_len] tag    — UTF-8 tag string
```

## Client rendering

The web client (RFD 15) renders each output as a message block:

- **world** — white text, no tag, full width
- **npc** — colored name tag, italic body for dialogue
- **system** — dim/grey text, prefixed with •
- **error** — red text

## Open questions

- Should output include rich text (bold, italic) or stay plain?
- Multi-line body — newlines preserved or wrapped?

## Implementation status

- [ ] Output format defined
- [ ] Server-side output serializer
- [ ] Client-side decoder and renderer

## See also

- **RFD 7**: Slash command protocol
- **RFD 14**: Wire format — bit-crushed binary frames
- **RFD 15**: Web client
