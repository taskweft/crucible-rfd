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

## Format — Elixir DSL

Each world output is an Elixir struct. The `WorldOutput` module defines
the typed fields; a `WorldOutput.Encoder` protocol handles serialization
to wire format (RFD 14) and to JSON (for the web client, RFD 15).

```elixir
defmodule Crucible.WorldOutput do
  @moduledoc """
  A structured narrative message emitted by the server on each tick.
  """

  @type source :: :world | :npc | :system | :error
  @type tag :: :room_desc | :dialogue | :combat | :event | :system

  defstruct [:source, :source_name, :body, :tags]

  @type t :: %__MODULE__{
    source: source(),
    source_name: String.t(),
    body: String.t(),
    tags: [tag()]
  }

  @spec new(source(), String.t(), [tag()]) :: t()
  def new(source, body, tags \\ []) do
    %__MODULE__{source: source, source_name: "", body: body, tags: tags}
  end

  @spec new(source(), String.t(), String.t(), [tag()]) :: t()
  def new(source, source_name, body, tags \\ []) do
    %__MODULE__{source: source, source_name: source_name, body: body, tags: tags}
  end
end
```

The encoder protocol decouples the struct from its wire representation:

```elixir
defprotocol Crucible.WorldOutput.Encoder do
  @spec encode(t(), :binary | :json) :: binary()
  def encode(output, format)
end
```

### Examples — struct form

```elixir
# Room description
Crucible.WorldOutput.new(:world,
  "You are in the Town Square. A fountain burbles.",
  [:room_desc])

# NPC dialogue
Crucible.WorldOutput.new(:npc, "Watch Captain",
  ~S("Keep your eyes open, stranger."),
  [:dialogue])

# System event
Crucible.WorldOutput.new(:system,
  "You pick up the rusty key.",
  [:event])

# Error response
Crucible.WorldOutput.new(:error,
  "Unknown command. Try /help.")
```

## Wire encoding

The `Encoder` protocol's binary implementation produces bit-crushed
frames (RFD 14):

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

The JSON implementation serializes to:

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

## Open questions

- Should output include rich text (bold, italic) or stay plain?
- Multi-line body — newlines preserved or wrapped?

## Implementation status

- [ ] Elixir `WorldOutput` struct defined
- [ ] `WorldOutput.Encoder` protocol with binary implementation
- [ ] `WorldOutput.Encoder` JSON implementation (web client)
- [ ] Tick loop emits `WorldOutput.t()` on each tick

## See also

- **RFD 7**: Slash command protocol
- **RFD 14**: Wire format — bit-crushed binary frames
- **RFD 15**: Web client
