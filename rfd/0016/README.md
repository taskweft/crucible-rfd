---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: protocol,output
stage: mvp
---

# RFD 16: World output (C structs)

## Problem

The server emits narrative text on every tick — room descriptions, NPC
dialogue, system messages. The client needs to distinguish message
source and type to render them correctly (tags, colors, layout).

## Format — C struct

Each world output is a C struct. A serializer function table handles
encoding to bit-crushed binary (RFD 14) and JSON (for the web client,
RFD 15). The server language is C (see `STACK_DECISION.md`), so the
output format is a plain struct — no Elixir, no protocol overhead.

```c
typedef enum {
    WORLD_OUTPUT_SOURCE_WORLD = 0,
    WORLD_OUTPUT_SOURCE_NPC   = 1,
    WORLD_OUTPUT_SOURCE_SYSTEM = 2,
    WORLD_OUTPUT_SOURCE_ERROR = 3,
} world_output_source_t;

typedef enum {
    WORLD_OUTPUT_TAG_ROOM_DESC   = 0,
    WORLD_OUTPUT_TAG_DIALOGUE    = 1,
    WORLD_OUTPUT_TAG_COMBAT      = 2,
    WORLD_OUTPUT_TAG_EVENT       = 3,
    WORLD_OUTPUT_TAG_SYSTEM      = 4,
} world_output_tag_t;

typedef struct {
    world_output_source_t source;
    const char *source_name;       // NPC name (NULL for world/system/error)
    const char *body;              // Narrative text (UTF-8)
    const world_output_tag_t *tags; // Tag array
    size_t tag_count;
} world_output_t;
```

### Serializer function table

A `world_output_serializer` struct decouples the output struct from its
wire representation — the same pattern as the original Elixir
`Encoder` protocol, but implemented as a C function pointer table:

```c
typedef struct {
    bool (*encode_binary)(const world_output_t *output, uint8_t **buf, size_t *len);
    char *(*encode_json)(const world_output_t *output);
    void (*free)(world_output_t *output);
} world_output_serializer_t;

// Singleton — one serializer table per server instance.
extern const world_output_serializer_t world_output_serializer;
```

### Constructor helper

```c
world_output_t world_output_make(world_output_source_t source,
                                  const char *source_name,
                                  const char *body,
                                  const world_output_tag_t *tags,
                                  size_t tag_count);
```

### Examples

```c
// Room description
world_output_t out = world_output_make(
    WORLD_OUTPUT_SOURCE_WORLD, NULL,
    "You are in the Town Square. A fountain burbles.",
    (world_output_tag_t[]){WORLD_OUTPUT_TAG_ROOM_DESC}, 1);

// NPC dialogue
out = world_output_make(
    WORLD_OUTPUT_SOURCE_NPC, "Watch Captain",
    "\"Keep your eyes open, stranger.\"",
    (world_output_tag_t[]){WORLD_OUTPUT_TAG_DIALOGUE}, 1);

// System event
out = world_output_make(
    WORLD_OUTPUT_SOURCE_SYSTEM, NULL,
    "You pick up the rusty key.",
    (world_output_tag_t[]){WORLD_OUTPUT_TAG_EVENT}, 1);
```

## Wire encoding

The `encode_binary` serializer produces bit-crushed frames (RFD 14):

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

The `encode_json` serializer produces:

```json
{"source": "world", "body": "You are in the Town Square.", "tags": ["room_desc"]}
{"source": "npc", "source_name": "Watch Captain", "body": "\"Keep your eyes open.\"", "tags": ["dialogue"]}
```

## Client rendering

The web client (RFD 15) renders each output as a message block:

- **world** — white text, no tag, full width
- **npc** — colored name tag, italic body for dialogue
- **system** — dim/grey text, prefixed with •
- **error** — red text

## See also

- **RFD 7**: Slash command protocol
- **RFD 14**: Wire format — bit-crushed binary frames
- **RFD 15**: Web client
- **STACK_DECISION.md**: Full StarVote scoring (C + libh2o won 28/35)

## Implementation status

- [ ] C `world_output_t` struct defined
- [ ] `world_output_serializer` function table with binary encoder
- [ ] JSON encoder implementation (web client)
- [ ] Tick loop emits `world_output_t` on each tick
