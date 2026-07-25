---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: client,web
stage: mvp
---

# RFD 15: Web client — Discord-like slash commands

## Goal

A browser-based MUD client that looks and feels like a Discord channel.
The player types slash commands into an input box and sees world output
scroll up like chat messages.

## Interface

```
┌─────────────────────────────────────────┐
│  Crucible — Medieval Town              │
├─────────────────────────────────────────┤
│                                         │
│  [World] You are in the Town Square.    │
│  [World] A fountain burbles in the      │
│          center. The Watch Captain      │
│          watches the crowd.             │
│                                         │
│  >> /look                              │
│  [World] You are in the Town Square.    │
│          Exits: north (Market St),      │
│          south (Old Well).              │
│                                         │
│  >> /talk captain                      │
│  [Captain] "Keep your eyes open,       │
│            stranger. Trouble about."    │
│                                         │
├─────────────────────────────────────────┤
│  [/] [____________________________] [➤] │
└─────────────────────────────────────────┘
```

## Design

- **Single-page HTML/JS** — no build step, no framework. Served by
  libh2o as a static file alongside the WebSocket endpoint.
- **WebSocket transport** — connects to `ws://host/ws`, sends binary
  frames (bit-crushed, see RFD 14), receives binary frames decoded to
  world output.
- **Slash command input** — `/command [args]` in the text box. Enter
  submits. Command history with up/down arrow keys.
- **World output** — scrollback buffer. Each response rendered as a
  message block with source tag ([World], [NPC name], [System]).
- **Autoscroll** — new content scrolls into view unless the player has
  scrolled up to read history.

## Open questions

- How to handle reconnection on temporary network loss?
- Should the client render markdown or only plain text?
- Player authentication — none for alpha, or simple name entry?

## Implementation status

- [ ] Static HTML/JS page
- [ ] WebSocket connect and binary frame send/receive
- [ ] /command input with history
- [ ] World output scrollback
- [ ] Source tag rendering ([World], [NPC], [System])

## See also

- **RFD 7**: Slash command protocol
- **RFD 14**: Wire format — bit-crushed binary frames
- **RFD 16**: World output format
