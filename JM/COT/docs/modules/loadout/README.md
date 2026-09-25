---
name: Loadout Module
description: Loadout presets management, kit spawning, player inventory restoration, and Loadouts/ JSON storage
type: project
---

## Overview

`JMLoadoutModule` enables administrators to save, edit, and apply full gear loadouts (clothing, weapons, quickbar items, attachments, inventory contents) to themselves or target players. Loadout presets are stored as JSON files on the server.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Loadout/JMLoadoutModule.c` — Loadout module manager, inventory serializer/deserializer, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Loadout/JMLoadoutForm.c` — UI window for managing loadout presets and applying loadouts to target players

## RPC Range

Range: `10460` – `10466` (`JMLoadoutModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10460` | `Sync` | Server → Client | Syncs list of saved loadout preset names |
| `10461` | `Apply` | Client → Server | Applies chosen loadout preset to target player |
| `10462` | `Save` | Client → Server | Saves target player's current equipment as a new loadout preset |
| `10463` | `Delete` | Client → Server | Deletes a saved loadout preset file |
| `10464` | `Request` | Client → Server | Requests contents preview of a loadout preset |
| `10465` | `Send` | Server → Client | Sends loadout structure payload to client preview |
| `10466` | `Log` | Server → Client | Administrative audit log notification |

## Permissions

- `PERM_LOADOUT_VIEW` (`Loadout.View`) — View loadouts form
- `PERM_LOADOUT_APPLY` (`Loadout.Apply`) — Apply loadouts to players
- `PERM_LOADOUT_SAVE` (`Loadout.Save` / `Loadout.Create`) — Save current player gear as a new preset
- `PERM_LOADOUT_DELETE` (`Loadout.Delete`) — Delete saved loadout presets

## File Storage

Loadouts are stored under `$profile:COT/Loadouts/*.json`. Each JSON file defines:
- Clothing items (headgear, mask, body, vest, legs, feet, back, gloves)
- Weapons, attached optics, magazines, and loaded ammunition
- Nested inventory containers and cargo items
