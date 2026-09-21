---
name: Dynamic Weather Presets
description: Presets are two-level (preset / phase) percentage-based weather machines that own their weather - config shape, min/max ranges, runtime persistence, RPCs, the Dynamic Presets tab, edit mode, freeze/manual-apply rules and how to verify a restart resumes
type: project
---

## What it is

A **preset** is a named weather system ("Storm front") made of **phases** ("Building", "Peak",
"Clearing"). Each phase owns the weather it puts on (`Conditions`, a `JMWeatherPreset`), how long it
holds and how long it fades in. There is no separate list of stored presets any more - the preset
IS the thing the dynamic machine walks (the code still calls it `JMWeatherState`).

Every choice the machine makes is a percentage list that adds up to 100:

| List | Lives on | Targets | Drawn when |
|---|---|---|---|
| `Edges` | `JMWeatherPhase` | phases of the same preset, or `""` = **leave the preset** | a phase's hold ends |
| `NextStates` | `JMWeatherState` | other presets (itself allowed) | an edge said "leave" |
| `Entry` | `JMWeatherState` | phases of that preset | the preset is entered (empty = first phase) |

The hold is drawn **once** on entering and is both the timer and the `MinDuration` handed to the
engine. A phase whose edges list itself is how weather persists across several rolls. A phase or
preset with no candidates holds and re-rolls after another hold - it never stops the machine.

## Min/max ranges

Every numeric weather value can be a **span** instead of a number. The low bound is the value's own
field; the high bound is its `...Hi` twin (`Forecast`/`ForecastHi`, `Storm.Density`/`DensityHi`,
`PDynFog.Distance`/`DistanceHi`, `WindFunc.Min`/`MinHi`, sandstorm `Duration`/`DurationHi`, ...).
A value is a span only when `Hi > low`; an absent, zero or equal `Hi` (which is what a file written
before ranges loads as) is a plain value, and `-1` still means "leave alone".
`JMWeatherRoll.Pick(low, high)` rolls **once per apply**, in every `Apply()`, so the same preset
gives a different sky each time it is entered. The rain/snow thresholds are already a min/max pair of
their own and stay as they were.

## Files

`Scripts/5_Mission/CommunityOnlineTools/modules/Weather/`

| File | Role |
|---|---|
| `JMWeatherNextState.c` | one weighted edge + `Roll` / `Normalize` / `Find` / `Remove` / `Rename` / `Copy` |
| `JMWeatherPhase.c`, `JMWeatherState.c` | config nodes; a phase owns `Conditions` |
| `JMWeatherRoll.c` | `Pick(low, high)`, the one roll behind every range (own class: a static on the payload base class broke the RPC write) |
| `JMWeatherPreset.c` | the weather values (`JMWeatherPreset`), `Neutral()`, `Copy()`, `ClearDate()`, the `...Hi` twins |
| `JMWeatherSerialize.c` | `Weather.json`: `States`, `DynamicEnabled`; migrations, `NormalizeState`, `SaveState`, `RemoveState`, `GetNextOdds` |
| `JMWeatherRuntime.c` | `WeatherState.json`: where the machine is |
| `JMWeatherMachine.c` | server-only engine: timer, rolls, pause, adopt, resume |
| `JMWeatherDynamicConfig.c` / `Control.c` / `Status.c` | RPC payloads: edit / move+apply / read position |
| `Components/JMWeatherRangeControl.c`, `JMWeatherRangeToggle.c` | one value-or-span slider, and the title-bar switch (same as the object spawner's) |
| `Tabs/Presets/JMWeatherChanceList.c` | reusable "list that sums to 100" widget (used 3x; picking in its dropdown adds) |
| `Tabs/Presets/JMWeatherDynamicStatusCard.c` | "Current Preset": switch, position, a "time left / hold" bar, an odds table of the next steps; title-bar pause/play, fast-forward (advance), restart |
| `Tabs/Presets/JMWeatherDynamicEditor.c` | the preset editor (working copy, sent whole on Save) |

## The Dynamic Presets tab

Top to bottom: **Current Preset** (live status), **Presets**, **Next Presets**, **Phase**,
**Opens On**, **After This Phase**.

- Every action is an icon in the card's title bar. **Presets**: Add, Apply Now, Delete (asks
  first), Save - the Save icon turns green and pulses while there is anything unsaved (a new preset, a
  rename, a touched list or weather card). **Phase**: Add, Enter Now, Remove. Beside each selector is a
  pencil that opens a rename popup (`JMConfirmation` edit prompt; its answer arrives on the form as
  `RenamePreset_Confirm` / `RenamePhase_Confirm`).
- **Current Preset** card: the title-bar icons are pause/play (holds the countdown by hand, `OP_HOLD` / `OP_RELEASE`; not persisted, and independent of Freeze Time - either one keeps the phase held), fast-forward (advance) and restart. The bar reads `time left / hold length`; below it a table of the possible next steps, one bar per step filled to its odds, best first.
- Cards only show what is useful: with no preset selected (and so with none stored) only **Presets**
  shows; a chance list's add dropdown disappears when nothing is left to add, and its card with it when
  it also has no rows; **Opens On** needs two or more phases.
- **Picking a preset (or Add) IS editing it** - there is no Edit button. The weather tabs enter *preset
  edit mode*: Sky / Precipitation / Wind show and edit the weather of the phase selected in the Phase
  card, every card switches to min/max, the **Time tab is disabled** (a preset carries no date), Apply on
  those tabs is a local preview, and the banner offers Save / Close. Switching phase commits the tabs
  into the working copy first. Close (or picking "Select a preset...") un-selects and throws the edit
  away; Save (banner or title-bar icon) sends it. A settings refresh reloads the preset from the server
  only while nothing is unsaved, staying on the phase being viewed.
- Only sections that were **touched** are written back into the phase, so selecting and saving never
  turns the world's live values into settings, and a tab that was never opened cannot zero anything.
- **Apply Now** puts the saved preset's weather on with its own stored timing (RPC `DynamicControl`
  `OP_APPLY`, permission `Weather.Preset.Use`); the machine, if running, moves to that phase.
- Every weather card has the range switch in its title bar. Off by default for the live world (Apply
  then sends a plain value, or a span if switched on); on while editing a preset.
- Permissions: Add/Save-new `Weather.Preset.Create`, Save-existing `Weather.Preset.Update`, Delete
  `Weather.Preset.Remove`, Apply Now `Weather.Preset.Use`, the master switch and Advance/Restart/Jump
  `Weather.Preset.Dynamic`. A save from someone without the switch permission cannot flip it.

## The bundled flow

With no `Weather.json` the server writes six presets (`JMWeatherSerialize.Defaults`), all in rotation, the machine itself off. Each is a front of three phases - it builds, peaks (and may stay on the peak, 35%), then eases and leaves - with every value a min/max span, so no two visits play out the same.

| Preset | Phases | Goes on to |
|---|---|---|
| Clear and Sunny | Clearing, Sunny, Light Clouds | itself 40, Overcast and Fog 30, Frequent Rain 15, Frequent Snow 15 |
| Overcast and Fog | Clouding Over, Thick Fog, Fog Lifting | Clear 30, itself 20, Rain 20, Snow 20, Heavy Rain 5, Heavy Snow 5 |
| Frequent Rain | Drizzle, Steady Rain, Easing Rain | itself 30, Heavy Rain 20, Overcast 25, Clear 25 |
| Heavy Rain | Rain Building, Downpour (lightning), Front Passing | Frequent Rain 35, Overcast 25, Clear 25, itself 15 |
| Frequent Snow | Flurries, Steady Snow, Snow Easing | itself 30, Heavy Snow 20, Overcast 25, Clear 25 |
| Heavy Snow | Snow Building, Whiteout (wind, fog), Blizzard Easing | Frequent Snow 35, Overcast 25, Clear 25, itself 15 |

Rain and snow are set in every phase (to zero where unwanted) and the rain/snow overcast thresholds follow, so a phase never inherits the last one's precipitation. An existing `Weather.json` is never replaced - delete it (and `WeatherState.json`) to get these back.

## Persistence

- **Enabled** is configuration: `Weather.json` -> `DynamicEnabled`. Unchanged across restarts.
- **Position** is runtime: `$profile:CommunityOnlineTools\WeatherState.json`
  (`State`, `Phase`, `HoldTotal`, `HoldRemaining`). Written on every phase change, on enable/disable,
  on freeze/unfreeze, on a clean shutdown (`OnMissionFinish`) and by a 60 s heartbeat, so a crash loses
  at most 60 s of a hold. Kept out of `Weather.json` so a phase change does not rewrite every preset.
- **Boot**: enabled + a recorded preset/phase that still exist -> the phase's weather is re-applied at
  once (no fade) with `MinDuration = HoldRemaining` and the timer resumes with `HoldRemaining`.
  Anything else -> random entry. Turning the machine **off** wipes the record.
- Time is stored as seconds *left*, not a wall-clock deadline: downtime does not count against a phase.

## Interactions

- **Freeze Time** pauses the countdown; a phase entered while frozen comes up paused. Freeze state
  itself is not persisted (pre-existing).
- **Editing config** never restarts a phase that is still valid; only a deleted preset/phase or
  switching on re-enters.

## Migration

`StatesVersion` in `Weather.json`:

- `0` -> `MigrateDynamic` (older flat timing/edges -> presets) then `MigrateToStates`: each preset
  becomes a one-phase preset with its timing and next-preset list.
- `1` -> `MigrateToWeather`: each phase's referenced stored preset is **copied into the phase**
  (`Conditions`, date dropped) and the top-level `Presets` list is emptied. Two phases that named the
  same preset stop sharing it. `InitialPreset` now names a preset (state) and is cleared if it is gone.
- `2` = current. Legacy fields (`Presets`, `Preset`, per-preset chain block, `CurrentPreset`) stay
  declared so JsonFileLoader can still read old files.

## RPCs (`JMWeatherModuleRPC`)

- `DynamicWeather` - `JMWeatherDynamicConfig`: `Enabled` + `Op` (`OP_TOGGLE` / `OP_SAVE` /
  `OP_REMOVE`), `TargetName`, one `State`. The server validates and stores its own copy.
- `DynamicControl` - `JMWeatherDynamicControl`: `OP_ADVANCE` / `OP_RESTART` / `OP_JUMP` / `OP_APPLY` / `OP_HOLD` / `OP_RELEASE`.
- `DynamicStatus` - request/response, `Weather.View`. The form polls every 2 s; the countdown runs
  locally between polls. It carries a **revision** bumped on every config change: a client that sees a
  revision it did not load asks for the settings again.
- `UsePreset` / `CreatePreset` / `UpdatePreset` / `RemovePreset` ids remain in the enum (renumbering
  would shift every later id) but are no longer handled.

## Verifying a restart resumes

1. Set `DynamicEnabled` on and give a phase a hold of at least 3 minutes (floor is 60 s).
2. Start the server; the log shows `Dynamic weather moved to <Preset> > <Phase>` and
   `WeatherState.json` appears.
3. Wait past one heartbeat (60 s); `HoldRemaining` drops below `HoldTotal`.
4. Kill and start the server; the log shows `Dynamic weather resumed at <Preset> > <Phase>` and
   `HoldRemaining` continues from the snapshot rather than restarting.
5. Disable in the UI -> `WeatherState.json` is cleared; restart stays off.
