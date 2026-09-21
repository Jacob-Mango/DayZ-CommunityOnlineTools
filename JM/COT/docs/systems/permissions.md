---
name: Permissions System
description: JMPermissions API and JMConstants.PERM_* constants, DeclarePermissions, server-side checks with PlayerIdentity, roles system, JMPlayerInstance fields, OnClientPermissionsUpdated
type: project
---

## Overview

COT uses a hierarchical permissions system (`GetPermissionsManager()`). Permissions are dot-separated strings. Granting a parent node (e.g. `Admin.MyModule`) implicitly grants all children.

## The API: `JMPermissions` and `JMConstants.PERM_*`

Module code goes through the static `JMPermissions` class, never `GetPermissionsManager()` directly, and passes every
node as a `JMConstants.PERM_*` constant - never a string literal. A literal typo is not an error: an unregistered node
silently resolves to its nearest registered parent, so the check quietly tests something else.

| Call | Meaning |
|---|---|
| `JMPermissions.Register( node )` | declare a node (in `DeclarePermissions()` only) |
| `JMPermissions.Has( node )` | client: does the local admin hold it? |
| `JMPermissions.Has( node, sender )` | does this player hold it? |
| `JMPermissions.Has( node, sender, instance )` | same, and hand back the checked `JMPlayerInstance` |
| `JMPermissions.HasRPC( node, sender [, instance] )` | server-side gate for an incoming RPC; a refusal is reported to anti-cheat |
| `JMPermissions.HasAny( nodes, sender )` / `HasAll( nodes, sender )` | several at once (`HasAll` of an empty list is false) |

`Has`, `HasRPC` and `Register` do not guard against a missing permission manager or a missing sender: a `NULL` there is the
calling mod's bug, and it fails loudly (a script exception in the log) instead of answering `false`. The sender of an RPC
received on the server is never `NULL`, so pass it through. Do not wrap these calls in a check that hides the failure.
Offline (singleplayer), the manager answers `true` for every node.

## Registering Permissions

Override `DeclarePermissions()` on the module. It runs on both client and server as the module registers, before the
mission loads - registering from a form, or after the mission loaded, is an error.

```c
override void DeclarePermissions()
{
    super.DeclarePermissions();

    JMPermissions.Register( JMConstants.PERM_MYMODULE_VIEW );
    JMPermissions.Register( JMConstants.PERM_MYMODULE_ACTION );
}
```

Every node has a constant in `JMConstants` (a mod declares its own in a `3_Game` class of its own). The **value** of a
constant is the key saved in role files - never change one, add a new constant instead.

## Checking Permissions

### Client-side (gate UI):
```c
override bool HasAccess()
{
    return JMPermissions.Has( JMConstants.PERM_MYMODULE_VIEW );
}

// In a form - bind a control once; it is re-evaluated on every permission change:
BindPermission( m_ActionButton, JMConstants.PERM_MYMODULE_ACTION );
```

### Server-side (validate before executing - always required):
```c
// Simple check with PlayerIdentity:
if ( !JMPermissions.HasRPC( JMConstants.PERM_MYMODULE_ACTION, sender ) )
    return;

// Check + get player instance (for logging/webhook):
JMPlayerInstance instance;
if ( !JMPermissions.HasRPC( JMConstants.PERM_MYMODULE_ACTION, sender, instance ) )
    return;
// instance.GetName(), instance.GetSteam64ID(), instance.PlayerObject, etc.
```

**Always re-validate permissions server-side. Never trust client state.**

## Permission Update Callback

Called on both module and form when permissions change:
```c
// Module:
override void OnClientPermissionsUpdated()
{
    super.OnClientPermissionsUpdated();
    // Form will also receive this callback automatically
}

// Form:
override void OnClientPermissionsUpdated()
{
    super.OnClientPermissionsUpdated();
    
    // Prefer BindPermission() once (above). For controls rebuilt on every refresh use UpdatePermission():
    m_DangerousButton.UpdatePermission( JMConstants.PERM_MYMODULE_DANGEROUS );
    m_ReadButton.UpdatePermission( JMConstants.PERM_MYMODULE_VIEW );
}
```

`UIActionBase.UpdatePermission(string)` shows/hides a disable overlay automatically. `JMFormBase.BindPermission( control, node )`
tracks the control and re-applies it on every permission change; `JMFormBase.UpdatePermission( control, node )` applies once
without tracking. `BindPanelPermission` is deprecated (use `BindPermission`).

## Roles System

- Two default roles are created by `COTModule.EnsureDefaultRoles()` (server, on mission load): `"everyone"` (created once, from
  the registered permission set, then editable) and `"admin"` (re-created on **every** start with every permission set to
  `2` / ALLOW, so it stays in sync as mods register new nodes - do not hand-edit it)
- Every player gets the `"everyone"` role on first load
- Permission state per node is `JMPermissionType`: `INHERIT` (0), `DISALLOW` (1), `ALLOW` (2)
- Per-player data (roles, own permissions) stored in: `$profile:PermissionsFramework/Players/` (`.json`)
- Role definitions stored in: `$profile:PermissionsFramework/Roles/` (`.txt`)
- Roles are edited in-game by the Role Manager module (`JMRoleManagerModule`, RPCs in `JMRoleManagerModuleRPC`)

## JMPlayerInstance

Per-player permission state plus a snapshot of in-game state, refreshed from `PlayerObject` on the server every 100 ms
(`Update()`). Read it through getters - the data fields are `protected`.

```c
// Identity / lookup
string GetName()          // display name
string GetGUID()          // network GUID
string GetSteam64ID()     // Steam ID
PlayerBase PlayerObject;  // actual in-game entity (public field)

// Stats snapshot
vector GetPosition() / GetOrientation()
float  GetHealth() / GetBlood() / GetShock() / GetEnergy() / GetWater() / GetStamina()
float  GetHeatComfort() / GetHeatBuffer() / GetWet() / GetTremor()
int    GetAvgPing() / GetMinPing() / GetMaxPing()

// COT flags (stored in m_PlayerVars, keyed by the JMPlayerVariables bit enum)
bool HasGodMode() / IsFrozen() / HasInvisibility() / IsRagdoll()
bool HasUnlimitedAmmo() / HasUnlimitedStamina() / HasAdminNVG() / HasBrokenLegs() / HasBloodyHands()
bool GetReceiveDmgDealt() / GetCannotBeTargetedByAI() / GetRemoveCollision()
bool IsDead() / IsUnconscious() / IsSick() / IsBleeding()

// Permissions / roles
bool HasPermission( string permission )
bool HasRole( string role )
void AddRole( string role, string nameRestriction = "" )
```

There are two builds of this class, chosen by `CF_MODULE_PERMISSIONS`: without it (the normal case) `JMPlayerInstance` is
defined in `Scripts/4_World/CommunityOnlineTools/Classes/PermissionsOld/JMPlayerInstance.c`; with it,
`Scripts/3_Game/CommunityOnlineTools/Permissions/JMPlayerInstance.c` instead mods CF's `CF_Permission_PlayerBase` to add the
same getters. Keep the two in sync (mods such as DayZ-Expansion compile against either). The `JMPlayerVariables` bit
enum (`GODMODE`, `FROZEN`, `INVISIBILITY`, ... `RAGDOLL`) is what the server syncs to clients.

## Getting a Player Instance

```c
// By GUID string:
JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );

// From permission check (server-side):
JMPlayerInstance instance;
JMPermissions.Has( JMConstants.PERM_X, sender, instance );

// Current client:
JMPlayerInstance me = GetPermissionsManager().GetClientPlayer();
string myGUID = GetPermissionsManager().GetClientGUID();
```

## Common Permission Strings (existing patterns)

```
COT.View
Admin.Player.Read
Admin.Player.Heal
Admin.Player.Heal.Attachments
Admin.Player.Heal.Cargo
Admin.Player.Teleport.Position
Admin.Player.Teleport.Location
Admin.Player.Teleport.Position.Cursor
Admin.Player.GodMode
Admin.Player.Kick
Admin.Player.Ban
Admin.Player.Set.Health
Admin.Ban.View
Admin.Ban.Unban
Admin.Roles.Update
Actions.QuickActions
Weather.View
Weather.QuickAction
Weather.Preset
Weather.Preset.Use
Weather.Preset.Create
Camera.View
```

This is a sample, not the full list - the authoritative set is the `PERM_*` block in `JMConstants.c`, plus whatever a module
registers itself. Not every module follows `Admin.<ModuleName>.<Action>` (Weather, Vehicles and Camera use
`<Module>.<Action>`); for a new module prefer `<Module>.View` / `<Module>.<Action>` like those, and never rename an existing value.
