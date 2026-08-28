---
name: Project Architecture
description: COT directory layout, DayZ script layer system, key entry points, helper functions, and file structure reference
type: project
---

## DayZ Script Layer System

DayZ loads scripts in order — each layer can override previous ones:
- `1_Core/` — Core engine constants, proto wrappers (C++ bindings)
- `3_Game/` — Game-wide data: constants, RPC enums, ban structs, player instances
- `4_World/` — Abstract base classes: JMModuleBase, JMRenderableModuleBase, JMFormBase, JMWindowBase, sidebar, webhook
- `5_Mission/` — Concrete implementations: actual modules, forms, UIActionManager

## Key File Reference

| Path | Purpose |
|------|---------|
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/RPC.c` | All RPC enum definitions with offsets |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/JMConstants.c` | Directory path constants |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/Bans/JMPlayerBan.c` | Ban record struct |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/Bans/JMPlayerBanStore.c` | JSON container for bans |
| `JM/COT/Scripts/3_Game/CommunityOnlineTools/Permissions/JMPlayerInstance.c` | Per-player COT state |
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
modules.Insert( JMMapModule );
modules.Insert( JMVehiclesModule );
modules.Insert( JMCommandModule );
modules.Insert( JMBanModule );
// World-level: COTModule, JMWebhookModule
```

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
DIR_COT         = "$profile:CommunityOnlineTools\\"
DIR_PERMISSIONS = "$profile:PermissionsFramework\\Permissions\\"
DIR_ROLES       = "$profile:PermissionsFramework\\Roles\\"
DIR_PLAYERS     = "$profile:PermissionsFramework\\Players\\"
DIR_BANS        = "$profile:PermissionsFramework\\Bans\\"
DIR_LOGS        = DIR_COT + "Logs\\"
DIR_LOADOUTS    = DIR_COT + "Loadouts\\"
```

## Module/Sidebar Flow

1. `COTModule` creates `JMCOTSideBar` on mission load
2. Sidebar calls `GetModuleManager().GetCOTModules()` and creates a button per module (if `HasButton()`)
3. `module.InitButton(buttonWidget)` sets up icon/label/color
4. On click: sidebar's `OnClick()` matches widget → calls `module.ToggleShow()`
5. `ToggleShow()` → `Show()` creates `CF_Window`, loads `GetLayoutRoot()`, inits form
6. `Close()` → destroys window, calls form's `OnHide()`

## Settings Persistence Pattern

Modules persist settings via JSON:
```c
private ref JMWeatherSerialize m_Settings;

void Load()
{
    m_Settings = JMWeatherSerialize.Load();  // JsonFileLoader internally
}

void Save()
{
    m_Settings.Save();
}
```
