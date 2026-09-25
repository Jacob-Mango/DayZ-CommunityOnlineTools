---
name: Advanced Systems
description: JMScriptInvokers, COT active state, notifications, admin logging, JSON persistence, chat commands, ESP, loadouts, object spawner, weather Send/Exec pattern, MissionServer overrides, CF framework, camera system, teleport persistence, vehicle enumeration, MissionGameplay, Expansion integration
type: project
---

## JMScriptInvokers — Decoupled Event Bus

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/JMScriptInvokers.c`

```c
static ref ScriptInvoker COT_ON_OPEN;                  // bool m_IsOpen — sidebar opened/closed
static ref ScriptInvoker COT_ON_CLOSE;
static ref ScriptInvoker COT_ON_SELECTED_PLAYER_CHANGE;
static ref ScriptInvoker COT_ON_PERMISSIONS_UPDATED;
static ref ScriptInvoker COT_ON_THEME_CHANGED;
static ref ScriptInvoker ON_PLAYER_CONTEXT_MENU;
static ref ScriptInvoker MENU_PLAYER_BUTTON;           // string guid, bool check
static ref ScriptInvoker MENU_PLAYER_CHECKBOX;         // string guid, bool checked
static ref ScriptInvoker ADD_OBJECT;
static ref ScriptInvoker REMOVE_OBJECT;
static ref ScriptInvoker ON_DELETE_ALL;
static ref ScriptInvoker ESP_VIEWTYPE_CHANGED;

JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
JMScriptInvokers.COT_ON_OPEN.Invoke( true );
```

`COT_ON_OPEN` is invoked from `CommunityOnlineToolsBase.SetOpen()` (and its callers) with the sidebar's open state - not with
the "active" state below.

## COT Active State

`GetCommunityOnlineToolsBase().IsActive()` — must check before any admin action triggered by input.

```c
if ( !GetCommunityOnlineToolsBase().IsActive() )
{
    ShowInactiveNotification( "STR_COT_INPUT_TELEPORT_CROSSHAIR" );
    return;
}
```

Server tracks per-GUID in `m_ActiveGUIDs` (`IsActive( identity )`); the client holds one local flag. Two separate flags exist:
`IsActive()` is "the admin switched COT on" (`SetActive( bool )`, synced with `JMCOTRPC.Active`), `IsOpen()` is "the sidebar is
showing" (`SetOpen`, debounced to one accepted call per 250 ms). Opening is refused while inactive; `COT_ON_OPEN` follows the open flag.

## Notification System

```c
COTCreateLocalAdminNotification( new StringLocaliser( "STR_KEY" ) );
COTCreateNotification( senderIdentity, new StringLocaliser( "STR_KEY" ) );
ShowInactiveNotification( "STR_COT_REASON_KEY" );
```

Icon: `"set:ccgui_enforce image:HudBuild"`. Title always `STR_COT_NOTIFICATION_TITLE_ADMIN`.

## Admin Logging

```c
GetCommunityOnlineToolsBase().Log( instance, "Banned player X" );
GetCommunityOnlineToolsBase().Log( senderIdentity, "Did action" );
SendWebhook( "ActionType", "message" );                                       // JMModuleBase methods
SendWebhook( "ActionType", instance, "message with player info" );
SendWebhookColored( "ActionType", instance, "message", JMConstants.WEBHOOK_COLOR_WARNING );
```

A `JMModuleAction` does both for you (`Describe()` is the log line, `DescribeWebhook()` the webhook). A webhook type must be
declared (`info.AddWebhookType`, or an action's `GetWebhookType()`) before an admin can switch it on for a channel.

Log file: `$profile:CommunityOnlineTools\Logs\cot-YYYY-MM-DD-hh-mm-ss.log`

## JSON Persistence Pattern

```c
class JMMyStore { ref array<ref JMMyRecord> Records = new array<ref JMMyRecord>(); }
class JMMyRecord { string SteamID; string Name; int Timestamp; }

JMJsonFile<JMMyStore>.Load( "path/file.json", store );     // false (and logged) when the file does not parse
JMJsonFile<JMMyStore>.Save( "path/file.json", store );     // creates every missing folder first
```

Use `JMJsonFile<T>`, not `JsonFileLoader<T>` directly (see [module-creation.md](module-creation.md#optional-settings-persistence)).
A few older files still call `JsonFileLoader<T>.JsonLoadFile/JsonSaveFile` (`JMSpawnerConfig`, `JMLoadoutSettings`).

## Chat Command System

Override in module:
```c
override array<string> GetCommandNames() { return new array<string>( { "mymodule" } ); }
override void GetSubCommands( inout array<ref JMCommand> commands )
{
    AddSubCommand( commands, "action", "Command_Action", JMConstants.PERM_MYMODULE_ACTION );
}
void Command_Action( JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance )
{
    string arg1;
    if ( !params.Next( arg1 ) ) return;
}
```

`JMCommandModule` dispatch (server, `JMCommandModuleRPC.PerformCommand`): `chat input → tokens[0]=command → tokens[1]=subcommand → args → JMCommand.Execute(sender, args)`.
`Execute` re-checks the subcommand's permission with `JMPermissions.HasRPC` and calls the named method through
`g_Script.CallFunctionParams`. `JMCommandConstructor` collects the commands from every module's `GetCommandNames()` /
`GetSubCommands()`; the Object Spawner's `spawn` is the built-in example.

## ESP System

- `JMESPCanvas` / `JMESPSkeleton` (both declared in `JMESPModule.c`) — 2D overlay, world→screen projection; stick-figure on players (color = health level)
- 34 `JMESPViewType` subclasses (`JMESPViewType.c`), each with `IsValid(Object, out JMESPMeta)` polymorphic check
- Colors: Player=Cyan `ARGB(255,58,220,206)`, Infected=Yellow, Car=Magenta, Item=Light blue
- Permissions follow: `ESP.View`, `ESP.View.<ViewType.Permission>` (registered from each view type), `ESP.Object.*`
- Tabs: `Tabs/Objects`, `Tabs/Filters`, `Tabs/Settings`; per-object actions go through `JMESPObjectAction` (see [rpc.md](rpc.md))
- `JMScriptInvokers.ESP_VIEWTYPE_CHANGED` fired when checkbox toggled in form

## Loadout System

Save: client selects objects → RPC → server walks inventory tree → `JMLoadoutSettings` writes `JMLoadout` JSON (`JsonFileLoader<JMLoadout>`) under `DIR_LOADOUTS`
Load: server sends `JMLoadoutMeta` on spawn → client caches → admin picks → server spawns recursively
Spawn modes: `COT_LoadoutSpawnMode.CURSOR`, `TARGET`, `PLAYER` (enum lives in `modules/Object/Enums.c`)
Actions are `JMModuleAction` subclasses in `JMLoadoutActions.c`. Compensations (`JMCompensationsModule`) is a sibling module for compensation item sets; Expansion prefabs are read through `JMExpansionLoadoutFile`.

## Object Spawner

`GetObjectAtCursor(ignorePlayers, distance)` — raycast from camera
Spawn flags: `ECE_IN_INVENTORY`, `ECE_PLACE_ON_SURFACE`, `ECE_INITAI`
Type filters: food/vehicles/firearms/ammo/clothing/containers/items/buildings/AI
Blocked / restricted class names come from `JMSpawnerConfig` (`$profile:CommunityOnlineTools\SpawnerConfig.json`, created
with defaults on first run and editable without recompiling): `UnfinishedItems` (exact lowercase class names that crash or
are unfinished), `RestrictedPatterns` (substring: `"placing"`, `"debug"`) and `RestrictedStartPatterns` (prefix: `"bldr_"`,
`"land_"`, `"staticobj_"`)

## Weather Module — Payload Registry

Each manual weather setting is a `JMWeatherBase` payload class (`JMWeatherPreset.c`: `JMWeatherStorm`, `JMWeatherSandstorm`,
`JMWeatherFog`, `JMWeatherRain`, `JMWeatherOvercast`, `JMWeatherDate`, ...). A payload names its RPC id (`GetRPC()`), permission
(`GetPermission()`), webhook wording, and how it applies (`Apply()` calls the vanilla Weather API). One code path serves all of them:

```c
void SetOvercast( float forecast, float time = 0, float minDuration = 0, float forecastHi = 0 )
{
    SubmitPhenomenon( new JMWeatherOvercast(), forecast, time, minDuration, forecastHi );
}
// Submit() -> server: Exec_Payload( payload, NULL ); client: Send_Payload( payload )
// RPC_Payload() reads the payload, checks CanServe( sender, payload.GetPermission() ),
//   optionally pushes it to clients (IsPushedToClients), then Exec_Payload()
// Exec_Payload() -> payload.Apply(), payload.Log(), SendWebhookColored(...)
```

Use this shape when a module has many independent settings. The dynamic weather state machine is documented in
[../modules/weather-dynamic.md](../modules/weather-dynamic.md).

## MissionServer Overrides

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/MissionServer.c`

- Constructor: `g_cotBase = new CommunityOnlineTools` (singleton created here)
- `OnMissionStart()` → `g_cotBase.OnStart()`
- `OnMissionLoaded()` → `GetPermissionsManager().SetMissionLoaded()`
- `OnMissionFinish()` — COT teardown
- `OnUpdate()` → `g_cotBase.OnUpdate()` + `m_JM_PlayerModule.UpdateSpectatorPositions()` every 1s
- `ShouldPlayerBeKilled(player)` → skips if `player.COTIsBeingKicked()`
- `OnEvent(ClientNewEventTypeID)` → `IsCOTBanned(identity)` before allowing entry

Ban check reads `Bans.json` through `JMPlayerBan.Load( guid, steamID )` (static, so it works before `JMBanModule` has loaded) and
uses `CF_Date.Now(true).GetTimestamp()` for expiry. It also calls `JMPlayerBan.DeleteBanFile()` on an expired record
(legacy per-player files; the deprecated helper hands off to `JMBanModule`).

## MissionGameplay (Client) Overrides

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/MissionGameplay.c`

- `OnInit()` — creates the ESP canvas (`JMESPModule.CreateCanvas()`)
- `CreateScriptedMenu( id )` — returns the Preview Lab test menus (`JMPreviewLabMenu` etc., from the Example module)
- `OnMissionStart()` — `GetCommunityOnlineTools().OnStart()` + offline player setup (`OfflineMissionStart()`: spawn with gear, register in permissions)
- `OnUpdate()` — relays to COT, polls the action-history hotkeys (Ctrl+Z / Ctrl+Y) while COT is open, hides the vanilla debug monitor while COT is open, handles simulation-disabled invisibility / manual HUD updates
- `OnMissionFinish()` — `GetCOTWindowManager().DestroyAllWindows()`, COT cleanup
- `ResetGUI()` — blocked when `m_COT_TempDisableOnSelectPlayer` set (freecam crash workaround)
- `ShowInventory()`, `Pause()`, `HandleMapToggleByKeyboardShortcut()` — swallowed while the COT sidebar is open or an unpinned COT window is active

## CF Framework Integration

| CF Class | Used for |
|---|---|
| `CF_Permission_PlayerBase` | Modded to carry the `JMPlayerInstance` getters, only when `CF_MODULE_PERMISSIONS` is defined |
| `JMModuleManager` / `JMModuleConstructorBase` | Module registry and constructor - COT `modded class`es both |
| `CF_Modules<T>.Get()` | Look up a module instance (`CF_Modules<JMESPModule>.Get( espModule )`) |
| `CF_InputBindings` | Skip input when EditBox has focus (`3_Game/.../InputBindings/CF_InputBindings.c`, unless `CF_INPUTBINDINGS_FOCUS_FIX`) |
| `CF_Windows` | Window focus for COT_MENU/ESP_CONTAINER (only when `CF_WINDOWS`) |
| `CF_Date` | Ban expiry timestamp comparison |
| `CF_DoublyLinkedNodes_WeakRef` | Vehicle lists (`CarScript.s_JM_AllCars`, ...) |
| `CF_Trace` | Debug logging (under `JM_COT_DIAG_LOGGING`) |

## Camera System

- `JMCameraBase` (3_Game) — abstract, FOV lerp, DOF raycast, `SendUpdateAccumalator` at 0.5s
- `JMCinematicCamera` (3_Game) — free camera with a travel path of `JMCameraWaypoint`s (position, speed, orientation, easing, per-waypoint exposure/DOF, hold time; Catmull-Rom when there are 4+)
- `JMFreecam` (3_Game) — plain 6-DOF free-roam camera used by the Map Editor (WASD/QE, Shift 5x, Ctrl 0.2x)
- `JMSpectatorCamera` (4_World) — 3rd-person, `DOLLY_CAM_PATH_LIMIT = 200`-position dolly path, bone tracking, modes from `JMCamera3rdPersonMode` (`DOLLY`, `AUTO`, ...)
- DOF: `PPEffects.OverrideDOF(true, focusDist, focalLen, focalNear, blur, offset)` (called from `JMCameraModule`)
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

MotorbikeScript.s_JM_AllBikes // doubly-linked list of all motorbikes

// Expansion vehicles (conditional):
#ifdef EXPANSIONMODVEHICLE
ExpansionVehicleBase.GetAll()
ExpansionVehicleCover.s_JM_AllCovers
#endif
```

`JMVehicleMetaData` (declared in `JMVehiclesModule.c`) captures: NetworkID, PersistentID, ClassName, Position, Orientation, VehicleType bitmask (`JMVT_CAR=2`, `JMVT_BOAT=4`, `JMVT_HELICOPTER=8`, `JMVT_PLANE=32`, `JMVT_BIKE=512`, `JMVT_ALL=558`), destruction flags (`JMDT_EXPLODED=2`, `JMDT_DESTROYED=4`), owner info (Expansion only).

> `JMVT_ALL` is a literal, not `JMVT_CAR | JMVT_BOAT | ...`: Enforce does not fold a global const built from other global consts, it comes out 0. Keep it in step with the bits.

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

Defines in use: `DZ_Expansion_Core`, `EXPANSIONMODCORE`, `EXPANSIONMODVEHICLE`, `EXPANSIONMODBASEBUILDING` (the Territory module),
`EXPANSIONMODMISSIONS`, `EXPANSIONMODAI`, `EXPANSIONMODNAVIGATION`, `DZ_Expansion_Market`, `DZ_Expansion_AI`, `DZ_Expansion_Hardline`,
`DZ_Expansion_Groups`. Expansion-guarded branches cannot currently be build-verified (see [mod-compatibility.md](mod-compatibility.md)).

Graceful degradation — COT works without Expansion installed.

## JMStatics — Utility Class

File: `JM/COT/Scripts/3_Game/CommunityOnlineTools/JMStatics.c`

```c
JMStatics.SortStringArray( names );
JMStatics.SortStringArrayKVPair( keys, values );
JMStatics.StrCmp( a, b );
JMStatics.StrCaseCmp( a, b );
JMStatics.CamelCaseToWords( s );   // "ObjectName" → "Object Name"
JMStatics.StringToEnumEx( enumType, name, out value );

// Static widget refs (set at startup):
JMStatics.ESP_CONTAINER
JMStatics.COT_MENU
JMStatics.WINDOWS_CONTAINER

// Overlay registry (widgets that must stay above windows):
JMStatics.AddOverlay( w ) / RemoveOverlay( w ) / IsOverlay( w ) / RegisterOverlay( w ) / UnregisterOverlay( w )
```

## COT_String / JMSearchMatcher — Search

Files: `JM/COT/Scripts/1_Core/CommunityOnlineTools/COT_String.c`, `JMSearchMatcher.c`

`COT_String` is a `string` subclass; the methods are called on the string:

```c
COT_String needle = searchText;
TStringArray keywords = needle.KeywordSearch_Prepare( requireAll );   // lowercases in place; `OR` / `|` => any-of
haystack.KeywordSearchImpl( search, keywords, requireAll );
haystack.KeywordSearchImplEx( search, keywords, requireAll, closestMatch );   // also yields an autocomplete match
```

List code should use `JMSearchMatcher` instead of carrying needle / keywords / requireAll / closestMatch around:
`new JMSearchMatcher( box.GetText() )`, then `matcher.Matches( row.Name )` per row and `matcher.GetClosestMatch()` for the box preview.

## UIActionScroller — Critical Timing

`UpdateScroller()` defers 34ms via `CallLater`. `m_IsUpdating` flag batches multiple calls into one. **Always call after adding/removing/resizing content.** No force-immediate option.

## COT_BuildParts — Base Building Spawn

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/BaseBuilding/Construction.c`

Added to vanilla `Construction`:
```c
COT_CanBuildPart( partName, checkMaterials ) / COT_CanDismantlePart( partName )
COT_GetParts( out map<string, ref JMConstructionPartData> parts, checkMaterials )   // enumerate all parts with build state
COT_BuildParts( TStringArray parts_name, player, checkMaterials ) // batch-build, handles dependencies
COT_BuildRequiredParts( partName, player, checkMaterials ) // recursive parent-first
COT_DismantleParts( parts_name, player ) / COT_DismantleRequiredParts( partName, player ) / COT_RepairPart( partName )
```

## JMESPViewType — Polymorphic Filtering

Base: `typename MetaType`, `int Colour` (ARGB), `string Permission`, `string Localisation`, `bool HasPermission`, `bool View`,
`bool IsValid(Object, out JMESPMeta)` (base implementation calls `Error( "Not implemented!" )` - always override it).

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

`JMPermission.Serialize( output )` produces these lines (children are prefixed with their parent's dotted path) and
`Deserialize` reads them back as `INHERIT` nodes; role files hold the same lines. Over the wire the tree is written by
`JMPermission.OnSend( ctx )` / read by `OnReceive( ctx )` (`RPC_SetPermissions` uses it).
