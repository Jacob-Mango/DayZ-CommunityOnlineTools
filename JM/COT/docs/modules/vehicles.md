---
name: Vehicles Module
description: Vehicle management form, engine state, refuel, repair, lock/unlock, seat management, and vehicle deletion
type: project
---

## Overview

The Vehicles Module (`JMVehiclesModule`) provides comprehensive management of vehicles in the world. Admins can view a list of spawned vehicles, inspect health and fluid levels (fuel, coolant, oil, brake fluid), repair broken parts, refuel, lock/unlock doors, manage seats, engine state, or delete vehicles.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Vehicles/JMVehiclesModule.c` — Vehicle operations controller, fluid refill, attachment repair, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Vehicles/JMVehiclesForm.c` — Master UI window for vehicle selection and property manipulation

## RPC Range

Range: `10650` – `10670` (`JMVehiclesModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10650` | `Sync` | Server → Client | Syncs spawned vehicle roster to admin UI |
| `10651` | `Refuel` | Client → Server | Sets vehicle fuel tank to 100% capacity |
| `10652` | `Repair` | Client → Server | Restores vehicle health, engine, and attached parts to Pristine state |
| `10653` | `Engine` | Client → Server | Force starts or stops vehicle engine |
| `10654` | `Lock` | Client → Server | Toggles vehicle door lock state |
| `10655` | `Delete` | Client → Server | Removes vehicle object from the world |
| `10656` | `TeleportTo` | Client → Server | Teleports admin to target vehicle |
| `10657` | `Bring` | Client → Server | Teleports vehicle to admin position |

## Permissions

- `PERM_VEHICLES_VIEW` (`Vehicles.View`) — Open vehicle management UI
- `PERM_VEHICLES_REPAIR` (`Vehicles.Repair`) — Repair vehicles and attached components
- `PERM_VEHICLES_REFUEL` (`Vehicles.Refuel`) — Refill vehicle fluids (fuel, oil, water, coolant)
- `PERM_VEHICLES_DELETE` (`Vehicles.Delete`) — Delete vehicles from the server
- `PERM_VEHICLES_LOCK` (`Vehicles.Lock`) — Force unlock or lock vehicle doors
