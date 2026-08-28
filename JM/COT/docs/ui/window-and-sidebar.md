---
name: Window and Sidebar System
description: JMWindowBase drag/resize/form embedding, JMCOTSideBar animation/button layout, COT_ScriptedWidgetEventHandler, CommunityOnlineToolsBase lifecycle, modded vanilla classes
type: project
---

## COT_ScriptedWidgetEventHandler

File: `JM/COT/Scripts/3_Game/CommunityOnlineTools/COT_ScriptedWidgetEventHandler.c`

**Not an alias** — actual subclass of `ScriptedWidgetEventHandler`. Adds `DestroyLater()` and `DestroyWidget()` safety methods. Base class for all COT GUI components: `JMWindowBase`, `JMFormBase`, `JMCOTSideBar`, all `UIActionBase` subclasses.

## JMWindowBase — Window Frame

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMWindowBase.c`

Extends `COT_ScriptedWidgetEventHandler`.

**Form Embedding Flow:**
```c
// Inside JMWindowBase.SetModule(module):
Widget content = g_Game.GetWorkspace().CreateWidgets( module.GetLayoutRoot(), m_ContentWidget );
JMFormBase form;
content.GetScript( form );       // retrieve form from widget's scriptclass
form.Init( this, module );        // initialize: SetModule → OnInit → OnClientPermissionsUpdated → OnShow
```

**Drag:** `OnDrag/Dragging/Drop` on `m_TitleWrapper`. Offset stored in `m_OffsetX/Y`, position set with `SetPosition(x - m_OffsetX, y - m_OffsetY)`.

**Resize:** Four drag zones (`m_ResizeDragUp/Down/Left/Right`). Stores start size/position before resize.

**Z-Ordering:** On any click inside a window, calls `GetCOTWindowManager().BringFront(this)`.

**Lifecycle:** `Show()` adds to GUI update queue + changes game focus. `Hide()` removes. Destructor unlinks layout root (required for DayZ 1.28+).

## JMCOTSideBar — Left Sidebar

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c`

### Animation
```c
float m_WidthFull = 300;          // expanded width
float m_WidthIcon;                 // compact width (≈80% of full)
float m_TotalAnimateTime = 0.35;  // 350ms

// Easing functions:
float EaseIn(float t)  { return Math.Sin( t * Math.PI / 2 ); }
float EaseOut(float t) { return 1 - Math.Cos( t * Math.PI / 2 ); }
```
Expands to full when no module windows are open; collapses to icon-only when any window is visible.

### Module Button Setup
```c
void Init()
{
    // Get all renderable modules:
    array<JMRenderableModuleBase> modules = GetModuleManager().GetCOTModules();

    // Sort alphabetically by module name
    SortModuleArray( modules );

    // Create button for each module with HasButton() == true:
    foreach ( JMRenderableModuleBase module : modules )
    {
        if ( !module.HasButton() ) continue;
        Widget btn = g_Game.GetWorkspace().CreateWidgets( "sidebar_button.layout",
            layoutRoot.FindAnyWidget( "Buttons" ) );
        module.InitButton( btn );  // sets icon/label/color
    }
}
```
Buttons inserted into `WrapSpacerWidget "Buttons"` — auto-wraps vertically.

### Click Handling
```c
override bool OnClick( Widget w, ... )
{
    foreach ( JMRenderableModuleBase module : GetModuleManager().GetCOTModules() )
    {
        if ( w == module.GetMenuButton() )
        {
            module.ToggleShow();
            return true;
        }
    }
    return false;
}
```

### Title Behavior
- Compact mode: shows "COT"
- Expanded mode: shows "Community Online Tools"

## Sidebar Layout Structure

File: `JM/COT/GUI/layouts/sidebar_menu.layout`

```
PanelWidgetClass JMCOTSideBar (scriptclass "JMCOTSideBar")
├── TitleBar (4% height, top) — "Community Online Tools"
├── Buttons_Scroller (4%–91% height) — scrollable
│   └── WrapSpacerWidget "Buttons" — module buttons go here
├── CreditsText (bottom) — "Created by Jacob_Mango, Arkensor and DannyDog"
└── Version_Text — version string
```

Positioned right_ref, top_ref. Dark blue background (0.03, 0.04, 0.06, 0.9).

## CommunityOnlineToolsBase — Lifecycle

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/CommunityOnlineToolsBase.c`

Global accessor: `CommunityOnlineToolsBase GetCommunityOnlineToolsBase()` via `g_cotBase`.

Key lifecycle:
```c
void OnStart()
{
    // Invoke COT_ON_OPEN(false)
    // Create timestamped log file
    // Init webhook module
}

void OnFinish()   { /* close log */ }
void OnUpdate( float timeslice )
{
    // Monitors g_Game.IsLoading()
    // Once done → calls OnLoaded() once
}
```

Useful utility methods:
```c
void HealEntityRecursive( EntityAI entity )        // heal + all attachments/cargo
void Refuel( EntityAI vehicle )                    // cars, boats, HypeTrain
void PlaceOnSurfaceAtPosition( EntityAI e, vector pos ) // raycast surface align
void ForceTransportPositionAndOrientation( ... )   // sync vehicle + preserve velocity
void ForceDisableInputs()                          // disable all inputs except UI nav
vector GetHeadTransform( EntityAI entity )         // bone pos + 0.12m head offset
```

Active state per-player (server-side `m_ActiveGUIDs` map). `SetActive(bool)` triggers `OnCOTActiveChanged()`.

## Modded Vanilla Classes (69 total)

COT mods 69 DayZ base classes. Key ones:

| Modded class | Purpose |
|---|---|
| `PlayerBase` | Adds godmode, spectate, invisibility, COT flags |
| `DayZPlayerImplement` | Player implementation extensions |
| `DayZPlayerCameraBase` | Camera enhancements |
| `StaminaHandler` | Unlimited stamina support |
| `ItemBase` | Base item extensions |
| `Weapon_Base` | Weapon admin features |
| `Construction` | Base building system hooks |
| `ActionBase` / `AnimatedActionBase` | User action system hooks |
| `CarScript` / `BoatScript` | Vehicle admin features |
| `DayZGame` | Game lifecycle hooks |
| `MissionServer` | Ban checks, COT init, spectator sync |
| `MissionGameplay` | Client COT init |
| `MissionMainMenu` | Main menu integration |
| `PluginAdminLog` | Webhook for deaths, damage, placement |
| `PluginManager` / `PluginDeveloper` | Plugin system hooks |
| `AnimalBase` / `ZombieBase` | Creature modding |
| `ModifiersManager` / `Drowning` | Modifier/status effects |
| `Environment` | Environmental effects hooks |

## UIActionButton / UIActionCheckbox / UIActionButtonToggle

**UIActionButton:**
```c
btn.SetButton( "Label" );       // set text
btn.GetButton();                // get text
btn.SetColor( ARGB(...) );      // tint
// OnClick fires UIEvent.CLICK
```

**UIActionCheckbox:**
```c
cb.SetLabel( "God Mode" );
cb.SetChecked( true );
bool on = cb.IsChecked();
// Both the checkbox widget AND label button trigger click event
```

**UIActionButtonToggle:**
```c
tog.SetButtonToggle( "Enable", "Disable" );  // off text, on text
bool on = tog.IsToggled();
tog.SetToggle( true );
// Automatically swaps displayed text on click
```

## UIActionText / UIActionImage

**UIActionText:**
```c
txt.SetLabel( "Position:" );
txt.SetText( "12345.0" );         // or pass float
txt.SetText( 3.14 );
// Default OnClick (no callback): copies text to clipboard
// With callback: fires UIEvent.CLICK
txt.SetLabelHAlign( TextWidget.ALIGN_RIGHT );
txt.SetTextHAlign( TextWidget.ALIGN_LEFT );
```

**UIActionImage:**
```c
img.SetImage( "path/to/image.edds" );
// No interaction, display only
```

## UIActionEditableText — Full API

```c
UIActionEditableText txt = UIActionManager.CreateEditableText( parent, "Label", this, "OnChange_X" );

// Configuration
txt.SetOnlyNumbers( true, false );  // numbers only, allow decimals
txt.SetOnlyNumbers( true, true );   // integers only
txt.SetEditBoxWidth( 0.6 );         // fraction of row for input
txt.SetText( 42.5 );                // set value
txt.SetText( "hello" );
txt.SetLabel( "New Label" );

// Reading
string val = txt.GetText();

// Mouse wheel increments numeric value (Shift = ×10)

// Event handler (fires after 100ms debounce):
void OnChange_X( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE ) return;
    string value = action.GetString();  // or txt.GetText()
}
```

**UIActionEditableTextPreview** — same API plus a semi-transparent preview/autocomplete box. Tab or Enter accepts the preview text.
