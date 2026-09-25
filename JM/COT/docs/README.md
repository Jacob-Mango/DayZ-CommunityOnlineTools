# Community Online Tools (COT) Documentation

Comprehensive documentation index for DayZ Community Online Tools (COT).

---

## Directory Organization

```
docs/
├── codebase/     Architecture, module lifecycle, RPCs, permissions, code comments reference
├── game/         DayZ engine scripts reference, PlayerBase mechanics, vanilla widget XML/flags
├── conventions/  Coding standards, naming grammar, style guide, cleanup playbook, folder rules
├── ui/           UIAction components guide, form patterns, window & sidebar, filter menus
└── modules/      Per-module deep dives (Player, Vehicles, Teleport, ESP, Webhooks, Weather, etc.)
```

---

## 1. Codebase Architecture & Systems (`codebase/`)

- [codebase/architecture.md](codebase/architecture.md) — Repository structure, script module layers (`1_Core`/`3_Game`/`4_World`/`5_Mission`), registered module lifecycle, profile paths, sidebar navigation flow.
- [codebase/rpc.md](codebase/rpc.md) — RPC enum allocation ranges, `ScriptRPC` read/write/send mechanics, network routing, server authority enforcement, never-targeted rule.
- [codebase/permissions.md](codebase/permissions.md) — Permission string registration, `JMPermissions.Has` & `HasRPC` checks, `roles.json` hierarchy, player instance permissions.
- [codebase/module-creation.md](codebase/module-creation.md) — Step-by-step guide to building a new COT module: `DescribeModule`, `JMModuleAction`, RPC enum allocation, registration, settings JSON persistence.
- [codebase/code-comments-reference.md](code-comments-reference.md) — Synthesized reference index of inline code comments, language quirks, UI constraints, and architectural notes.
- [codebase/mod-compatibility.md](codebase/mod-compatibility.md) — API contracts with dependent mods (e.g. DayZ-Expansion): `protected` vs `private` rules, override safety, compatibility audit script.
- [codebase/deprecations.md](codebase/deprecations.md) — Mapping of deprecated methods and classes, forwarder warnings (`WarnOnce`), legacy compatibility layer.
- [codebase/advanced.md](codebase/advanced.md) — Deep technical notes on `JMScriptInvokers`, notifications, active state flags, JSON serializers, chat commands, and framework integrations.

---

## 2. DayZ Engine & Game Mechanics (`game/`)

- [game/dayz-reference.md](game/dayz-reference.md) — Overview of vanilla DayZ script structures at `DayZ Projects/scripts`: entity class hierarchy, `PlayerBase` stats, inventory APIs, weather engine API, Enforce spawn flags (`ECE_PLACE_ON_SURFACE`, etc.).
- [game/playerbase.md](game/playerbase.md) — COT modifications and interactions with `PlayerBase`: godmode, movement freeze, character invisibility, health/blood/water/energy stats, condition cures, `Exec_*` server pattern.

---

## 3. Project Conventions & Standards (`conventions/`)

- [conventions/conventions.md](conventions/conventions.md) — **Essential reading before editing scripts**: what not to do (comments, scope creep, permissions, preprocessor `#ifdef` rules, build/verify steps, Enforce runtime gotchas, mod compatibility).
- [conventions/naming.md](conventions/naming.md) — Identifier naming grammar: `Add`/`Remove` pairs, lifecycle hooks (`On<Subject><Phase>`), `Exec_` vs `RPC_`, popups, verbs.
- [conventions/style-guide.md](conventions/style-guide.md) — Code formatting, line endings (`CRLF`/`LF`), tab indentation, variable declaration order, comment formatting.
- [conventions/cleanup-playbook.md](conventions/cleanup-playbook.md) — Step-by-step playbook for refactoring legacy forms: member ordering, tab state ownership, component extraction, verification workflows.
- [conventions/module-folder-layout.md](conventions/module-folder-layout.md) — Module file organization standards: `Tabs/<Name>/`, `Components/`, shared helpers, `organize_tabs.py` batch tool.

---

## 4. UI Action Components & Form Building (`ui/`)

- [ui/components-guide.md](ui/components-guide.md) — **START HERE**: complete developer guide to all `UIAction` components (buttons, sliders, inputs, toggles, prompts, tables, scrollers, filter menus, etc.) and when/how to use each.
- [ui/module-form-patterns.md](ui/module-form-patterns.md) — Start here to build or maintain a COT form: panel layouts, lazy tabs, overlays, permissions, tab classes, splitting forms into components.
- [ui/forms.md](ui/forms.md) — `JMFormBase` lifecycle, `UIActionManager` API, confirmations, permission gating.
- [ui/window-and-sidebar.md](ui/window-and-sidebar.md) — `JMWindowBase` drag/resize/pin/embed/z-order, `JMCOTSideBar` (categories, footer), `CommunityOnlineToolsBase` lifecycle.
- [ui/filter-menus.md](ui/filter-menus.md) — `UIActionFilterMenu`: the single filter-button menu, how to build one and how a mod adds a row (`JMFilterRegistry`).
- [ui/tab-injection-extensibility.md](ui/tab-injection-extensibility.md) — Adding tabs, panels, context-menu items and modules from a sub-mod (`AddTab`, `ExampleScriptOverride/`).
- [ui/limitations.md](ui/limitations.md) — Known Enforce Script layout limitations, widget clipping rules, and scroller workarounds.
- [ui/widget-input-localization.md](ui/widget-input-localization.md) — Layout XML widget types, `UIActionSlider/SelectBox`, input action names, stringtable format.
- [ui/overview.md](ui/overview.md) — DayZ vanilla widget system overview and layout file format details.

---

## 5. Modules Index (`modules/`)

- [modules/README.md](modules/README.md) — Complete per-module index linking all COT module reference guides.
- [modules/player.md](modules/player.md) — `JMPlayerModule`: player management overview (godmode, freeze, invisibility, stats, heal, strip).
- [modules/player-flows.md](modules/player-flows.md) — `JMPlayerForm` roster and tabs, stat slider ranges, spectating start/end flow, roster refresh, ban check at login, `PluginAdminLog` webhook hooks.
- [modules/vehicles.md](modules/vehicles.md) — `JMVehiclesModule`: vehicle management, engine state, refuel, repair, lock/unlock, key management, Expansion integration.
- [modules/teleport.md](modules/teleport.md) — `JMTeleportModule`: predefined and custom teleport locations (`teleports.json`), position history.
- [modules/object-spawner.md](modules/object-spawner.md) — `JMObjectSpawnerModule`: item & entity spawner, inventory placement, search filters.
- [modules/esp.md](modules/esp.md) — `JMESPModule`: Extra Sensory Perception visual overlay, entity filter types, range sliders, bounding boxes.
- [modules/weather.md](modules/weather.md) — `JMWeatherModule`: manual weather sliders (rain, fog, wind, overcast, date/time).
- [modules/weather-dynamic.md](modules/weather-dynamic.md) — Dynamic weather state machine: state/phase config, percentage lists, `WeatherState.json` persistence, RPCs.
- [modules/cot-map.md](modules/cot-map.md) — `JMMapModule`: interactive COT map widget, markers, click-to-teleport.
- [modules/events.md](modules/events.md) — `JMEventsModule`: server event manager, dynamic event triggers, event spawning.
- [modules/map-editor.md](modules/map-editor.md) — `JMMapEditorModule`: 3D object placement editor, transform manipulators, layout export.
- [modules/loot-analysis.md](modules/loot-analysis.md) — `JMLootAnalysisModule`: Central Economy item density analyzer, distribution scanning.
- [modules/server-stats.md](modules/server-stats.md) — `JMServerStatsModule`: real-time server telemetry (FPS, memory, player counts).
- [modules/anti-cheat.md](modules/anti-cheat.md) — `JMAntiCheatModule`: speed hack / teleport detection hooks, inventory checks.
- [modules/webhook.md](modules/webhook.md) — `JMWebhookModule` / `JMWebhookCOTModule`: Discord webhook integration, administrative audit logging.
- [modules/loadout.md](modules/loadout.md) — `JMLoadoutModule`: loadout presets, player kit spawning, JSON loadout storage (`Loadouts/`).
- [modules/role-manager.md](modules/role-manager.md) — `JMRoleManagerModule`: roles and permissions hierarchy (`roles.json`), role assignment.
- [modules/ban-manager.md](modules/ban-manager.md) — `JMBanModule`: ban persistence (`bans.json`), timed and permanent bans, GUID/IP lookups.
- [modules/action-history.md](modules/action-history.md) — `JMActionHistoryModule`: admin action logging, audit history browser, RPC execution.
- [modules/command.md](modules/command.md) — `JMCommandModule`: chat commands (`/cot`, `/tp`, etc.), permission checks.
- [modules/entity-manager.md](modules/entity-manager.md) — `JMEntityManagerModule`: live active entity browser, deletion, inspection.
- [modules/item-stats.md](modules/item-stats.md) — `JMItemStatsModule`: item property diagnostic viewer.
- [modules/compensations.md](modules/compensations.md) — `JMCompensationsModule`: player reward and compensation kit distribution.
- [modules/namalsk.md](modules/namalsk.md) — `JMNamalskEventManagerModule`: Namalsk map event integration (EVR storms, blizzards).
- [modules/territory.md](modules/territory.md) — `JMTerritoryModule`: DayZ-Expansion base building territory management.
- [modules/compat.md](modules/compat.md) — `JMCompatModule`: cross-mod compatibility layer.
- [modules/example.md](modules/example.md) — `JMExampleModule`: reference implementation / template module.

---

*Last updated: September 2026*
