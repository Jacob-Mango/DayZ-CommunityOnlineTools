---
name: Weather Module
description: Weather control overview, manual sliders (rain, fog, wind, overcast, date/time), dynamic weather preset state machine, and persistence
type: project
---

## Overview

The Weather Module (`JMWeatherModule`) provides manual and preset control over DayZ environment conditions. Administrators can adjust overcast, rain, fog, snowfall, wind speed/direction, ground wetness, temperature, and date/time (year, month, day, hour, minute) in real time.

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

---

## Dynamic Weather Presets

### What It Is

A **preset** is a named weather system ("Storm front") made of **phases** ("Building", "Peak", "Clearing"). Each phase owns the weather it puts on (`Conditions`, a `JMWeatherPreset`), how long it holds and how long it fades in. There is no separate list of stored presets any more - the preset IS the thing the dynamic machine walks (the code still calls it `JMWeatherState`).

Every choice the machine makes is a percentage list that adds up to 100:

| List | Lives on | Targets | Drawn when |
|---|---|---|---|
| `Edges` | `JMWeatherPhase` | phases of the same preset, or `""` = **leave the preset** | a phase's hold ends |
| `NextStates` | `JMWeatherState` | other presets (itself allowed) | an edge said "leave" |
| `Entry` | `JMWeatherState` | phases of that preset | the preset is entered (empty = first phase) |

The hold is drawn **once** on entering and is both the timer and the `MinDuration` handed to the engine. A phase whose edges list itself is how weather persists across several rolls. A phase or preset with no candidates holds and re-rolls after another hold - it never stops the machine.

### Min/Max Ranges

Every numeric weather value can be a **span** instead of a number. The low bound is the value's own field; the high bound is its `...Hi` twin (`Forecast`/`ForecastHi`, `Storm.Density`/`DensityHi`, `PDynFog.Distance`/`DistanceHi`, `WindFunc.Min`/`MinHi`, sandstorm `Duration`/`DurationHi`, ...). A value is a span only when `Hi > low`; an absent, zero or equal `Hi` (which is what a file written before ranges loads as) is a plain value, and `-1` still means "leave alone".

`JMWeatherRoll.Pick(low, high)` rolls **once per apply**, in every `Apply()`, so the same preset gives a different sky each time it is entered. The rain/snow thresholds are already a min/max pair of their own and stay as they were.

### Dynamic Weather System Files

Location: `Scripts/5_Mission/CommunityOnlineTools/modules/Weather/`

| File | Role |
|---|---|
| `JMWeatherNextState.c` | one weighted edge + `Roll` / `Normalize` / `Find` / `Remove` / `Rename` / `Copy` |
| `JMWeatherPhase.c`, `JMWeatherState.c` | config nodes; a phase owns `Conditions` |
| `JMWeatherRoll.c` | `Pick(low, high)`, the one roll behind every range |
| `JMWeatherPreset.c` | the weather values (`JMWeatherPreset`), `Neutral()`, `Copy()`, `ClearDate()`, the `...Hi` twins |
| `JMWeatherSerialize.c` | `Weather.json`: `States`, `DynamicEnabled`; migrations, `NormalizeState`, `SaveState`, `RemoveState`, `GetNextOdds` |
| `JMWeatherRuntime.c` | `WeatherState.json`: where the machine is |
| `JMWeatherMachine.c` | server-only engine: timer, rolls, pause, adopt, resume |
| `JMWeatherDynamicConfig.c` / `Control.c` / `Status.c` | RPC payloads: edit / move+apply / read position |
| `Components/JMWeatherRangeControl.c`, `JMWeatherRangeToggle.c` | value-or-span slider, title-bar switch |
| `Tabs/Presets/JMWeatherChanceList.c` | reusable "list that sums to 100" widget |
| `Tabs/Presets/JMWeatherDynamicStatusCard.c` | "Current Preset" live card |
| `Tabs/Presets/JMWeatherDynamicEditor.c` | preset working copy editor |

### The Dynamic Presets Tab UI

Top to bottom: **Current Preset** (live status), **Presets**, **Next Presets**, **Phase**, **Opens On**, **After This Phase**.

- Every action is an icon in the card's title bar. **Presets**: Add, Apply Now, Delete (asks first), Save - the Save icon turns green and pulses while there is anything unsaved.
- **Current Preset** card: pause/play, fast-forward (advance) and restart.
- **Picking a preset (or Add) IS editing it** - there is no Edit button. The weather tabs enter *preset edit mode*: Sky / Precipitation / Wind show and edit the weather of the phase selected in the Phase card.
- Permissions: Add/Save-new `Weather.Preset.Create`, Save-existing `Weather.Preset.Update`, Delete `Weather.Preset.Remove`, Apply Now `Weather.Preset.Use`, the master switch and Advance/Restart/Jump `Weather.Preset.Dynamic`.

### Persistence

- **Enabled** is configuration: `Weather.json` -> `DynamicEnabled`. Unchanged across restarts.
- **Position** is runtime: `$profile:CommunityOnlineTools\WeatherState.json` (`State`, `Phase`, `HoldTotal`, `HoldRemaining`). Written on every phase change, on enable/disable, on freeze/unfreeze, on clean shutdown, and by a 60 s heartbeat.
- **Boot**: enabled + a recorded preset/phase that still exist -> the phase's weather is re-applied at once with `MinDuration = HoldRemaining`.
