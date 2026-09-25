---
name: Compensations Module
description: Player reward and compensation kit creation, JSON kit definitions, and distribution RPCs
type: project
---

## Overview

`JMCompensationsModule` allows admins to define compensation packages (pre-configured item kits/inventories) and grant them to players who lost items due to bugs or server resets. Packages are defined in JSON and can be delivered directly to a player's inventory or ground location.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Compensations/JMCompensationsModule.c` — Compensation kit manager, JSON storage, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Compensations/JMCompensationsForm.c` — UI form for viewing compensation kits and assigning them to players

## RPC Range

Range: `10480` – `10485` (`JMCompensationsModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10480` | `Sync` | Server → Client | Syncs available compensation templates to client |
| `10481` | `Give` | Client → Server | Awards a compensation package to a specified player GUID |
| `10482` | `Create` | Client → Server | Saves a new compensation template definition |
| `10483` | `Delete` | Client → Server | Deletes a compensation template |
| `10484` | `Claim` | Client → Server | Player claim request for pending compensations |
| `10485` | `Log` | Server → Client | Log event notification for compensation claim/grant |

## Permissions

- `PERM_COMPENSATIONS_VIEW` (`Compensations.View`) — View compensation kits in UI
- `PERM_COMPENSATIONS_GIVE` (`Compensations.Give`) — Grant compensation kits to online or offline players
- `PERM_COMPENSATIONS_MANAGE` (`Compensations.Manage`) — Create, edit, or delete compensation package templates
