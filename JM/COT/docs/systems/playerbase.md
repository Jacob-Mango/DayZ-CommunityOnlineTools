---
name: PlayerBase Modded Methods and Module Server Internals
description: COT additions to PlayerBase (godmode/freeze/invisibility methods), JMPlayerModule Exec_ pattern for all actions, all registered player permissions, static helper functions
type: project
---

## PlayerBase — COT-Added Properties & Methods

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Entities/Player/PlayerBase.c`

### Added Properties

```c
// God mode
bool m_COT_GodMode;
bool m_COT_GodMode_Preference;

// Invisibility (enum JMInvisibilityType: None=0, DisableSimulation=1, Interactive=2)
int m_JMIsInvisible;
int m_JMIsInvisibleRemoteSynch;
int m_COT_Invisibility_Preference;
Timer m_COT_InvisibilityUpdateTimer;

// Freeze
bool m_JMIsFrozen;
bool m_JMIsFrozenRemoteSynch;

// Teleport-back
vector m_JMLastPosition;
bool m_JMHasLastPosition;

// Flags
bool m_JMHasUnlimitedAmmo;
bool m_JMHasUnlimitedStamina;
bool m_JMHasCustomScale;
float m_JMScaleValue;
bool m_JMHasAdminNVG;
bool m_JMHasAdminNVGRemoteSynch;
bool m_COT_ReceiveDamageDealt;
bool m_COT_CannotBeTargetedByAI;
bool m_COT_RemoveCollision;
bool m_COT_IsBeingKicked;
bool m_COT_IsLeavingFreeCam;

// Bitmask syncing
map<int, bool> m_COT_PlayerVars;
int m_COT_PlayerVarsBitmask;
```

### COT Methods on PlayerBase

```c
// God mode — simply toggles SetAllowDamage()
player.COTSetGodMode( bool mode, bool preference = true );
bool COTHasGodMode();

// Freeze — disables input controller + simulation
player.COTSetFreeze( bool mode );
bool COTIsFrozen();

// Invisibility — cycles None/DisableSimulation/Interactive
// Also automatically sets RemoveCollision + CannotBeTargetedByAI
player.COTSetInvisibility( int mode, bool preference = true );
int COTIsInvisible();

// Ammo/Stamina
player.COTSetUnlimitedAmmo( bool mode );
player.COTSetUnlimitedStamina( bool mode );

// Admin NVG
player.COTSetAdminNVG( bool mode );

// Collision/AI
player.COTSetRemoveCollision( bool mode, bool preference = true );
player.COTSetCannotBeTargetedByAI( bool mode, bool preference = true );

// Teleport memory
player.SetLastPosition();       // saves current pos to m_JMLastPosition
vector GetLastPosition();

// Scale
player.COTSetScale( float scale );

// Kick state
player.COTSetIsBeingKicked( bool v );
bool COTIsBeingKicked();

// Disease removal
player.COTRemoveAllDiseases();
```

All state changes call `COT_SynchPlayerVars()` which updates `m_COT_PlayerVarsBitmask` and calls `SetSynchDirty()` for network sync.

## JMPlayerModule — Server RPC Pattern (Exec_*)

**Rule:** Every public module method follows: `Public() → if(host) Exec_() else SendRPC()`. Every `RPC_*` method checks permissions then calls `Exec_*`.

### Key Exec_ Methods

**Exec_SetGodMode:**
```c
private void Exec_SetGodMode( bool value, array<string> guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
{
    array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers( guids );
    foreach ( JMPlayerInstance pi : players )
    {
        PlayerBase player = PlayerBase.Cast( pi.PlayerObject );
        if ( !player ) continue;
        player.COTSetGodMode( value );
    }
}
```

**Exec_TeleportTo:**
```c
player.SetLastPosition();         // saves position for TeleportPrevious
player.SetWorldPosition( position );
GetCommunityOnlineToolsBase().Log( ident, "Teleported ... to " + position );
SendWebhook( "Teleport", instance, "..." );
pi.Update();
```

**Exec_Heal:**
```c
player.GetBleedingManagerServer().RemoveAllSources();
CommunityOnlineToolsBase.HealEntityRecursive( player, includeAttachments, includeCargo );
player.SetBrokenLegs( eBrokenLegs.NO_BROKEN_LEGS );
player.COTRemoveAllDiseases();
player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
player.GetStatWater().Set( player.GetStatWater().GetMax() );
// Temporarily re-enables damage if godmode is on to allow healing
```

**Exec_Kick (deferred 500ms):**
```c
// Sends kick message first, then 500ms later:
player.COTSetIsBeingKicked( true );
g_Game.SendLogoutTime( player, 0 );
missionServer.PlayerDisconnected( player, identity, id );
// Then syncs player list 1500ms after kick
```

**Exec_Ban:**
- Cannot ban admins (checks if target has `"COT"` permission)
- Calls `JMBanModule.Ban(guid, name, message, duration, adminID, adminName)`
- Same deferred kick as Exec_Kick
- `duration = -1` → permanent ban; else absolute Unix timestamp expiry

### All Player Module Permissions

```
Admin.Player.Heal / .Heal.Attachments / .Heal.Cargo
Admin.Player.Godmode
Admin.Player.Freeze
Admin.Player.Invisibility
Admin.Player.UnlimitedAmmo / .UnlimitedStamina
Admin.Player.Spectate
Admin.Player.Strip / .ClearCargo / .Dry / .StopBleeding / .BrokenLegs
Admin.Player.ReceiveDamageDealt / .CannotBeTargetedByAI / .RemoveCollision
Admin.Player.Kick / .Ban / .Message
Admin.Player.AccessInventory
Admin.Player.AdminNVG
Admin.Player.Teleport.Position / .SenderTo / .Previous
Admin.Player.Permissions / .Roles
Admin.Player.Set.Health / .Shock / .Blood / .Energy / .Water / .Stamina / .HeatBuffer / .BloodyHands
Admin.Transport.Repair
```

### Keyboard Shortcut Handler Pattern

All `InputToggle*` methods follow:
```c
void InputToggleGodMode( UAInput input )
{
    if ( !input.LocalPress() ) return;

    JMPlayerInstance instance;
    if ( !GetPermissionsManager().HasPermission( "Admin.Player.GodMode", instance ) ) return;

    if ( !GetCommunityOnlineToolsBase().IsActive() )
    {
        ShowInactiveNotification( "STR_COT_INPUT_GODMODE" );
        return;
    }

    bool value = !instance.PlayerObject.COTHasGodMode();  // toggle current state
    array<string> guids = { instance.GetGUID() };         // act on self
    SetGodMode( value, guids );
}
```

Invisibility cycles through `JMInvisibilityType.None → DisableSimulation → Interactive → None`.

## Static Utility Functions

### 3_Game/CommunityOnlineTools/StaticFunctions.c

```c
// Math/string
string FormatFloat( float value, int decimals )
float ToFloat( string text, bool onlyPositive )
string VectorToString( vector vec, int decimals )

// Config
array<string> GetChildrenFromBaseClass( string className, string baseClass )

// Key state
bool SHIFT()   // Shift held
bool CTRL()    // Ctrl held
bool ALT()     // Alt held

// Raycasts
array<Object> GetObjectsAt( vector from, vector to, ... )
Object GetPointerObject( float distance )
Object GetCursorObject( float distance )

// Zombie classes — returns 180+ class name strings
array<string> WorkingZombieClasses()
```

### 4_World/CommunityOnlineTools/Classes/StaticFunctions.c

```c
// Player helpers
PlayerBase GetPlayerObjectByIdentity( PlayerIdentity ident )
PlayerBase GetPlayer()
void SpawnInfected( vector pos )

// Notifications (see advanced.md)
COTCreateNotification( ident, message, icon, time )
COTCreateLocalAdminNotification( message, icon, time )

// Position helpers
vector GetCurrentPosition()              // player or camera pos
vector GetCursorPos( Object ignore )     // crosshair world pos
vector GetPointerPos( float dist, Object ignore )

// Physics
bool COT_PerformRayCast( vector start, vector end, Object ignore, out bool hit )
bool COT_SurfaceIsWater( vector position )

// Messages
void Message( PlayerBase player, string txt, string style )
EntityAI GetWeaponInHands()
```

## COT Preprocessor Defines (config.cpp)

Active defines in `JM/COT/Scripts/config.cpp`:
```cpp
"JM_COT"                          // always on
"JM_COT_VEHICLE_ONSPAWNVEHICLE"   // vehicle spawn hooks
"COT_BUGFIX_REF"                  // reference handling fixes
"COT_BUGFIX_REF_UIACTIONS"        // UIAction reference fixes
"COT_UIACTIONS_SETWIDTH"          // width setter support
"COT_REFRESHSTATS_NEW"            // new stats refresh path
"JM_COT_VEHICLEMANAGER"           // vehicle manager
"JM_COT_INVISIBILITY"             // invisibility system
"JM_COT_EXPTRANSITION_1"          // Expansion mod transition
"JM_COT_DEBUGMONITOR_REMOVED"     // debug monitor removed
"JM_COT_ENABLE_INDIVIDUAL_PERMS"  // per-player permissions
"COT_ANYSPECTATE"                 // spectating
// Disabled (commented out):
// "JM_COT_LOGGING"
// "JM_COT_ESP_DEBUG"
// "JM_COT_WEBHOOK_DEBUG"
```
