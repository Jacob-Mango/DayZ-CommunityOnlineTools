---
name: COT Map Module
description: Interactive COT map interface, player/vehicle position markers, click-to-teleport, and coordinate filter overlay
type: project
---

## Overview

`JMMapModule` powers COT's interactive full-screen map interface (`JMMapForm`). It renders live markers for online players, vehicles, territories, and custom points of interest, allowing admins to double-click anywhere on the map to teleport.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/COTMap/JMMapModule.c` — Map module controller, marker registration, coordinate conversion helpers
- `Scripts/5_Mission/CommunityOnlineTools/modules/COTMap/JMMapForm.c` — Full-screen map GUI form containing `MapWidget`

## Permissions

- `PERM_MAP_VIEW` (`Map.View`) — Open the COT Map interface
- `PERM_MAP_PLAYERS` (`Map.Players`) — Render live player position markers on the map
- `PERM_MAP_VEHICLES` (`Map.Vehicles`) — Render live vehicle markers on the map
- `PERM_MAP_TELEPORT` (`Map.Teleport`) — Click/double-click on the map to teleport your player or selected player to target coordinates

## Key Features

- **Dynamic Marker Filtering**: Toggle visibility for players, dead bodies, vehicles, base structures, or custom markers.
- **Click-to-Teleport**: Double-clicking on map coordinates calculates world terrain height (`g_Game.SurfaceY`) and sends a teleport request via `JMTeleportModule`.
- **Coordinate Display**: Shows cursor map coordinates (X, Z) and surface height (Y) in real time.
