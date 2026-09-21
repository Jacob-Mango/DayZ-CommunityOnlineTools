---
name: Project Architecture
description: COT directory layout, DayZ script layer system, key entry points, helper functions, and file structure reference
type: project
---

## DayZ Script Layer System

DayZ loads scripts in order — each layer can override previous ones:
- `1_Core/` — Dependency-free helpers: `COT_String`, `JMSearchMatcher`, `proto/EnMath.c`
- `3_Game/` — Game-wide data: constants, RPC enums, ban structs, player instances, loadout data, cameras, `JMJsonFile`, `JMRPCThrottle`
- `4_World/` — Base classes and shared services: JMModuleBase, JMRenderableModuleBase, JMModuleAction, JMFormBase, JMFormTab, JMWindowBase, sidebar, webhook, permissions, anti-cheat, commands, plus the vanilla-entity overrides
- `5_Mission/` — Concrete implementations: actual modules, forms, the `UIAction*` widgets and `UIActionManager`, the Entity Manager
- `Common/COTDefines.c` — compile-time defines shared by every layer

## Key File Reference

| Path | Purpose |
|------|---------|
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/RPC.c` | All RPC enum definitions with offsets |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/JMConstants.c` | Directory path constants |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/Bans/JMPlayerBan.c` | Ban record struct |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/Bans/JMPlayerBanStore.c` | JSON container for bans |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/PermissionsOld/JMPlayerInstance.c` | Per-player COT state (the active class; `3_Game/.../Permissions/JMPlayerInstance.c` is the `CF_MODULE_PERMISSIONS` variant) |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/PermissionsOld/JMPermissionManager.c` | `GetPermissionsManager()`: roles, permission tree, roster |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Permissions/JMPermissions.c` | Static API module code uses for permission checks |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Module/JMModuleInfo.c` / `JMModuleAction.c` | `DescribeModule()` descriptor and the client -> server action class |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Module/JMModuleBase.c` | Base module (webhook, commands) |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Module/JMRenderableModuleBase.c` | Base for GUI modules |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Module/JMModuleManager.c` | Module registry and dispatcher |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Module/JMModuleConstructor.c` | World-level module registration |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMFormBase.c` | Base form class |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMWindowManager.c` | Window lifecycle manager |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMWindowBase.c` | Individual window frame |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c` | Left sidebar (module button list) |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/COTModule.c` | Sidebar controller, menu state, input |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/CommunityOnlineToolsBase.c` | Base COT class (activation state, logging) |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/Webhook/JMWebhookModule.c` | Discord webhook POST queuing |
| `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/JMSelectedObjects.c` | Player/object selection tracker |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/CommunityOnlineTools.c` | Mission-level RPC dispatcher, COT entry |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/JMModuleConstructor.c` | **Register all modules here** |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionBase.c` | Base GUI action widget |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionManager.c` | Factory for creating widgets |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMBanModule.c` | Ban module + RPC handlers |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMPlayerBanForm.c` | Ban list UI form |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c` | Player actions module |
| `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c` | Player list + controls UI |
| `JM/COT/GUI/layouts/` | All .layout XML UI files |

## Registered Modules (JMModuleConstructor 5_Mission)

```c
modules.Insert( JMPlayerModule );
modules.Insert( JMObjectSpawnerModule );
modules.Insert( JMESPModule );
modules.Insert( JMTeleportModule );
modules.Insert( JMCameraModule );
modules.Insert( JMWeatherModule );
modules.Insert( JMLoadoutModule );
modules.Insert( JMCompensationsModule );
modules.Insert( JMMapModule );
modules.Insert( JMMapEditorModule );
modules.Insert( JMAntiCheatModule );
modules.Insert( JMVehiclesModule );
modules.Insert( JMCommandModule );
modules.Insert( JMBanModule );
modules.Insert( JMRoleManagerModule );
modules.Insert( JMWebhookCOTModule );
modules.Insert( JMLootAnalysisModule );
modules.Insert( JMEventsModule );
modules.Insert( JMServerStatsModule );
modules.Insert( JMActionHistoryModule );

#ifdef DIAG
modules.Insert( JMExampleModule );
modules.Insert( JMItemStatsModule );
#endif

// Namalsk: only when NamEventManager and EVRStorm are both loaded
modules.Insert( JMNamalskEventManagerModule );

#ifdef EXPANSIONMODBASEBUILDING
modules.Insert( JMTerritoryModule );
#endif
```

World-level modules (no sidebar button) are registered in the 4_World `JMModuleConstructor.c` (a
`modded class JMModuleConstructorBase`): `COTModule`, `JMWebhookModule`.

`JMModuleManager` (`GetModuleManager()`) and `JMModuleConstructorBase` come from the Community Framework mod; COT
adds to them with `modded class`. `JMModuleManager.InitModule` calls `module.DeclarePermissions()` and files every
`JMRenderableModuleBase` into `GetCOTModules()`.

Module folders under `modules/`: `ActionHistory`, `AntiCheat`, `BanManager`, `COTMap`, `Camera`, `Compat`,
`Compensations`, `ESP`, `Events`, `Example`, `ItemStats`, `Loadout`, `LootAnalysis`, `MapEditor`, `Namalsk`, `Object`,
`Player`, `RoleManager`, `ServerStats`, `Teleport`, `Territory`, `Vehicles`, `Weather`, `Webhook`. `Compat/` holds
helpers (`JMExpansionLoadoutFile`, `JMItemSetCompat`), not a module. Shared, module-less GUI code lives in
`5_Mission/.../gui/` (the `UIAction*` widgets, `EntityManager/`, the debug monitor).

## Global Singleton / Helper Functions

| Function | Purpose |
|----------|---------|
| `GetModuleManager()` | Access JMModuleManager (all modules) |
| `GetModuleManager().GetModule(JMMyModule)` | Get specific module instance |
| `GetModuleManager().GetCOTModules()` | Get all renderable modules |
| `GetPermissionsManager()` | Access permission/role system |
| `GetCommunityOnlineToolsBase()` | COT main instance |
| `GetCommunityOnlineTools()` | Mission-level COT instance |
| `JM_GetSelected()` | Global JMSelectedObjects (player/object selection) |
| `GetCOTWindowManager()` | Window lifecycle manager |
| `IsMissionHost()` | True on server or offline |
| `IsMissionClient()` | True on client |
| `IsMissionOffline()` | True in singleplayer |

## JMSelectedObjects (Player Selection)

Global singleton for tracking which players/objects the admin has selected:

```c
JM_GetSelected().AddPlayer( guid );
JM_GetSelected().RemovePlayer( guid );
JM_GetSelected().ClearPlayers();
JM_GetSelected().GetPlayers();   // auto-selects self if empty
JM_GetSelected().AddObject( obj );
JM_GetSelected().ClearObjects();
```

Modules subscribe to selection changes via script invokers:
```c
JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
```

## Key Constants (JMConstants.c)

```c
DIR_COT          = "$profile:CommunityOnlineTools\\"
DIR_PF           = "$profile:PermissionsFramework\\"
DIR_PERMISSIONS  = DIR_PF + "Permissions\\"
DIR_ROLES        = DIR_PF + "Roles\\"
DIR_PLAYERS      = DIR_PF + "Players\\"
DIR_BANS         = DIR_PF + "Bans\\"
DIR_NOTES        = DIR_PF + "Notes\\"
DIR_LOGS         = DIR_COT + "Logs\\"
DIR_LOADOUTS     = DIR_COT + "Loadouts\\"
DIR_DELETIONS    = DIR_COT + "Deletions\\"
DIR_COMPENSATIONS = DIR_COT + "Compensations\\"
DIR_EXPORTS      = DIR_COT + "Exports\\"

FILE_WEBHOOK       = DIR_COT + "Webhooks.json"
FILE_WEATHER       = DIR_COT + "Weather.json"
FILE_WEATHER_STATE = DIR_COT + "WeatherState.json"
FILE_ESP           = DIR_COT + "ESP.json"
FILE_TELEPORT      = DIR_COT + "Teleports_"        // + map name + EXT_TELEPORT
FILE_SPAWNER_CONFIG = DIR_COT + "SpawnerConfig.json"
```

`JMConstants` also holds every `PERM_*` node, the `EXT_*` file extensions, the `STAT_*_MIN/MAX` clamp ranges, the RPC
limits (`RPC_MAX_GUIDS`, `INVENTORY_MAX_ITEMS`, `RPC_RATE_LIMIT_S`) and the `WEBHOOK_COLOR_*` palette.

## Module/Sidebar Flow

1. `COTModule` creates `JMCOTSideBar` on mission load
2. `JMCOTSideBar.Init()` sorts `GetModuleManager().GetCOTModules()` and groups them by `GetCategory()` in the order given by
   `JMSideBarConfig.GetCategoryOrder()` (Players, World, Items, Vehicles, Events, Server, Expansion, Other; any other
   category name is appended alphabetically with the settings icon). One `JMCOTSideBarCategory` per category
3. `JMCOTSideBarCategory.AddModule()` creates a tile per module (if `HasButton()`) and calls `module.InitButton(buttonWidget)`
   to set up icon/label/colour
4. On click: the category's `OnClick()` matches the widget → calls `module.ToggleShow()`
5. `ToggleShow()` closes the window if visible; otherwise it checks `HasAccess()` and `GetCommunityOnlineToolsBase().IsActive()`
   and calls `Show()`
6. `Show()` (default build) asks `GetCOTWindowManager().Create()` for a `JMWindowBase`, binds the module with `SetModule()`
   and shows it. Only when `CF_WINDOWS` is defined does it instead create a `CF_Window` from `GetLayoutRoot()`
7. `Close()` → `m_Window.DestroyLater()`; `JMWindowManager` later calls `module.OnWindowDestroyed()` to clear `m_Window`

## Settings Persistence Pattern

Modules persist settings via JSON through `JMJsonFile<T>` (see [module-creation.md](module-creation.md#optional-settings-persistence)):
```c
private ref JMWeatherSerialize m_Settings;

void Load()
{
    m_Settings = JMWeatherSerialize.Load();  // JMJsonFile<JMWeatherSerialize>.Load internally
}

void Save()
{
    m_Settings.Save();                       // JMJsonFile<JMWeatherSerialize>.Save( FILE_WEATHER, this )
}
```
