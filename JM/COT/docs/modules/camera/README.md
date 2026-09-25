---
name: Camera Module
description: Freecam controls, cinematic camera tools, spectator modes, movement speed, FOV adjustments, and position synchronization
type: project
---

## Overview

`JMCameraModule` provides administrative freecam and spectator camera functionality. Admins can detach their view from their character, fly around the map, adjust FOV/speed/smoothing, view entity details, and teleport their character to the camera location.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Camera/JMCameraModule.c` — Main camera module, camera instantiation, state handling, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Camera/JMCameraForm.c` — Camera control UI form (speed, FOV, camera mode toggles)
- `Scripts/5_Mission/CommunityOnlineTools/modules/Camera/JMCinematicCamera.c` — Freecam implementation with smooth interpolation and keyframe support
- `Scripts/5_Mission/CommunityOnlineTools/modules/Camera/JMSpectatorCamera.c` — Spectator camera tracking target entities

## RPC Range

Range: `10160` – `10164` (`JMCameraModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10160` | `SetPosition` | Client → Server | Updates server with admin's freecam position for network streaming |
| `10161` | `TeleportTo` | Client → Server | Teleports character body to current freecam camera location |
| `10162` | `EnterFreecam` | Client → Server | Notifies server that admin activated freecam |
| `10163` | `LeaveFreecam` | Client → Server | Notifies server that admin exited freecam |
| `10164` | `SyncSettings` | Client ↔ Server | Syncs camera settings (speed presets, smoothing options) |

## Key Controls & Capabilities

- **Freecam Toggle**: Keybind activates freecam mode (`JMCinematicCamera`).
- **Speed & FOV Sliders**: Adjust camera movement speed (slow walk to rapid fly) and field of view dynamically.
- **Teleport to Camera**: Pressing teleport key places the player character directly at the camera's focus position.
- **Invisibility / Godmode Integration**: Auto-enables godmode or character protection while freecam mode is active.
