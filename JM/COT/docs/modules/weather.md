---
name: Weather Module
description: Weather control overview, manual sliders (rain, fog, wind, overcast, date/time), linking to weather-dynamic.md deep dive
type: project
---

## Overview

The Weather Module (`JMWeatherModule`) provides manual and preset control over DayZ environment conditions. Administrators can adjust overcast, rain, fog, snowfall, wind speed/direction, ground wetness, temperature, and date/time (year, month, day, hour, minute) in real time.

For an in-depth breakdown of the dynamic weather state machine, phase configurations, percentage lists, and state file persistence, see [weather-dynamic.md](weather-dynamic.md).

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Weather/JMWeatherModule.c` — Weather controller, engine weather API interface (`g_Game.GetWeather()`), RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Weather/JMWeatherForm.c` — Main UI window containing manual weather sliders and dynamic preset tabs

## RPC Range

Range: `10600` – `10623` (`JMWeatherModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10600` | `SetOvercast` | Client → Server | Sets overcast target value and transition time |
| `10601` | `SetRain` | Client → Server | Sets rain target value and transition time |
| `10602` | `SetFog` | Client → Server | Sets fog density and distance parameters |
| `10603` | `SetWind` | Client → Server | Sets wind speed and direction vector |
| `10604` | `SetDate` | Client → Server | Sets world year, month, day, hour, and minute |
| `10605` | `SetSnow` | Client → Server | Sets snowfall target value (on supported maps) |
| `10606`..`10623` | `DynamicState` | Client ↔ Server | Dynamic weather machine control and preset synchronization |

## Permissions

- `PERM_WEATHER_VIEW` (`Weather.View`) — Open Weather management form
- `PERM_WEATHER_SET` (`Weather.Set`) — Apply manual weather slider changes or date/time adjustments
- `PERM_WEATHER_DYNAMIC` (`Weather.Dynamic`) — Edit dynamic weather state machine presets
