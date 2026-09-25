---
name: Object Spawner Module
description: Entity & item spawner, inventory placement vs ground placement, search filters, quantity/health presets, and favorite items list
type: project
---

## Overview

`JMObjectSpawnerModule` powers COT's item and object spawning tool (`JMObjectSpawnerForm`). Administrators can search the game's entity class hierarchy, configure spawn flags (health, quantity, attachment state), and spawn items directly into hands, target player inventory, or on the ground.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Object/JMObjectSpawnerModule.c` — Server spawning dispatcher, item instantiation, attachment handling, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Object/JMObjectSpawnerForm.c` — UI window with entity list grid, category filters, search input, spawn target selection, and favorites panel

## RPC Range

Range: `10220` – `10223` (`JMObjectSpawnerModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10220` | `Spawn` | Client → Server | Spawns selected item class with quantity/health parameters |
| `10221` | `Delete` | Client → Server | Deletes target object in cursor focus |
| `10222` | `SyncFavorites` | Client ↔ Server | Syncs admin favorite item presets |
| `10223` | `Log` | Server → Client | Audit log entry for object spawn operations |

## Permissions

- `PERM_OBJECT_SPAWN` (`Object.Spawn` / `Items.Spawn`) — Spawn objects/items in the world or player inventory
- `PERM_OBJECT_DELETE` (`Object.Delete` / `Items.Delete`) — Delete targeted objects in world focus

## Key Features

- **Spawn Locations**:
  - **In Hands**: Places item directly into target player's hands.
  - **In Inventory**: Attempts auto-placement into target player's empty clothing or backpack slots.
  - **At Crosshair / Ground**: Spawns item at cursor collision point or player feet.
- **Item Parameters**: Configurable quantity, health percentage (Pristine, Worn, Damaged, Badly Damaged, Ruined), wetness, and battery/magazine filling options.
- **Favorites & Presets**: Quick-access favorites bar saved per admin profile.
