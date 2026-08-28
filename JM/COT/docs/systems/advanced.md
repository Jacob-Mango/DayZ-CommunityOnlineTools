---
name: Advanced Systems
description: JMScriptInvokers, COT active state, notifications, admin logging, JSON persistence, chat commands, ESP, loadouts, object spawner, weather Send/Exec pattern, MissionServer overrides, CF framework, camera system, teleport persistence, vehicle enumeration, MissionGameplay, Expansion integration
type: project
---

## JMScriptInvokers — Decoupled Event Bus

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/JMScriptInvokers.c`

```c
static ref ScriptInvoker COT_ON_OPEN;          // bool state — COT active toggled
static ref ScriptInvoker MENU_PLAYER_BUTTON;   // string guid, bool check
static ref ScriptInvoker MENU_PLAYER_CHECKBOX; // string guid, bool checked
static ref ScriptInvoker ADD_OBJECT;
static ref ScriptInvoker REMOVE_OBJECT;
static ref ScriptInvoker ON_DELETE_ALL;
static ref ScriptInvoker ESP_VIEWTYPE_CHANGED;

JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
JMScriptInvokers.COT_ON_OPEN.Invoke( true );
```

## COT Active State

`GetCommunityOnlineToolsBase().IsActive()` — must check before any admin action triggered by input.

```c
if ( !GetCommunityOnlineToolsBase().IsActive() )
{
    ShowInactiveNotification( "STR_COT_INPUT_TELEPORT_CROSSHAIR" );
    return;
}
```

Server tracks per-GUID in `m_ActiveGUIDs`. `SetActive(bool)` fires `JMScriptInvokers.COT_ON_OPEN`.

## Notification System

```c
COTCreateLocalAdminNotification( new StringLocaliser( "STR_KEY" ) );
COTCreateNotification( senderIdentity, new StringLocaliser( "STR_KEY" ) );
ShowInactiveNotification( "STR_COT_REASON_KEY" );
```

Icon: `"set:ccgui_enforce image:HudBuild"`. Title always `STR_COT_NOTIFICATION_TITLE_ADMIN`.

## Admin Logging

```c
Log( instance, "Banned player X" );
Log( senderIdentity, "Did action" );
SendWebhook( "ActionType", "message" );
SendWebhook( "ActionType", instance, "message with player info" );
```

Log file: `$profile:CommunityOnlineTools\Logs\cot-YYYY-MM-DD-hh-mm-ss.log`

## JSON Persistence Pattern

```c
class JMMyStore { ref array<ref JMMyRecord> Records = new array<ref JMMyRecord>(); }
class JMMyRecord { string SteamID; string Name; int Timestamp; }

JsonFileLoader<JMMyStore>.JsonLoadFile( "path/file.json", store );
JsonFileLoader<JMMyStore>.JsonSaveFile( "path/file.json", store );
```

## Chat Command System

Override in module:
```c
override array<string> GetCommandNames() { return new array<string>( { "mymodule" } ); }
override void GetSubCommands( inout array<ref JMCommand> commands )
{
    AddSubCommand( commands, "action", "Command_Action", "Admin.MyModule.Action" );
}
void Command_Action( JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance )
{
    string arg1;
    if ( !params.Next( arg1 ) ) return;
}
```

`JMCommandModule` dispatch: `chat input → tokens[0]=command → tokens[1]=subcommand → args → Execute(sender, args)`

## ESP System

- `JMESPCanvas` — 2D overlay, world→screen projection
- `JMESPSkeleton` — stick-figure on players (color = health level)
- 40+ `JMESPViewType` subclasses, each with `IsValid(Object, out JMESPMeta)` polymorphic check
- Colors: Player=Cyan `ARGB(255,58,220,206)`, Infected=Yellow, Car=Magenta, Item=Light blue
- Permissions follow: `ESP.View`, `ESP.View.<TypeName>`, `ESP.Object.*`
- `JMScriptInvokers.ESP_VIEWTYPE_CHANGED` fired when checkbox toggled in form

## Loadout System

Save: client selects objects → RPC → server walks inventory tree → `JsonFileLoader<JMLoadout>.JsonSaveFile()`
Load: server sends `JMLoadoutMeta` on spawn → client caches → admin picks → server spawns recursively
Spawn modes: `COT_LoadoutSpawnMode.CURSOR`, `TARGET`, `PLAYER`

## Object Spawner

`GetObjectAtCursor(ignorePlayers, distance)` — raycast from camera
Spawn flags: `ECE_IN_INVENTORY`, `ECE_PLACE_ON_SURFACE`, `ECE_INITAI`
Type filters: food/vehicles/firearms/ammo/clothing/containers/items/buildings/AI
Restricted names: `"placing"`, `"debug"`, `"bldr_"`, `"land_"`, `"staticobj_"`

## Weather Module — Send/Exec Pattern

```c
void SetOvercast( float forecast, float time, float minDuration )
{
    if ( g_Game.IsServer() ) Exec_SetOvercast( wBase, NULL );
    else                     Send_SetOvercast( wBase );
}
// Exec_ delegates to wBase.Apply() which calls vanilla Weather API
// RPC_* checks permission then calls Exec_*
```

Each weather property follows this Send/Exec split. Use it when a module has many independent settings.

## MissionServer Overrides

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/MissionServer.c`

- Constructor: `g_cotBase = new CommunityOnlineTools` (singleton created here)
- `OnMissionStart()` → `g_cotBase.OnStart()`
- `OnMissionLoaded()` → `GetPermissionsManager().SetMissionLoaded()`
- `OnUpdate()` → `g_cotBase.OnUpdate()` + `m_JM_PlayerModule.UpdateSpectatorPositions()` every 1s
- `ShouldPlayerBeKilled(player)` → skips if `player.COTIsBeingKicked()`
- `OnEvent(ClientNewEventTypeID)` → `IsCOTBanned(identity)` before allowing entry

Ban check uses `CF_Date.Now(true).GetTimestamp()` for expiry. Auto-deletes expired ban file.

## MissionGameplay (Client) Overrides

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/MissionGameplay.c`

- `OnInit()` — initializes ESP canvas widget
- `OnMissionStart()` — `GetCommunityOnlineTools().OnStart()` + offline player setup (spawn with gear, register in permissions)
- `OnUpdate()` — relays to COT, handles simulation-disabled invisibility, manual HUD updates
- `OnMissionFinish()` — destroys windows, COT cleanup
- `ResetGUI()` — blocked when `m_COT_TempDisableOnSelectPlayer` set (freecam crash workaround)
- Menu/Pause overrides — blocked if COT active or window open

## CF Framework Integration

| CF Class | Used for |
|---|---|
| `CF_Permission_PlayerBase` | Base for JMPlayerInstance (modded) |
| `CF_Module` / `CF_Modules<T>.Get()` | Module registry |
| `CF_InputBindings` | Skip input when EditBox has focus |
| `CF_Windows` | Window focus for COT_MENU/ESP_CONTAINER |
| `CF_Date` | Ban expiry timestamp comparison |
| `CF_Trace` | Debug logging (under `JM_COT_DIAG_LOGGING`) |

## Camera System

- `JMCameraBase` — abstract, FOV lerp, DOF raycast, `SendUpdateAccumalator` at 0.5s
- `JMCinematicCamera` — free camera with `m_Positions[]`/`m_Times[]`/`m_IsSmooth[]` path
- `JMSpectatorCamera` — 3rd-person, 200-pos dolly path, bone tracking, 3 modes
- DOF: `PPEffects.OverrideDOF(true, focusDist, focalLen, focalNear, blur, offset)`
- Constants: `CAMERA_SMOOTH=0.8`, `CAMERA_VELDRAG=0.9`, `CAMERA_MSENS=35.0`

## Teleport Module

- Locations auto-populated from `CfgWorlds [MapName] Names` on first run
- File per world: `FILE_TELEPORT + worldName + EXT_TELEPORT`
- Cursor teleport max: 1000m (notification if exceeded)
- Types: "AF…" → "Airfield", "MB…" → "Military Base", sorted alphabetically, "ALL" at index 0

## Vehicle Enumeration

`UpdateVehiclesMetaData()` uses **COT-maintained linked lists**:
```c
// Vanilla vehicles — COT modded CarScript/BoatScript to maintain these:
CarScript.s_JM_AllCars      // doubly-linked list of all cars
BoatScript.s_JM_AllBoats    // doubly-linked list of all boats

// Expansion vehicles (conditional):
#ifdef EXPANSIONMODVEHICLE
ExpansionVehicleBase.GetAll()
ExpansionVehicleCover.s_JM_AllCovers
#endif
```

`JMVehicleMetaData` captures: NetworkID, PersistentID, ClassName, Position, Orientation, VehicleType bitmask (`JMVT_CAR=2`, `JMVT_BOAT=4`, `JMVT_HELICOPTER=8`, `JMVT_PLANE=32`, `JMVT_BIKE=512`), destruction flags, owner info (Expansion only).

## Expansion Mod Integration Pattern

All Expansion references guarded:
```c
#ifdef EXPANSIONMODVEHICLE
// Expansion-specific code here
#endif

#ifdef DZ_Expansion_Market
// Market-specific code here
#endif
```

Graceful degradation — COT works without Expansion installed.

## JMStatics — Utility Class

File: `JM/COT/Scripts/3_Game/CommunityOnlineTools/JMStatics.c`

```c
JMStatics.SortStringArray( names );
JMStatics.SortStringArrayKVPair( keys, values );
JMStatics.StrCmp( a, b );
JMStatics.StrCaseCmp( a, b );
JMStatics.CamelCaseToWords( s );   // "ObjectName" → "Object Name"
JMStatics.StringToEnumEx( s );

// Static widget refs (set at startup):
JMStatics.ESP_CONTAINER
JMStatics.COT_MENU
JMStatics.WINDOWS_CONTAINER
```

## COT_String — Search

File: `JM/COT/Scripts/1_Core/CommunityOnlineTools/COT_String.c`

```c
COT_String.KeywordSearch_Prepare( searchText )   // handles OR operators
COT_String.KeywordSearchImpl( haystack, keywords, requireAll )
COT_String.KeywordSearchImplEx( haystack, keywords )  // with autocomplete
```

## UIActionScroller — Critical Timing

`UpdateScroller()` defers 34ms via `CallLater`. `m_IsUpdating` flag batches multiple calls into one. **Always call after adding/removing/resizing content.** No force-immediate option.

## COT_BuildParts — Base Building Spawn

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/BaseBuilding/Construction.c`

Added to vanilla `Construction`:
```c
COT_GetParts( out parts, checkMaterials )   // enumerate all parts with build state
COT_BuildParts( parts, player, checkMaterials ) // batch-build, handles dependencies
COT_BuildRequiredParts( partName, player, checkMaterials ) // recursive parent-first
COT_DismantleParts() / COT_RepairPart( partName )
```

## JMESPViewType — Polymorphic Filtering

Base: `typename MetaType`, `int Colour` (ARGB), `string Permission`, `string Localisation`, `bool IsValid(Object, out JMESPMeta)` (pure virtual).

| Type | Color |
|------|-------|
| Player | Cyan `ARGB(255,58,220,206)` |
| Infected | Yellow `ARGB(255,215,219,0)` |
| Car | Magenta `ARGB(255,255,113,237)` |
| Item | Light blue `ARGB(255,70,180,255)` |

## Permission Serialization Format

Dot-notation strings with state suffix (0=Inherit, 1=Disallow, 2=Allow):
```
"Admin 2"
"Admin.Player.Permissions 1"
"Weather.Preset 0"
```

Transmitted via `JMPermission.OnReceive(ctx)` in `RPC_SetPermissions`.
