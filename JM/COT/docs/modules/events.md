---
name: Events Module
description: Server event manager, dynamic event triggers, event spawning presets, and server event configuration
type: project
---

## Overview

`JMEventsModule` allows server administrators to manage, trigger, and spawn dynamic world events (such as helicopter crashes, air drops, animal packs, or custom modded events) directly from the COT interface without restarting the server.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Events/JMEventsModule.c` — Main event manager, event lifecycle hooks, RPC dispatchers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Events/JMEventsForm.c` — UI window listing available server events and active event states

## RPC Range

Range: `10540` – `10545` (`JMEventsModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10540` | `Sync` | Server → Client | Syncs list of registered events and active instances |
| `10541` | `Spawn` | Client → Server | Manually triggers/spawns a selected event preset |
| `10542` | `Cancel` | Client → Server | Terminates an active event instance |
| `10543` | `Update` | Client → Server | Modifies event configuration settings |
| `10544` | `Log` | Server → Client | Event log notification |

## Permissions

- `PERM_EVENTS_VIEW` (`Events.View`) — View the event list and active events
- `PERM_EVENTS_SPAWN` (`Events.Spawn`) — Trigger event spawning
- `PERM_EVENTS_MANAGE` (`Events.Manage`) — Configure event parameters and schedule rules
