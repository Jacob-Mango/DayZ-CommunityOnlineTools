# COT Module Documentation Index

Per-module reference guides for Community Online Tools. Each module has its own dedicated directory containing its complete reference guide.

## Core & Administration

- [player/README.md](player/README.md) — `JMPlayerModule`: Core player state management, roster, tabs, stat sliders, spectating flow, and ban checks
- [role-manager/README.md](role-manager/README.md) — `JMRoleManagerModule`: Roles and permissions hierarchy (`roles.json`), role assignment, per-permission gating
- [ban-manager/README.md](ban-manager/README.md) — `JMBanModule`: Ban persistence (`bans.json`), temporary and permanent bans, IP/GUID lookups, connection checks
- [action-history/README.md](action-history/README.md) — `JMActionHistoryModule`: Admin action auditing, client history browser, RPC execution logging
- [command/README.md](command/README.md) — `JMCommandModule`: Chat command system (`/cot`, `/tp`, etc.), command execution and permissions
- [webhook/README.md](webhook/README.md) — `JMWebhookModule` / `JMWebhookCOTModule`: Discord webhook integration, administrative logging stream, JSON configuration

## World & Spawning

- [object-spawner/README.md](object-spawner/README.md) — `JMObjectSpawnerModule`: Entity & item spawner, inventory placement, preset quantities, search filtering
- [vehicles/README.md](vehicles/README.md) — `JMVehiclesModule`: Vehicle management, engine state, refuel, repair, lock/unlock, seat assignment, cleanup
- [teleport/README.md](teleport/README.md) — `JMTeleportModule`: Predefined and custom teleport locations (`teleports.json`), position saving, click/admin teleportation
- [loadout/README.md](loadout/README.md) — `JMLoadoutModule`: Loadout presets, player kit spawning, JSON file storage (`Loadouts/`)
- [weather/README.md](weather/README.md) — `JMWeatherModule`: Weather control overview, manual sliders, and dynamic weather preset state machine
- [cot-map/README.md](cot-map/README.md) — `JMMapModule`: Interactive COT map widget, entity/player markers, click-to-teleport, coordinate lookup
- [events/README.md](events/README.md) — `JMEventsModule`: Dynamic server event manager, event triggers, spawning event configurations
- [map-editor/README.md](map-editor/README.md) — `JMMapEditorModule`: 3D object placement editor, transform manipulators, mapping preset export/import

## Diagnostics & Analysis

- [esp/README.md](esp/README.md) — `JMESPModule`: Extra Sensory Perception visual overlay, entity filter types, range sliders, networked object scanning
- [entity-manager/README.md](entity-manager/README.md) — `JMEntityManagerModule`: Live active entity inspector, active entity listing, deletion and manipulation
- [loot-analysis/README.md](loot-analysis/README.md) — `JMLootAnalysisModule`: Central Economy item density analyzer, distribution scanning, report generation
- [server-stats/README.md](server-stats/README.md) — `JMServerStatsModule`: Server performance metrics (FPS, uptime, memory, player counts) telemetry stream
- [anti-cheat/README.md](anti-cheat/README.md) — `JMAntiCheatModule`: Teleport and speed hack detection hooks, inventory validation, disconnect/kick actions
- [item-stats/README.md](item-stats/README.md) — `JMItemStatsModule`: Item statistics and property diagnostic viewer
- [example/README.md](example/README.md) — `JMExampleModule`: Reference implementation / module template for developers

## Mod Integrations & Frameworks

- [compensations/README.md](compensations/README.md) — `JMCompensationsModule`: Player compensation/reward kit distribution system
- [namalsk/README.md](namalsk/README.md) — `JMNamalskEventManagerModule`: Namalsk map event integration (EVR storms, blizzard controls)
- [territory/README.md](territory/README.md) — `JMTerritoryModule`: DayZ-Expansion base building territory management & admin overrides
- [compat/README.md](compat/README.md) — `JMCompatModule`: Cross-mod compatibility layer and legacy API fallbacks
