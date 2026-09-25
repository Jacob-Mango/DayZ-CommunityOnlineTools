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

Player toggles (god mode, freeze, invisibility, unlimited ammo/stamina, admin NVG, ...) and stat sliders do **not** have a
hand-written `Send / Exec / RPC` trio each. They are registered entries (`JMPlayerToggle`, `JMPlayerStat`) declared in
`RegisterToggles()` / `RegisterStats()` and served by one code path (`SetToggle`, `Exec_Toggle`, `RPC_Toggle`, `SetStat`,
`Exec_SetStat`, `RPC_SetStat`). See [module-creation.md](module-creation.md#repeated-actions-register-them-do-not-copy-them).
`SetGodMode()` and the other named wrappers survive as one-liners that call `SetToggleById( JMPlayerToggle.GODMODE, ... )`.

### Key Exec_ Methods

**Exec_Toggle** (one method for every registered toggle):
```c
protected void Exec_Toggle( JMPlayerToggle toggle, int value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
{
    array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
    // for each player with a PlayerObject:
    //   toggle.Apply( player, value );     // e.g. JMPlayerToggleGodMode -> player.COTSetGodMode( value )
    //   ProcessToggle( ... );              // log + webhook
    //   toggle.OnApplied( players[i] );
    // then ShowToggleNotification( ... )
}
```

**Exec_TeleportTo:**
```c
player.SetLastPosition();         // saves position for TeleportPrevious
player.SetWorldPosition( position );
GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=...] to " + position );
SendWebhookColored( "Teleport", instance, "...", JMConstants.WEBHOOK_COLOR_TELEPORT );
pi.Update();
```
(The RPC handler first rejects a position that fails `CommunityOnlineToolsBase.IsValidWorldPosition()`.)

**Exec_Heal:**
```c
// Temporarily re-enables damage if godmode is on, so the heal can apply
player.GetBleedingManagerServer().RemoveAllSources();
CommunityOnlineToolsBase.HealEntityRecursive( player, includeAttachments, includeCargo );   // flags = Heal.Attachments / Heal.Cargo permissions of the sender
player.SetBrokenLegs( eBrokenLegs.NO_BROKEN_LEGS );
player.COTRemoveAllDiseases();
player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
player.GetStatWater().Set( player.GetStatWater().GetMax() );
```

**Exec_Kick (deferred 500ms):**
```c
// Sends the kick message first (SendKickMessage), then 500ms later Exec_Kick_Single:
player.COTSetIsBeingKicked( true );
g_Game.SendLogoutTime( player, 0 );
missionServer.PlayerDisconnected( player, identity, id );
// Player list is re-synced 1500ms after the kick
```

**Exec_Ban:**
- Skips the sender, and refuses to ban other staff (a target that holds the `"COT"` permission node) - the sender gets `STR_COT_PLAYER_MODULE_CANT_BAN_ADMINS`
- Sends the ban message (`SendBanMessage`), then runs `Exec_Ban_Single` 500 ms later: the same deferred disconnect as `Exec_Kick_Single`, logged and webhooked as `Ban`
- **It does not write a ban record.** `JMPlayerModule` never calls `JMBanModule.Ban()`; only the Ban Manager and Anti-Cheat forms do (see [../modules/player-flows.md](../modules/player-flows.md)). `duration` is accepted (`-1` = permanent) but only travels as far as the RPC

### All Player Module Permissions

The authoritative list is the `PERM_PLAYER_*` block of `JMConstants.c` (values are the keys saved in role files).

```
Admin.Player.View / .Read / .Stats / .Statistics.View / .InjectedPanel
Admin.Player.Heal / .Heal.Attachments / .Heal.Cargo
Admin.Player.GodMode
Admin.Player.Freeze
Admin.Player.Invisibility
Admin.Player.Ragdoll
Admin.Player.UnlimitedAmmo / .UnlimitedStamina
Admin.Player.Spectate
Admin.Player.Strip / .ClearCargo / .Dry / .StopBleeding / .BrokenLegs / .Vomit / .Scale
Admin.Player.Bleed.Add / .Bleed.Stop / .Disease.Add / .Disease.Remove
Admin.Player.ReceiveDamageDealt / .CannotBeTargetedByAI / .RemoveCollision
Admin.Player.Kick / .Ban / .Message / .Notif
Admin.Player.Inventory / .AccessInventory / .Inventory.Delete / .Inventory.Modify / .Inventory.Repair / .Inventory.Take
Admin.Player.AdminNVG
Admin.Player.Teleport.Position / .Position.Cursor / .Position.Cursor.NoLog / .Position.View / .SenderTo / .Previous
Admin.Player.Teleport.Location / .Location.Create / .Location.Edit / .Location.Remove
Admin.Player.Permissions / .Roles
Admin.Player.Set.Health / .Shock / .Blood / .Energy / .Water / .Stamina / .HeatBuffer / .BloodyHands
Admin.Roles.View / .Create / .Delete / .Permissions / .Update
Admin.Transport.Repair
```

### Keyboard Shortcut Handler Pattern

The `InputToggle*` methods are one-liners over a shared `InputToggle( id, input )` (`InputFreezePlayer`, `InputToggleGodMode`,
`InputToggleInvisibility`, `InputToggleCannotBeTargetedByAI`, `InputToggleUnlimitedStamina`, `InputToggleUnlimitedAmmo`,
`InputToggleAdminNV`):
```c
void InputToggleGodMode( UAInput input ) { InputToggle( JMPlayerToggle.GODMODE, input ); }

protected void InputToggle( string id, UAInput input )
{
    if ( !input.LocalPress() ) return;

    JMPlayerToggle toggle = GetToggle( id );
    if ( !toggle ) return;

    JMPlayerInstance instance;
    if ( !JMPermissions.Has( toggle.GetPermission(), instance ) ) return;

    if ( !GetCommunityOnlineToolsBase().IsActive() )
    {
        ShowInactiveNotification( toggle.GetInputHint() );
        return;
    }

    bool value = !toggle.ReadEntity( instance.PlayerObject, instance );   // toggle current state
    array< string > guids = { instance.GetGUID() };                       // act on self
    SetToggle( toggle, value, guids );
}
```

Invisibility cycles through `JMInvisibilityType.None → DisableSimulation → Interactive → None`.

## Static Utility Functions

### 3_Game/CommunityOnlineTools/StaticFunctions.c

```c
// Math/string
string FormatFloat( float value, int decimals )
float ToFloat( string text, bool onlyPositive = false )
string VectorToString( vector vec, int decimals = -1 )
bool CheckStringType( string str, int type )
bool ArrayContains( array< string > arr, string match )

// Config
TStringArray GetChildrenFromBaseClass( string strConfigName, string strBaseClass )
string GetRandomChildFromBaseClass( string strConfigName, string strBaseClass, int minScope = -1, string strIgnoreClass = "" )
TVectorArray GetSpawnPoints()

// Key state
bool SHIFT()   // Shift held
bool CTRL()    // Ctrl held
bool ALT()     // Alt held
bool WINKEY()  // Windows key held

// Raycasts / world
set< Object > GetObjectsAt( vector from, vector to, Object ignore = NULL, float radius = 0.5, Object with = NULL )
Object GetPointerObject( float distance = 100.0, Object ignore = NULL, float radius = 0.5, Object with = NULL )
Object GetCursorObject( float distance = 100.0, Object ignore = NULL, float radius = 0.5, Object with = NULL )
vector SnapToGround( vector pos, bool aboveWater = true )

// Files
array< string > FindFilesInLocation( string folder )
void DeleteFiles( string folder, array< string > files )

// Zombie classes — a hardcoded list of working infected class names
TStringArray WorkingZombieClasses()
```

### 4_World/CommunityOnlineTools/Classes/StaticFunctions.c

```c
// Player helpers
PlayerBase GetPlayerObjectByIdentity( PlayerIdentity ident )
PlayerBase GetPlayer()
ZombieBase SpawnInfected( vector pos )

// Notifications (see advanced.md) - message is a StringLocaliser
COTCreateNotification( PlayerIdentity ident, StringLocaliser message, string icon = "set:ccgui_enforce image:HudBuild", float time = 1.5 )
COTCreateLocalAdminNotification( StringLocaliser message, string icon = ..., float time = 1.5 )

// Position helpers
vector GetCurrentPosition()                                  // player or camera pos
vector GetCursorPos( Object ignore = NULL )                  // crosshair world pos
vector GetPointerPos( float distance = 100.0, Object ignore = NULL )

// Physics
vector COT_PerformRayCast( vector rayStart, vector rayEnd, Object ignore, out bool hit = false )
bool COT_CameraRaycast( float distance, out vector contactPos, Object ignore = NULL )   // the one camera pick every module uses
bool COT_SurfaceIsWater( vector position )

// Messages / weapons
void Message( PlayerBase player, string txt, string style = "colorImportant" )
Weapon GetWeaponInHands()
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
