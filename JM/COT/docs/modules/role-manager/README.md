---
name: Role Manager Module
description: Roles and permissions hierarchy (roles.json), per-permission node assignment, and admin role management
type: project
---

## Overview

`JMRoleManagerModule` controls COT's permission and role system. Roles (such as Admin, Moderator, VIP) define a set of permission nodes (`PERM_*`). Roles are assigned to player GUIDs or Steam64 IDs and persisted in `$profile:COT/roles.json`.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/RoleManager/JMRoleManagerModule.c` — Permission registry loader, role hierarchy builder, permission verification, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/RoleManager/JMRoleManagerForm.c` — UI window for creating roles, assigning permission nodes, and assigning roles to players

## RPC Range

Range: `10500` – `10505` (`JMRoleManagerModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10500` | `Sync` | Server → Client | Syncs role list and permission trees to admin UI |
| `10501` | `CreateRole` | Client → Server | Creates a new role definition |
| `10502` | `DeleteRole` | Client → Server | Deletes an existing role |
| `10503` | `UpdateRole` | Client → Server | Modifies permission nodes associated with a role |
| `10504` | `AssignRole` | Client → Server | Assigns or revokes a role for a target player GUID |
| `10505` | `Log` | Server → Client | Permission audit log notification |

## Permissions

- `PERM_ROLEMANAGER_VIEW` (`RoleManager.View`) — Open Role Manager UI
- `PERM_ROLEMANAGER_MANAGE` (`RoleManager.Manage`) — Create, modify, or delete roles and assign permissions

## System Integration

For detailed architectural rules on how permissions propagate, see [../../codebase/permissions-system.md](../../codebase/permissions-system.md).
- **Default Roles**: COT ships with default roles (`Everyman`, `Admin`).
- **Inheritance**: Roles can inherit permissions from parent roles.
