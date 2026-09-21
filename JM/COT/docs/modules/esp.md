---
name: ESP Module
description: Extra Sensory Perception (ESP) visual overlay, entity filter types, range sliders, bounding box rendering, and RPC entity network sync
type: project
---

## Overview

The ESP Module (`JMESPModule`) renders visual overlays (labels, 3D bounding boxes, health bars, distance indicators) on entities in the world. It provides granular filtering for players, infected, animals, vehicles, items, and base building objects.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/ESP/JMESPModule.c` — ESP network controller, scan loop, RPC receiver
- `Scripts/5_Mission/CommunityOnlineTools/modules/ESP/JMESPForm.c` — UI settings window for toggling filters, colors, and render distances
- `Scripts/5_Mission/CommunityOnlineTools/modules/ESP/JMESPView.c` — Screen projection math, widget canvas rendering, bounding box draw routines

## RPC Range

Range: `10300` – `10321` (`JMESPModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10300` | `Sync` | Server → Client | Streams entity data for targets outside local network bubble |
| `10301` | `Request` | Client → Server | Requests extended entity sync for active ESP scanning |
| `10302`..`10321` | `Action` | Client ↔ Server | Triggers specific ESP object actions (`JMESPObjectAction`) |

## Permissions

- `PERM_ESP_VIEW` (`ESP.View`) — Master permission to activate ESP
- `PERM_ESP_PLAYER` (`ESP.Player`) — Render ESP overlay for players
- `PERM_ESP_INFECTED` (`ESP.Infected`) — Render ESP overlay for infected/zombies
- `PERM_ESP_VEHICLE` (`ESP.Vehicle`) — Render ESP overlay for vehicles
- `PERM_ESP_ITEM` (`ESP.Item`) — Render ESP overlay for items and loot

## Customization & Rendering

- **Render Distance**: Adjustable sliders per category (0 m to 1000+ m).
- **Style Options**: Bounding box mode (Corner, Box, Point), line thickness, color presets per category, text labels (Name, Distance, Health %, Inventory contents).
- **Target Actions**: Right-clicking an ESP tag allows direct admin actions (Teleport To, Bring, Inspect Inventory, Delete Entity).
