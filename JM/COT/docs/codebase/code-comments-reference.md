# Code Comments & Implementation Reference

An organized repository guide synthesizing all inline documentation, architecture comments, technical gotchas, and design notes extracted directly from the COT codebase (`3_Game`, `4_World`, `5_Mission`, and `ExampleScriptOverride`).

---

## Table of Contents

1. [Enforce Script Language & Compiler Gotchas](#1-enforce-script-language--compiler-gotchas)
2. [UI & Widget System Design](#2-ui--widget-system-design)
3. [Context Menus & Filter Menus](#3-context-menus--filter-menus)
4. [Module & System Architecture](#4-module--system-architecture)
5. [Permissions & RPC Network Layer](#5-permissions--rpc-network-layer)
6. [Player Management & Spectating](#6-player-management--spectating)
7. [Vehicles & Map Integration](#7-vehicles--map-integration)
8. [Weather & Environmental Systems](#8-weather--environmental-systems)
9. [Webhooks & Audit Logging](#9-webhooks--audit-logging)
10. [Sub-mod Extensibility & ExampleScriptOverride](#10-sub-mod-extensibility--examplescriptoverride)

---

## 1. Enforce Script Language & Compiler Gotchas

Code comments across all three script layers highlight critical engine quirks in Enforce Script:

### Per-File Preprocessor Defines (`#define`)
* **Source:** `Scripts/3_Game/CommunityOnlineTools/StaticFunctions.c`, `Scripts/5_Mission/CommunityOnlineTools/CommunityOnlineTools.c`
* **Note:** `#define` scope in Enforce Script is **per-file, NOT per compiled module**. A `#define` in a `3_Game` or `4_World` file is invisible in `5_Mission`. Global flags (e.g. `COT_DEBUGLOGS`) must be explicitly declared in each file that uses them.

### No Block Scope
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Player/Components/JMPlayerRowMenu.c`
* **Note:** `if` / `else if` branches share the function's top-level namespace. Variables used across branches must be declared once at the top of the function rather than inside individual branch blocks to avoid redeclaration compiler errors.

### Expression Formatting Rules
* **Source:** `docs/systems/conventions.md`
* **Note:** `||`, `&&`, and string concatenation `+` must remain on a single line per sub-expression. Multi-line conditional expressions can fail silently or cause syntax parsing errors in Enforce.

### Foreach & Method Returns
* **Source:** `docs/systems/conventions.md`
* **Note:** `foreach (auto x : obj.GetList())` causes a Null Pointer Exception (NPE) at runtime if `GetList()` returns null or a temporary array. Always assign return values to a local variable before iterating.

### Class Member Protection
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Webhook/JMWebhookForm.c`
* **Note:** Keep module fields `protected` rather than `private`. Sub-mods (e.g. DayZ-Expansion) override form classes using `modded class` and require access to parent handles.

---

## 2. UI & Widget System Design

### Stub Widgets vs. Floating Panels
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/Menus/UIActionContextMenu.c`
* **Note:** `UIActionContextMenu` uses a 1px transparent `layoutRoot` stub so that its per-frame `Update()` handler remains in the active widget hierarchy. The actual visible panel (`m_Panel`) is created as a separate child of `anchor` (e.g. `wnd.GetWidgetRoot()` or `WINDOWS_CONTAINER`). This prevents the panel from being clipped by scrollable parent containers.

### Z-Ordering and Focus
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/Core/UIActionManager.c`
* **Note:** Parenting overlay panels to `JMStatics.WINDOWS_CONTAINER` puts their `SetSort(9999)` in the same sibling group as all COT form windows. This prevents dropdowns and context menus from getting buried when another window receives focus.

### Native Scrollbar Width Hiding Trick
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/Menus/UIActionContextMenu.c`
* **Note:** Enforce `ScrollWidget` lacks a native getter/setter to toggle scrollbar visibility. To hide the scrollbar without losing scrolling capabilities, `SetScrollbarVisible(false)` extends the scroller width past the panel's right edge by the exact scrollbar gutter width (`GetScrollbarWidth()`). The parent panel's `clipchildren = 1` crops the gutter away cleanly.

### Window Geometry Persistence
* **Source:** `Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMWindowLayoutStore.c`
* **Note:** Form window positions and sizes are saved in `profile/CommunityOnlineTools/Layouts/`. Exact pixel sizes (`HEXACTSIZE`, `VEXACTSIZE`) ensure positions survive screen resolution changes without drifting.

---

## 3. Context Menus & Filter Menus

### Open Grace Period (`OPEN_DELAY`)
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/Menus/UIActionContextMenu.c`
* **Note:** Context menus engage a `0.15s` delay (`OPEN_DELAY`) after opening. This prevents the mouse button press that raised the menu from immediately triggering an outside-click dismissal on the first frame.

### Deferred Rebuilding (`DeferRebuild`)
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/Menus/UIActionFilterMenu.c`
* **Note:** Page navigation or toggle repaints in `UIActionFilterMenu` must be deferred via `g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(RebuildCurrentPage)`. Rebuilding synchronously inside a row's click handler destroys the press target mid-event, causing the engine to reset cursor focus.

### Consistent Menu Shape
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Player/Components/JMPlayerRowMenu.c`
* **Note:** When permissions are missing or actions are invalid for a player state, menu rows are disabled/greyed out (`SetItemEnabled(id, false)`) rather than removed. Keeping a static menu layout prevents mis-clicks when operating on multiple rows.

---

## 4. Module & System Architecture

### Module Registration Sequence
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/JMModuleConstructor.c`
* **Note:** `JMModuleConstructor.RegisterModules()` registers active modules into `TTypenameArray`. Sub-mods append custom modules by overriding `RegisterModules` in `modded class JMModuleConstructor`.

### Module Lifecycle
* **Source:** `Scripts/4_World/CommunityOnlineTools/Classes/Module/JMModuleBase.c`
* 1. `OnInit()`: Internal data structure initialization.
* 2. `RegisterRPCs()`: Binds RPC ids to module methods via `AddRPC`.
* 3. `OnMissionLoaded()`: Executed once the mission host or client enters gameplay.
* 4. `OnSettingsUpdated()`: Invoked when JSON configuration files change on disk.

### Sidebar Categories
* **Source:** `Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c`
* **Note:** Sidebar modules are organized into categories: `General`, `Players`, `Environment`, `Server`, and `Utilities`. Sidebuttons bind to `btn`, `ttl`, `btn_img`, and `btn_txt` inside `JMRenderableModuleBase`.

---

## 5. Permissions & RPC Network Layer

### RPC Sub-Range Allocation
* **Source:** `Scripts/3_Game/CommunityOnlineTools/RPC.c`
* **Note:** Each module receives a dedicated RPC enum block (e.g. `JMPlayerModuleRPC`, `JMVehiclesModuleRPC`). RPCs must validate caller permissions on the server using `JMPermissions.HasRPC(identity, perm)` before executing authority logic.

### Server Host Direct Call vs. Client RPC Split
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Namalsk/JMNamalskEventManagerModule.c`
* **Note:** Server authority checks use `IsMissionHost()`: if called on the mission server directly, execution bypasses network serialization and calls `Exec_*` or internal logic immediately. If invoked on a client, it writes parameters to a `ScriptRPC` and dispatches it over the network.

---

## 6. Player Management & Spectating

### Spectator Controller Detach/Attach
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c`
* **Note:** Spectate mode creates a spectator entity (`PlayerSpectator`), detaches the admin's camera from their body, and disables input on the original `PlayerBase` controller. Ending spectate restores camera target and reinstates local input.

### Roster Refresh & Stat Trackers
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c`
* **Note:** Roster rows poll player instances (`JMPlayerInstance`). Stat trackers subscribe to `JMScriptInvokers` events for real-time updates without polling global lists every tick.

---

## 7. Vehicles & Map Integration

### Network ID Tracking
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Vehicles/Tabs/Map/JMVehiclesFormTabMap.c`
* **Note:** Vehicles are tracked using split 32-bit integer pairs (`m_NetworkIDLow`, `m_NetworkIDHigh`) to safely reference network entities without object pointer corruption across re-spawns.

### Expansion Vehicle Guards
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Vehicles/Tabs/Map/JMVehiclesFormTabMap.c`
* **Note:** Vehicle actions specific to DayZ-Expansion (cover/uncover, key pairing, locking) are guarded by `#ifdef EXPANSIONMODVEHICLE`. Context menu `SetItemEnabled` silently ignores unregistered IDs, allowing shared layout definitions.

---

## 8. Weather & Environmental Systems

### Weather State Machine (`WeatherState.json`)
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Weather/`
* **Note:** Weather state transitions follow a percentage-weighted state machine saved to `profile/CommunityOnlineTools/WeatherState.json`. Sliders drive manual overrides while automated updates lerp target values via `g_Game.GetWeather()`.

---

## 9. Webhooks & Audit Logging

### Webhook Section Updates (`UpdateState`)
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/modules/Webhook/JMWebhookForm.c`
* **Note:** Toggling event checkboxes or saving webhooks uses `UpdateState()` to refresh widgets in place. Full form rebuilds only occur when webhooks are created, deleted, or renamed. This prevents wiping unsaved text in other open webhook cards.

### Minimum Webhook Interval
* **Source:** `Scripts/5_Mission/CommunityOnlineTools/CommunityOnlineTools.c`
* **Note:** Admin activation toggles enforce `ACTIVE_MIN_INTERVAL_MS = 2000` to rate-limit outbound HTTP POST payloads to Discord endpoints.

---

## 10. Sub-mod Extensibility & ExampleScriptOverride

### Extension Points
* **Source:** `ExampleScriptOverride/README.md`
* Sub-mods extend COT via standardized registries:
  - **Modules:** `modded class JMModuleConstructor` -> `modules.Insert(MyModule)`
  - **Filter Menus:** `JMFilterRegistry.Register(scope, entry)`
  - **Context Menus:** `JMContextMenuRegistry.Populate(menuType, menu)`
  - **Form Tabs:** `form.AddTab(tabInstance)`

---

*Document generated from codebase comments audit.*
