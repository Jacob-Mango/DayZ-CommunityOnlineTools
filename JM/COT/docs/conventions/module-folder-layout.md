# Module folder layout

Where files go inside a module folder under `Scripts/5_Mission/CommunityOnlineTools/modules/` (and the
form-bearing folder `gui/EntityManager/`). The layout is applied and kept by
`Workbench/Batchfiles/cleanup/organize_tabs.py`; this page is the rule it implements. For how a form was
taken apart in the first place see [cleanup-playbook.md](cleanup-playbook.md).

## The shape

```
modules/<Module>/
    JM<Module>Module.c            the module
    JM<Module>Form.c              the form
    JM<Module>Serialize.c         RPC / JSON payloads, Enums.c, plain data types
    Components/                   parts the form, the module or several tabs are built from
    Tabs/
        <TabName>/
            JM<Module>FormTab<TabName>.c    the tab class
            <helper classes only this tab uses>
            Components/                     components only this tab uses (when there are any)
```

Real examples:

```
modules/Player/
    JMPlayerForm.c  JMPlayerModule.c
    Components/  JMPlayerRoster.c  JMPlayerRosterFilter.c  JMPlayerRowMenu.c  JMPlayerRowWidget.c
                 JMPermissionRowWidget.c  JMRoleRowWidget.c
                 JMPlayerStat.c  JMPlayerStats.c  JMPlayerToggle.c  JMPlayerToggles.c
    Tabs/General/     JMPlayerFormTabGeneral.c  JMPlayerStatRow.c
    Tabs/Inventory/   JMPlayerFormTabInventory.c
    Tabs/Actions/  Tabs/Position/  Tabs/Statistics/

modules/Weather/
    JMWeatherForm.c  JMWeatherModule.c  JMWeatherSerialize.c
    JMWeatherPreset.c  JMWeatherNextState.c  JMWeatherTransition.c  JMWeatherDynamicConfig.c   (data types)
    JMWeatherMachine.c  JMWeatherPhase.c  JMWeatherState.c  JMWeatherRoll.c  JMWeatherRuntime.c   (dynamic state machine)
    JMWeatherDynamicControl.c  JMWeatherDynamicStatus.c                                          (dynamic RPC payloads)
    Components/  JMWeatherModeBanner.c  JMWeatherPresetWidget.c  JMWeatherRangeControl.c  JMWeatherRangeToggle.c
    Tabs/Overview/  Tabs/Sky/  Tabs/Precipitation/  Tabs/Wind/  Tabs/Time/
    Tabs/Presets/  JMWeatherFormTabPresets.c  JMWeatherChanceList.c  JMWeatherDynamicEditor.c  JMWeatherDynamicStatusCard.c

modules/Camera/                     a form with no tabs: no Tabs/ folder
    JMCameraForm.c  JMCameraModule.c  JMCameraSerialize.c
    Components/  JMCameraTravelPanel.c
```

A folder is only created when it has something to hold: no tabs, no `Tabs/`; no components, no `Components/`.

## What goes where

| Kind | Rule | Goes in |
|---|---|---|
| Module, form | files named `*Module.c` / `*Form.c` | module folder |
| Tab | file named `<Prefix>FormTab<Name>.c` | `Tabs/<Name>/` |
| Tab helper | a class every mention of which (outside its own file) is in that tab's group; repeated until stable, so a helper of a helper follows | `Tabs/<Name>/` |
| Component | see below | `Components/`, or `Tabs/<Name>/Components/` when only that tab uses it |
| Data type, payload, enum | everything else that is not shared UI: presets, history/location records, `*Serialize.c`, `Enums.c` | module folder |
| Shared by other modules | mentioned by another module, `ExampleScriptOverride` or `AutoTest` | stays where it is |

A **component** is a part the form, the module or a tab is built from. It is any of:

1. **A back-reference class.** A plain class with a field `m_Form`, `m_Tab`, `m_Owner` or `m_Parent` typed as a
   `...Form...` or `...Tab...` class (`JMPlayerRoster`, `JMTeleportFilter`, `JMVehiclesTypeFilter`, ...).
2. **A UI class.** It extends `UIActionBase`, `UIActionData`, `ScriptedWidgetEventHandler`,
   `COT_ScriptedWidgetEventHandler` or `COT_WidgetHolder`, or it builds widgets itself (three or more mentions of
   `Widget` / `UIAction*`): row widgets, popups, hover panels, cell data (`JMPlayerRowWidget`,
   `JMESPTransformPopup`, `JMVehiclesHoverInfo`, `JMLoadoutButtonData`).
3. **A class family.** A base class and its subclasses, all kept in one module folder: `JMPlayerStat` with the
   built-in stats, `JMPlayerToggle` with the built-in toggles, `JMActionHistoryEntry` with every history entry.

A component is never a tab, a form, a module, or a `UIScriptedMenu` (a menu is a screen of its own).

## What is not touched

- **`modded class` files.** Modded classes are applied in load order, which follows the path; moving them could
  change the result.
- **Files with no class** (constants, functions).
- **4_World and 3_Game.** Framework classes (`JMFormBase`, `JMWindowBase`, `JMRenderableModuleBase`, the
  `UIAction*` widgets under `gui/Actions/`) are not module code.
- **Layouts (`GUI/layouts`) and stringtable keys.** They live in other addons and are found by path or key, not by
  script folder.
- **Folders that are already curated**, such as `ItemStats/Types/`.

## Rules for new code

- A new tab goes in `Tabs/<Name>/` from the start, named `<Prefix>FormTab<Name>.c`. A helper only that tab uses
  goes beside it; a second user promotes it to `Components/` (or the module folder if it is a data type).
- Extracting a piece out of a form (see the playbook, 3.2) makes a component: put it in `Components/`, next to
  the form.
- Do not add an `#include`-style path anywhere. Enforce Script finds every file under the layer folder, so
  moving a file changes nothing for callers.

## Running it

```
python Workbench/Batchfiles/cleanup/organize_tabs.py            # dry run: prints what would move
python Workbench/Batchfiles/cleanup/organize_tabs.py --apply    # moves files (git mv for tracked ones)
```

It scans `Scripts`, `AutoTest` and `ExampleScriptOverride`, recomputes the whole plan from the current tree, and
does nothing when the layout is already right, so it is safe to re-run after adding code. Then build:
Enforce loads a layer's files in path order, and a full build is the only check that a base class still comes
before its subclasses. Use the fresh-log procedure in the playbook (section 5), not a cached verdict.

## History

Four passes, each built and booted clean: 31 files into 28 tab folders; 16 files into `Components/` (back-reference
classes); 4 files (stat and toggle families); then 32 files (UI classes, class families, in every other module).
`modules/Player/` went from 18 files in one folder to 2 files at the top, a `Components/` folder and five tab folders.

## Known oddities left alone

- `JMPlayerStats.c` and `JMPlayerToggles.c` hold the built-in subclasses (`JMPlayerStatHealth`,
  `JMPlayerToggleGodMode`, ...), not classes of those names. `JMPlayerStats.c` is also easy to confuse with the
  unrelated session-history class `JMPlayerStats` in `4_World/.../PermissionsOld/`.
- `JMWeatherPreset.c` declares `JMWeatherBase`, `JMtemStatsModule.c` is missing an `I`. Renames are separate work.
