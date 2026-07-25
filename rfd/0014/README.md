---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: protocol,wire-format
stage: full
---

# RFD 14: Wire format — bit-crushed binary frames

## Motivation

Every byte on the wire carries data, not framing overhead. The WebSocket
transport sends binary frames (opcode 2) with compact field-level encoding.
Variable-length integers, bit-packed enums, and fixed-size fields where
possible — no JSON, no text markup, no redundant keys.

## Frame types

### Command frame (client → server)

```
uint8   command_id    — enum: 0=look, 1=move, 2=talk, 3=say,
                       4=take, 5=use, 6=inventory, 7=help
uint8   arg_count
for each arg:
  uint8   arg_len
  uint8[] arg         — UTF-8 argument bytes
```

The client sends one command frame per turn. The server responds with
one or more output frames.

### Output frame (server → client)

```
uint8   source        — 0=world, 1=npc, 2=system, 3=error
uint8   name_len      — 0 for world/system/error
uint8[] name          — NPC UTF-8 name (only when source=npc)
uint16  body_len
uint8[] body          — UTF-8 narrative text
uint8   tag_count
for each tag:
  uint8 tag_len
  uint8[] tag         — UTF-8 tag (room_desc, dialogue, event, etc.)
```

### Tick frame (server → client, every tick)

```
uint32  tick          — monotonic tick counter
uint16  output_count
repeated OutputFrame
```

## Encoding rationale

- **uint8 command_id** — 7 commands fits in 3 bits, but a full byte avoids
  bit-shifting overhead in JS and aligns every frame to byte boundaries.
- **uint8/uint16 length prefixes** — bounded by max command length (255
  chars per arg) and max narrative text (65 KB per output).
- **No framing delimiter** — WebSocket is message-oriented; each frame is
  one WebSocket message. No start/end markers needed.

## See also

- **RFD 7**: Slash command protocol (user-facing commands)
- **RFD 16**: World output format (structure behind OutputFrame)
- **RFD 3**: Transport — libh2o (WebSocket carrier)
- **RFD 15**: Web client (consumer of this wire format)
