---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: protocol
stage: mvp
---

# RFD 7: Slash command protocol

## Command set

| Command | Args | Description |
|---------|------|-------------|
| `/look` | — | Describe current room |
| `/look at <target>` | NPC or object name | Examine a target |
| `/move <direction>` | n, s, e, w | Move to adjacent room |
| `/talk <npc>` | NPC name | Start dialogue with NPC |
| `/say <text>` | message | Speak to NPCs in room |
| `/take <object>` | object name | Pick up an object |
| `/use <object> [target]` | object + optional target | Use an object |
| `/inventory` | — | List carried items |
| `/help` | — | Show available commands |

## Protocol

Client connects via WebSocket. Server sends `>> ` on each ready tick.
Client sends raw slash command lines, no JSON wrapper.

```
>> /look
You are in the Town Square. A fountain burbles in the center.
The Watch Captain stands nearby, watching the crowd.

>> /talk captain
"You there," the Captain says. "Keep your eyes open. Strangers about."
```

The same protocol applies to every client — human, scripted bot,
or automated evaluation harness. The server does not distinguish
between connection types (see RFD 2).

Wire frames are bit-crushed binary, not text/JSON (see RFD 14).

## Implementation status

- [x] Command set defined
- [ ] Protocol documentation
- [ ] Command parser
- [ ] Automated client adapter
