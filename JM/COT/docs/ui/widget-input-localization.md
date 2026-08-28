---
name: Widget Input and Localization
description: Layout XML widget types and positioning, UIActionSlider/SelectBox API, input action XML format, all input action names, stringtable localization format
type: project
---

## Layout File Widget Types

Widget classes available in .layout XML files:

| Widget class | Purpose |
|---|---|
| `PanelWidgetClass` | Opaque background container |
| `FrameWidgetClass` | Transparent container |
| `TextWidgetClass` | Static single-line text |
| `MultilineTextWidgetClass` | Wrapped multi-line text |
| `EditBoxWidgetClass` | Single-line text input |
| `MultilineEditBoxWidgetClass` | Multi-line text input |
| `ButtonWidgetClass` | Clickable button |
| `SliderWidget` | Slider control |
| `CheckBoxWidget` | Checkbox |
| `MapWidget` | In-game map display |
| `TextListboxWidget` | Scrollable text list |

### Positioning System

```xml
<FrameWidgetClass my_widget>
    <position> 10 5 </position>       <!-- X Y offset from parent -->
    <size> 0.5 30 </size>             <!-- W H (proportional or pixel) -->
    <halign> center_ref </halign>     <!-- left_ref | center_ref | right_ref -->
    <valign> top_ref </valign>        <!-- top_ref | center_ref | bottom_ref -->
    <hexactpos> 0 </hexactpos>        <!-- 0=proportional, 1=pixel X position -->
    <vexactpos> 0 </vexactpos>        <!-- 0=proportional, 1=pixel Y position -->
    <hexactsize> 0 </hexactsize>      <!-- 0=proportional, 1=pixel W size -->
    <vexactsize> 1 </vexactsize>      <!-- 0=proportional, 1=pixel H size -->
    <color> 0.12 0.16 0.22 1 </color> <!-- R G B A, 0.0-1.0 range -->
    <style> DayZDefaultPanel </style>
</FrameWidgetClass>
```

Common sizes:
- `size 1 1` = fill parent 100%
- `size 0.5 1` = half width, full height
- `size 350 130` = exact pixels (needs hexactsize/vexactsize=1)

Fonts: `"gui/fonts/sdf_MetronLight24"`, `"gui/fonts/sdf_MetronBold24"`

## UIActionSlider — Full API

```c
UIActionSlider slider = UIActionManager.CreateSlider( parent, "#STR_COT_LABEL", minVal, maxVal, this, "OnChange_X" );

// Configuration
slider.SetCurrent( 3.5 );             // Set value
slider.SetMinMax( 0.0, 100.0 );       // Change range
slider.SetStepValue( 0.1 );           // Snap increment
slider.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );  // Display format
slider.SetLabel( "Speed" );           // Change label text
slider.SetColor( ARGB(255, 200, 100, 50) );
slider.SetAlpha( 0.8 );
slider.SetSliderWidth( 0.7 );         // Fraction of row for slider bar
slider.SetWidgetWidth( slider.GetLabelWidget(), 0.3 );
slider.SetWidgetWidth( slider.GetSliderWidget(), 0.7 );

// Reading
float val = slider.GetCurrent();
float min = slider.GetMin();
float max = slider.GetMax();

// Event handler:
void OnChange_X( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE ) return;
    float value = action.GetCurrent();
}
```

## UIActionSelectBox — Full API

```c
// Create (options can be updated later)
UIActionSelectBox sel = UIActionManager.CreateSelectionBox( parent, "Label", new array<string>( {"Option A", "Option B"} ), this, "OnChange_X" );

// Configuration
sel.SetSelections( newOptions );       // Replace all options
sel.SetSelection( 0 );                 // Set selected index (no event)
sel.SetSelection( 0, false );          // Set without firing event
sel.SetSelectorWidth( 1.0 );

// Reading
int idx = sel.GetSelection();          // 0-based index

// Event handler:
void OnChange_X( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE ) return;
    int selected = action.GetSelection();
}
```

Internally uses `OptionSelectorMultistate`. The change event fires via `m_Selection.m_OptionChanged`.

## UIActionEditableText

```c
UIActionEditableText txt = UIActionManager.CreateEditableText( parent, "Label", this, "OnChange_X" );

// Reading
string value = txt.GetString();    // or action.GetString() in handler

// Setting
txt.SetString( "default text" );
txt.SetLabel( "New Label" );
```

## JMConfirmation Dialog

```c
// Types:
enum JMConfirmationType { INFO = 0, EDIT = 1, SELECTION = 2 }

// In a form method — show a Yes/No dialog:
CreateConfirmation_Two(
    JMConfirmationType.INFO,
    "Confirm Action",
    "Are you sure you want to delete this?",
    "#STR_COT_GENERIC_YES", "OnConfirm_Delete",
    "#STR_COT_GENERIC_NO",  ""           // empty = no action
);

void OnConfirm_Delete( JMConfirmation confirm )
{
    // user clicked Yes
    m_Module.DeleteSomething();
}

// EDIT type — dialog has a text input:
CreateConfirmation_Two(
    JMConfirmationType.EDIT,
    "Enter Reason", "",
    "OK", "OnConfirm_Reason",
    "Cancel", ""
);

void OnConfirm_Reason( JMConfirmation confirm )
{
    string reason = confirm.GetEditBoxValue();
}
```

Callback receives a `JMConfirmation` parameter. Use `confirm.GetEditBoxValue()` for EDIT type.

## Input Actions (Inputs.xml)

File: `JM/COT/Scripts/Data/Inputs.xml`

Defines input action names + default key bindings. Add new actions here when creating module shortcuts.

XML format:
```xml
<modded_inputs>
    <inputs>
        <actions>
            <input name="UAMyModuleAction" loc="STR_COT_INPUT_MY_MODULE_ACTION" />
        </actions>
        <sorting name="JM_COT" loc="COT">
            <input name="UAMyModuleAction" />
        </sorting>
    </inputs>
    <preset>
        <input name="UAMyModuleAction">
            <btn name="kF5" />               <!-- single key -->
        </input>
        <!-- Modifier combo: -->
        <input name="UACOTTogglePlayer">
            <btn name="kRMenu"><btn name="kT" /></btn>   <!-- RightAlt+T -->
        </input>
    </preset>
</modded_inputs>
```

### All Existing Input Actions

| Action | Default Key | Purpose |
|--------|------------|---------|
| `UACOTModuleToggleCOT` | End | Toggle COT active |
| `UACOTToggleButtons` | Y | Toggle sidebar |
| `UACOTTogglePlayer` | RAlt+T | Player module |
| `UACOTToggleCamera` | RAlt+Y | Camera module |
| `UACOTToggleESP` | RAlt+U | ESP module |
| `UACOTToggleMap` | RAlt+I | Map module |
| `UACOTToggleTeleport` | RAlt+J | Teleport module |
| `UACOTToggleLoadout` | RAlt+K | Loadout module |
| `UACOTToggleVehicle` | RAlt+L | Vehicles module |
| `UACOTToggleWeather` | RAlt+O | Weather module |
| `UACameraToolToggleCamera` | Insert | Enter/exit freecam |
| `UACameraToolZoomForwards` | Ctrl+ScrollUp | Camera zoom |
| `UACameraToolZoomBackwards` | Ctrl+ScrollDown | Camera zoom |
| `UATeleportModuleTeleportCursor` | H | Cursor teleport |
| `UAPlayerModuleGodMode` | U | Toggle godmode |
| `UAPlayerModuleStopSpectating` | Numpad1 | Exit spectate |
| `UAObjectModuleDeleteOnCursor` | Delete | Delete at cursor |
| `UAObjectModuleSpawnInfected` | (unbound) | Spawn infected |

Key name constants: `kEnd`, `kInsert`, `kDelete`, `kF1`-`kF12`, `kT`-`kZ`, `kLControl`, `kRMenu` (RAlt), `kLMenu` (LAlt), `kLShift`, `mWheelUp`, `mWheelDown`, `kNumPad1`-`kNumPad9`

## Localization System

File: `JM/COT/languagecore/stringtable.csv`

CSV format — 14 languages:
```
KEY, original, english, czech, german, russian, polish, hungarian, italian, spanish, french, chinese(trad), japanese, portuguese, chinese(simp)
```

Usage in code — prefix key with `#`:
```c
UIActionManager.CreateSlider( parent, "#STR_COT_CAMERA_MODULE_BLUR", ... );
UIActionManager.CreateButton( parent, "#STR_COT_GENERIC_REFRESH", ... );
new StringLocaliser( "#STR_COT_NOTIFICATION_KEY" )
```

Naming convention: `STR_COT_[MODULE]_[DESCRIPTION]`
Examples:
- `STR_COT_CAMERA_MODULE_NAME` → "Camera Tools"
- `STR_COT_GENERIC_YES` / `STR_COT_GENERIC_NO` / `STR_COT_GENERIC_REFRESH`
- `STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_GODMODE` → "God Mode"
- `STR_COT_FORMAT_PERCENTAGE` → display format for percentage sliders
- `STR_COT_FORMAT_METRE` → display format for distance sliders
