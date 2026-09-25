---
name: Territory Module
description: DayZ-Expansion base building territory management, territory listing, member permissions, and admin override
type: project
---

## Overview

`JMTerritoryModule` provides integration with DayZ-Expansion's base building territory system. It allows server administrators to inspect active territories, view territory members and flags, modify territory boundaries, or delete/override territory registrations.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Territory/JMTerritoryModule.c` — Territory integration, Expansion territory queries, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Territory/JMTerritoryForm.c` — UI window listing active territories, member lists, and territory actions

## RPC Range

Range: `10560` – `10563` (`JMTerritoryModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10560` | `Sync` | Server → Client | Syncs registered territories list to client UI |
| `10561` | `Delete` | Client → Server | Admin deletion of a territory definition and flag |
| `10562` | `Update` | Client → Server | Updates territory attributes (radius, level, ownership) |
| `10563` | `TeleportTo` | Client → Server | Teleports admin to territory flag center |

## Conditional Compilation

This module is compiled into standard builds when `EXPANSIONMODBASEBUILDING` is defined (i.e., when DayZ-Expansion BaseBuilding is present).
