# COT Documentation

Reference docs for DayZ Community Online Tools.

## Layout

```
docs/
├── ui/          DayZ vanilla UI reference + COT form patterns
├── systems/     COT architecture, RPC, permissions, modules
└── modules/     Per-module deep dives (player flows, etc.)
```

## Quick links

### DayZ UI & forms

- [ui/README.md](ui/README.md) — index of every UI doc (vanilla reference + COT forms)
- [ui/forms.md](ui/forms.md) — `JMFormBase` lifecycle, `UIActionManager` API, confirmations, permission gating
- [ui/module-form-patterns.md](ui/module-form-patterns.md) — **start here to build or maintain a form**: panel layouts, lazy tabs, overlays, permissions, tab classes, splitting a form into components, where files go, code style, how to verify, and the new-form checklist
- [ui/limitations.md](ui/limitations.md) — known Enforce Script layout limitations and workarounds
- [ui/filter-menus.md](ui/filter-menus.md) — `UIActionFilterMenu`: the one filter-button menu, how to build one and how a mod adds a row (`JMFilterRegistry`)
- [ui/tab-injection-extensibility.md](ui/tab-injection-extensibility.md) — adding tabs, panels, context-menu items and modules from a sub-mod (`AddTab`, `ExampleScriptOverride/`)
- [ui/window-and-sidebar.md](ui/window-and-sidebar.md) — `JMWindowBase` drag/resize/pin/embed/z-order, `JMCOTSideBar` (categories, footer), `CommunityOnlineToolsBase` lifecycle, modded classes
- [ui/widget-input-localization.md](ui/widget-input-localization.md) — layout XML widget types, `UIActionSlider/SelectBox`, all 45 input action names, stringtable format
- [ui/styling/style-guide.md](ui/styling/style-guide.md) — naming conventions, patterns, best practices for COT forms
- [ui/styling/textures-edds-and-paa.md](ui/styling/textures-edds-and-paa.md) - converting an image to `.edds` or `.paa`, the `ImageToPAA` suffix rules, the `.edds` container

### COT systems

- [systems/architecture.md](systems/architecture.md) — repo structure, layer system (1_Core/3_Game/4_World/5_Mission), registered modules, profile paths, sidebar flow
- [systems/module-creation.md](systems/module-creation.md) — how to create a new module: `DescribeModule`, `JMModuleAction`, RPC enum, registration, settings persistence
- [systems/rpc.md](systems/rpc.md) — RPC enum pattern, every allocated id range, client↔server flow, `ScriptRPC` write/read/send, never-targeted rule
- [systems/permissions.md](systems/permissions.md) — registering permissions, `HasPermission` checks, roles, `JMPlayerInstance`
- [systems/advanced.md](systems/advanced.md) — `JMScriptInvokers`, COT active state, notifications, admin logging, JSON persistence, chat commands, ESP, loadouts, object spawner, weather Send/Exec pattern, MissionServer overrides, CF framework, camera, teleport persistence, vehicle enumeration, MissionGameplay, Expansion integration
- [systems/playerbase.md](systems/playerbase.md) — COT `PlayerBase` properties/methods (godmode/freeze/invisibility), `Exec_*` pattern, all player permissions, static helpers, `config.cpp` defines
- [systems/dayz-reference.md](systems/dayz-reference.md) — vanilla DayZ scripts at `DayZ Projects/scripts`: entity hierarchy, `PlayerBase` stats, inventory, weather API, ECE spawn flags
- [systems/mod-compatibility.md](systems/mod-compatibility.md) — COT's API contract with dependent mods (DayZ-Expansion): why `private` breaks `modded class`, which members are held `protected`, and the audit script to re-check
- [systems/module-folder-layout.md](systems/module-folder-layout.md) — where files go inside a module folder: `Tabs/<Name>/`, `Components/`, what is shared, and `organize_tabs.py` which applies it
- [systems/naming.md](systems/naming.md) — naming grammar: `Add`/`Remove`, lifecycle hooks (`On<Subject><Phase>`), `Exec_` / `RPC_`, popups, verbs
- [systems/deprecations.md](systems/deprecations.md) — every old name that still works, its replacement, what was removed without a forwarder, and behaviour changes
- [systems/conventions.md](systems/conventions.md) — **read before editing scripts**: what not to do (comments, scope creep, permissions, version `#ifdef`s, generated files, build/verify traps, native-crash debugging, Enforce Script runtime gotchas, other-mod compatibility), class field layout, and commit rules
- [systems/cleanup-playbook.md](systems/cleanup-playbook.md) — how forms were cleaned up (member order, tab-owned variables, component extraction, folder layout, verification), the lessons behind each rule, and the tools in `Workbench/Batchfiles/cleanup/` for repeating it on other files

### Modules

- [modules/README.md](modules/README.md) — **module index**: complete list of all COT module reference guides
- [modules/player.md](modules/player.md) — `JMPlayerModule`: player management overview (godmode, freeze, invisibility, stats, heal, strip)
- [modules/player-flows.md](modules/player-flows.md) — `JMPlayerForm` roster and tabs, stat slider ranges, spectating start/end flow, roster refresh, ban check at login, `PluginAdminLog` webhook hooks
- [modules/role-manager.md](modules/role-manager.md) — `JMRoleManagerModule`: roles and permissions hierarchy (`roles.json`), role assignment
- [modules/ban-manager.md](modules/ban-manager.md) — `JMBanModule`: ban persistence (`bans.json`), timed and permanent bans, GUID/IP lookups
- [modules/action-history.md](modules/action-history.md) — `JMActionHistoryModule`: admin action logging, audit history browser, RPC execution
- [modules/command.md](modules/command.md) — `JMCommandModule`: chat commands (`/cot`, `/tp`, etc.), permission checks
- [modules/webhook.md](modules/webhook.md) — `JMWebhookModule` / `JMWebhookCOTModule`: Discord webhook integration, administrative audit logging
- [modules/object-spawner.md](modules/object-spawner.md) — `JMObjectSpawnerModule`: item & entity spawner, inventory placement, search filters
- [modules/vehicles.md](modules/vehicles.md) — `JMVehiclesModule`: vehicle management, engine state, refuel, repair, lock/unlock, deletion
- [modules/teleport.md](modules/teleport.md) — `JMTeleportModule`: predefined and custom teleport locations (`teleports.json`), position history
- [modules/loadout.md](loadout.md) — `JMLoadoutModule`: loadout presets, player kit spawning, JSON loadout storage (`Loadouts/`)
- [modules/weather.md](modules/weather.md) — `JMWeatherModule`: manual weather sliders (rain, fog, wind, overcast, date/time)
- [modules/weather-dynamic.md](modules/weather-dynamic.md) — dynamic weather state machine: state/phase config, percentage lists, `WeatherState.json` persistence, RPCs
- [modules/cot-map.md](modules/cot-map.md) — `JMMapModule`: interactive COT map widget, markers, click-to-teleport
- [modules/events.md](modules/events.md) — `JMEventsModule`: server event manager, dynamic event triggers, event spawning
- [modules/map-editor.md](modules/map-editor.md) — `JMMapEditorModule`: 3D object placement editor, transform manipulators, layout export
- [modules/esp.md](modules/esp.md) — `JMESPModule`: Extra Sensory Perception visual overlay, entity filter types, range sliders, bounding boxes
- [modules/entity-manager.md](modules/entity-manager.md) — `JMEntityManagerModule`: live active entity browser, deletion, inspection
- [modules/loot-analysis.md](modules/loot-analysis.md) — `JMLootAnalysisModule`: Central Economy item density analyzer, distribution scanning
- [modules/server-stats.md](modules/server-stats.md) — `JMServerStatsModule`: real-time server telemetry (FPS, memory, player counts)
- [modules/anti-cheat.md](modules/anti-cheat.md) — `JMAntiCheatModule`: speed hack / teleport detection hooks, inventory checks
- [modules/item-stats.md](modules/item-stats.md) — `JMItemStatsModule`: item property diagnostic viewer
- [modules/example.md](modules/example.md) — `JMExampleModule`: reference implementation / template module
- [modules/compensations.md](modules/compensations.md) — `JMCompensationsModule`: player reward and compensation kit distribution
- [modules/namalsk.md](modules/namalsk.md) — `JMNamalskEventManagerModule`: Namalsk map event integration (EVR storms, blizzards)
- [modules/territory.md](modules/territory.md) — `JMTerritoryModule`: DayZ-Expansion base building territory management
- [modules/compat.md](modules/compat.md) — `JMCompatModule`: cross-mod compatibility layer


## Conventions used in these docs

- **Code references** use the `file:line` format and link relative to the repo root.
- **Engine truth** boxes mark behaviour verified against the engine bindings.
- **Convention** boxes mark project-level conventions that aren't enforced by the engine.
- Cross-doc links use relative paths (`../systems/rpc.md`) so docs work both in repo browsing and external viewers.

## Last updated

2026-09-21 - every doc under `systems/`, `modules/` and the COT-specific `ui/` pages was checked against the code on this date (RPC ranges with `Workbench/Batchfiles/CheckRPCRanges.ps1`, identifiers by search). The vanilla widget reference (`ui/widgets/`, `ui/events/`, `ui/patterns/`, `ui/reference/`, `ui/workspace/`) describes engine behaviour and was only spot-checked against `scripts/1_core/proto/enwidgets.c`.

