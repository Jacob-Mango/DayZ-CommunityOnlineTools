---
name: Entity Manager Module
description: Live active entity inspector, active entity grid list, deletion, and entity state inspection
type: project
---

## Overview

`JMEntityManagerModule` provides a live browser for all active entities loaded in the server's network bubble or world instance. Admins can filter entities by type, inspect health/distance/position, and perform bulk deletion or cleanup.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/gui/EntityManager/JMEntityManagerModule.c` — Entity manager module controller, scanning logic
- `Scripts/5_Mission/CommunityOnlineTools/gui/EntityManager/JMEntityManagerForm.c` — UI window with entity list grid, filter search, and control buttons

## Permissions

- `PERM_ENTITYMANAGER_VIEW` (`EntityManager.View`) — Open Entity Manager form and browse active entities
- `PERM_ENTITYMANAGER_DELETE` (`EntityManager.Delete`) — Delete selected entities from the world

## Capabilities

- **Entity Filter Grid**: Filter active entities by category (Animals, Infected, Items, Vehicles, Base Building, Static Objects).
- **Inspection**: Displays network ID, class name, distance from admin, exact coordinates, and health status.
- **Bulk Actions**: Delete selected entity, delete all entities of matching type within radius, or copy coordinates/class names to clipboard.
