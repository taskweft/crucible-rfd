---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: protocol
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

For LLM evaluation, the same protocol applies: the LLM client receives
observation text and sends command text. Evaluation logs capture every
command-observation pair.

## Implementation status

- [x] Command set defined
- [ ] Protocol documentation
- [ ] Command parser
- [ ] LLM client adapter
