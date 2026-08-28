---
name: Permissions System
description: Registering permissions, HasPermission checks with PlayerIdentity, roles system, JMPlayerInstance fields, OnClientPermissionsUpdated
type: project
---

## Overview

COT uses a hierarchical permissions system (`GetPermissionsManager()`). Permissions are dot-separated strings. Granting a parent node (e.g. `Admin.MyModule`) implicitly grants all children.

## Registering Permissions

Done in the module **constructor** (runs on both client and server):

```c
void JMMyModule()
{
    GetPermissionsManager().RegisterPermission( "Admin.MyModule.View" );
    GetPermissionsManager().RegisterPermission( "Admin.MyModule.Read" );
    GetPermissionsManager().RegisterPermission( "Admin.MyModule.Action" );
    GetPermissionsManager().RegisterPermission( "Admin.MyModule.Action.Dangerous" );
}
```

## Checking Permissions

### Client-side (gate UI):
```c
override bool HasAccess()
{
    return GetPermissionsManager().HasPermission( "Admin.MyModule.View" );
}

// In form — show/hide/disable controls:
if ( GetPermissionsManager().HasPermission( "Admin.MyModule.Action" ) )
    m_ActionButton.Show();
```

### Server-side (validate before executing — always required):
```c
// Simple check with PlayerIdentity:
if ( !GetPermissionsManager().HasPermission( "Admin.MyModule.Action", sender ) )
    return;

// Check + get player instance (for logging/webhook):
JMPlayerInstance instance;
if ( !GetPermissionsManager().HasPermission( "Admin.MyModule.Action", sender, instance ) )
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
    
    // Use UIActionBase.UpdatePermission() to gate controls:
    m_DangerousButton.UpdatePermission( "Admin.MyModule.Action.Dangerous" );
    m_ReadButton.UpdatePermission( "Admin.MyModule.Read" );
}
```

`UIActionBase.UpdatePermission(string)` shows/hides a disable overlay automatically.

## Roles System

- Built-in roles: `"everyone"` (configurable defaults), `"admin"` (all permissions)
- Per-player roles stored in: `$profile:PermissionsFramework/Players/`
- Role definitions stored in: `$profile:PermissionsFramework/Roles/`
- Admin role gives all permissions at level 2 (full access)

## JMPlayerInstance Fields

Extended player state — contains both permissions and in-game COT flags.

```c
// Identity / lookup
string GetName()          // display name
string GetGUID()          // network GUID
string GetSteam64ID()     // Steam ID
PlayerBase PlayerObject;  // actual in-game entity

// Stats
vector m_Position;
vector m_Orientation;
float m_Health;
float m_Blood;
float m_Shock;
float m_Energy;
float m_Water;
float m_Stamina;

// COT flags
bool m_GodMode;
bool m_Frozen;
bool m_Invisibility;
bool m_UnlimitedAmmo;
bool m_UnlimitedStamina;
bool m_AdminNVG;
bool m_BrokenLegs;
bool m_ReceiveDamageDealt;
bool m_CannotBeTargetedByAI;
bool m_RemoveCollision;
```

File: `JM/COT/Scripts/3_Game/CommunityOnlineTools/Permissions/JMPlayerInstance.c`

## Getting a Player Instance

```c
// By GUID string:
JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );

// From permission check (server-side):
JMPlayerInstance instance;
GetPermissionsManager().HasPermission( "Admin.X", sender, instance );

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
Admin.Player.Teleport.Cursor
Admin.Player.Teleport.View
Admin.Player.Godmode
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

Pattern: `Admin.<ModuleName>.<Action>` — follow it for new modules.
