# COT Module Documentation Index

Per-module reference guides for Community Online Tools.

## Core & Administration

- [player.md](player.md) — `JMPlayerModule`: Core player state management (godmode, freeze, invisibility, stats, heal, strip, spectate setup)
- [player-flows.md](player-flows.md) — `JMPlayerForm` roster and tabs deep dive, stat slider ranges, spectating flow, roster updates, ban check at login
- [role-manager.md](role-manager.md) — `JMRoleManagerModule`: Roles and permissions hierarchy (`roles.json`), role assignment, per-permission gating
- [ban-manager.md](ban-manager.md) — `JMBanModule`: Ban persistence (`bans.json`), temporary and permanent bans, IP/GUID lookups, connection checks
- [action-history.md](action-history.md) — `JMActionHistoryModule`: Admin action auditing, client history browser, RPC execution logging
- [command.md](command.md) — `JMCommandModule`: Chat command system (`/cot`, `/tp`, etc.), command execution and permissions
- [webhook.md](webhook.md) — `JMWebhookModule` / `JMWebhookCOTModule`: Discord webhook integration, administrative logging stream, JSON configuration

## World & Spawning

- [object-spawner.md](object-spawner.md) — `JMObjectSpawnerModule`: Entity & item spawner, inventory placement, preset quantities, search filtering
- [vehicles.md](vehicles.md) — `JMVehiclesModule`: Vehicle management, engine state, refuel, repair, lock/unlock, seat assignment, cleanup
- [teleport.md](teleport.md) — `JMTeleportModule`: Predefined and custom teleport locations (`teleports.json`), position saving, click/admin teleportation
- [loadout.md](loadout.md) — `JMLoadoutModule`: Loadout presets, player kit spawning, JSON file storage (`Loadouts/`)
- [weather.md](weather.md) — `JMWeatherModule`: Weather control overview, manual sliders (rain, fog, wind, overcast, date/time)
- [weather-dynamic.md](weather-dynamic.md) — Dynamic weather state machine deep dive, preset/phase transitions, `WeatherState.json` persistence
- [cot-map.md](cot-map.md) — `JMMapModule`: Interactive COT map widget, entity/player markers, click-to-teleport, coordinate lookup
- [events.md](events.md) — `JMEventsModule`: Dynamic server event manager, event triggers, spawning event configurations
- [map-editor.md](map-editor.md) — `JMMapEditorModule`: 3D object placement editor, transform manipulators, mapping preset export/import

## Diagnostics & Analysis

- [esp.md](esp.md) — `JMESPModule`: Extra Sensory Perception visual overlay, entity filter types, range sliders, networked object scanning
- [entity-manager.md](entity-manager.md) — `JMEntityManagerModule`: Live active entity inspector, active entity listing, deletion and manipulation
- [loot-analysis.md](loot-analysis.md) — `JMLootAnalysisModule`: Central Economy item density analyzer, distribution scanning, report generation
- [server-stats.md](server-stats.md) — `JMServerStatsModule`: Server performance metrics (FPS, uptime, memory, player counts) telemetry stream
- [anti-cheat.md](anti-cheat.md) — `JMAntiCheatModule`: Teleport and speed hack detection hooks, inventory validation, disconnect/kick actions
- [item-stats.md](item-stats.md) — `JMItemStatsModule`: Item statistics and property diagnostic viewer
- [example.md](example.md) — `JMExampleModule`: Reference implementation / module template for developers

## Mod Integrations & Frameworks

- [compensations.md](compensations.md) — `JMCompensationsModule`: Player compensation/reward kit distribution system
- [namalsk.md](namalsk.md) — `JMNamalskEventManagerModule`: Namalsk map event integration (EVR storms, blizzard controls)
- [territory.md](territory.md) — `JMTerritoryModule`: DayZ-Expansion base building territory management & admin overrides
- [compat.md](compat.md) — `JMCompatModule`: Cross-mod compatibility layer and legacy API fallbacks
