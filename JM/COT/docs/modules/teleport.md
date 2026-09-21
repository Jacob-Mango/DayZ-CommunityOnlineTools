---
name: Teleport Module
description: Predefined and custom teleport locations (teleports.json), position saving, click/admin teleportation, and history
type: project
---

## Overview

`JMTeleportModule` manages teleportation functionality across COT. It allows admins to teleport to predefined locations (e.g. major towns, military bases, airfields), save custom landmark coordinates to `$profile:COT/teleports.json`, teleport to crosshair position, teleport players to other players, and maintain a back/forward position history.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Teleport/JMTeleportModule.c` — Teleportation execution, location list storage, vehicle/passenger position sync, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Teleport/JMTeleportForm.c` — UI window listing categories, locations, coordinate inputs, and save/delete buttons

## RPC Range

Range: `10240` – `10247` (`JMTeleportModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10240` | `TeleportToPosition` | Client → Server | Teleports target player/vehicle to explicit X, Y, Z vector |
| `10241` | `TeleportToPlayer` | Client → Server | Teleports source player to target player's current location |
| `10242` | `BringPlayer` | Client → Server | Teleports target player to source admin's location |
| `10243` | `SavePosition` | Client → Server | Saves new teleport preset location |
| `10244` | `DeletePosition` | Client → Server | Removes saved teleport preset |
| `10245` | `SyncPositions` | Server → Client | Syncs preset location database to client |
| `10246` | `ReturnPrevious` | Client → Server | Teleports player back to previous position before last teleport |
| `10247` | `Log` | Server → Client | Teleport action audit log |

## Permissions

- `PERM_TELEPORT_VIEW` (`Teleport.View`) — Open teleport UI
- `PERM_TELEPORT_MAP` (`Teleport.Map`) — Teleport via map click or coordinate entry
- `PERM_TELEPORT_PLAYER` (`Teleport.Player`) — Teleport to another player or bring player
- `PERM_TELEPORT_MANAGE` (`Teleport.Manage`) — Create or delete saved location presets

## Vehicle Handling

When a player is inside a vehicle and teleports, `JMTeleportModule` relocates the vehicle object and all attached passengers together, validating ground collision height to prevent vehicle clipping.
