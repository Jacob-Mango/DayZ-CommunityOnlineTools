---
name: Map Editor Module
description: In-game 3D object placement editor, transform gizmos/manipulators, object snapping, mapping file import/export
type: project
---

## Overview

`JMMapEditorModule` provides an in-game 3D map editing toolset. Admins can spawn, move, rotate, scale, clone, and align world objects visually using 3D manipulators, and export placed object sets to JSON files or mapping scripts.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/MapEditor/JMMapEditorModule.c` — Map editor controller, object list manager, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/MapEditor/JMMapEditorForm.c` — Map editor UI toolbar, object browser, transform sliders, import/export controls

## RPC Range

Range: `10900` – `10912` (`JMMapEditorModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10900` | `SpawnObject` | Client → Server | Spawns a new object in editor context |
| `10901` | `DeleteObject` | Client → Server | Deletes an editor-managed object |
| `10902` | `SetTransform` | Client → Server | Updates position, rotation (pitch/yaw/roll), or scale |
| `10903`..`10911` | `Group/Save` | Client ↔ Server | Group selection, object snapping, batch move, set save/load |
| `10912` | `Export` | Client → Server | Exports current editor map layout to file |

## Permissions

- `PERM_MAPEDITOR_VIEW` (`MapEditor.View`) — Open Map Editor interface
- `PERM_MAPEDITOR_PLACE` (`MapEditor.Place`) — Spawn and manipulate objects in editor
- `PERM_MAPEDITOR_SAVE` (`MapEditor.Save`) — Export or save map editor layouts

## Key Capabilities

- **3D Transform Gizmos**: Visual handles for translation (X/Y/Z) and rotation (Pitch/Yaw/Roll).
- **Surface Snapping**: Toggle alignment to terrain surface normal or ground collision.
- **Set Import & Export**: Save placed objects to `$profile:COT/MapEditor/` as JSON mappings compatible with custom server map loaders.
