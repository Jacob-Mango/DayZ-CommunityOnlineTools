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

Extends `COT_ScriptedWidgetEventHandler`. Windows are created and tracked by `JMWindowManager` (`GetCOTWindowManager()`:
`Create()`, `BringFront()`, `HasAnyActive()` / `HasAnyUnpinnedActive()`, `DestroyAllWindows()`).

**Form Embedding Flow:**
```c
// Inside JMWindowBase.SetModule(module):
Widget menu = g_Game.GetWorkspace().CreateWidgets( m_Module.GetLayoutRoot(), m_ContentWidget );   // size clamped to the screen
menu.GetScript( m_Form );                       // retrieve form from the layout's scriptclass
                                                // (m_Module.InitForm( menu ) is the fallback when the layout has none)
m_Form.Init( this, m_Module );                  // SetModule -> OnCreate -> OnClientPermissionsUpdated -> OnShow
m_TitleText.SetText( m_Module.GetTitle() );
GetCOTWindowManager().BringFront( this );
m_Form.OnResize( winW, winH - m_TitleBarHeight );   // initial size, so the form can lay out immediately
```

**Drag:** `OnDrag/Dragging/Drop` on `m_TitleWrapper`. Offset stored in `m_OffsetX/Y`, position set with `SetPosition(x - m_OffsetX, y - m_OffsetY)`.

**Resize:** Eight drag zones - four edges (`m_ResizeDragUp/Down/Left/Right`) and four corners
(`m_ResizeDragTopLeft/TopRight/BotLeft/BotRight`), each with a highlight widget. Start size/position are stored before the resize, and
`m_Form.OnResize( w, h )` is called.

**Pin and minimize:** `TogglePin()` keeps a window open when the sidebar closes (`IsPinned()`); `Minimize()` / `Restore()` /
`ToggleMinimize()` collapse it to its title bar (`IsMinimized()`). `HasOpenPopup()` / `CloseOpenPopup()` are used by the Escape chain.
The window also owns a confirmation panel (`CreateConfirmation_One/Two/Three`), which forms reach through `JMFormBase`.

**Z-Ordering:** On any click inside a window, calls `GetCOTWindowManager().BringFront(this)`.

**Lifecycle:** `Show()` adds to the GUI update queue and changes game focus; `Hide()` removes it. The destructor unlinks the layout root
(required for DayZ 1.28+). `JMRenderableModuleBase.OnWindowDestroyed()` clears the module's reference when the window goes.

A `JMFormBase` can also pin one tab strip over its scrolling content (`JMPinnedStrip`), so the strip stays visible while the panel scrolls.

## JMCOTSideBar — Left Sidebar

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c` (categories in `JMCOTSideBarCategory.c`,
status strip in `JMCOTSideBarFooter.c`, category order/icons in `JMSideBarConfig.c`)

### Animation
```c
float m_WidthFull = 300;                                   // expanded width, read from the layout root
float m_WidthIcon = m_WidthFull * 0.8;                     // compact width
float m_TotalAnimateTime = JMUIAnimations.SIDEBAR_ANIMATE_TIME;   // 0.35 s

// Easing functions (Lerp between min and max):
float EaseIn(float percent, float max, float min)  { return Math.Lerp( min, max, Math.Sin( percent * Math.PI / 2 ) ); }
float EaseOut(float percent, float max, float min) { return Math.Lerp( min, max, 1 - Math.Cos( percent * Math.PI / 2 ) ); }
```
Expands to full when no module windows are open; collapses to the compact rail when any window is visible
(`CheckForVisibleModules()`). Interrupting an animation resumes from the current offset instead of jumping.

### Module Button Setup
`JMCOTSideBar.Init()` sorts `GetModuleManager().GetCOTModules()`, groups them by `GetCategory()` in the order of
`JMSideBarConfig.GetCategoryOrder()` (Players, World, Items, Vehicles, Events, Server, Expansion, Other; an unknown category is
appended alphabetically with the settings icon), and builds one `JMCOTSideBarCategory` per group. Each category owns the tiles:

```c
// JMCOTSideBarCategory.AddModule( module ) - only for modules with HasButton() == true
module.InitButton( btnWidget );     // sets icon/label/colour; requires the inner ButtonWidget "btn" to exist
```
The category's tile container sits inside the `Buttons_Scroller` / `WrapSpacerWidget "Buttons"` of `sidebar_menu.layout`; a category
can open a flyout (`sidebar_flyout.layout`) of its modules.

### Click Handling
`JMCOTSideBarCategory.OnClick( Widget w, ... )` maps the clicked tile (the inner `btn` ButtonWidget, or the `btn_bkg` root) back to its
module and calls `module.ToggleShow()`.

### Title Behavior
- Compact mode: shows "COT"
- Expanded mode: shows "Community Online Tools"

### Footer
`JMCOTSideBarFooter` is a plain helper (not a widget scriptclass) pinned to the bottom (`HEIGHT = 130` px, must match the `Footer` panel
in the layout). Once a second it refreshes: server name, player count, in-game clock, weather and where it is heading (from the
dynamic-weather machine), server performance (FPS) and the COT build, plus an Anti-Cheat status button. It has a full layout and a
compact layout, swapped with the sidebar mode.

## Sidebar Layout Structure

File: `JM/COT/GUI/layouts/sidebar_menu.layout`

```
PanelWidgetClass JMCOTSideBar (scriptclass "JMCOTSideBar", width 300, style COTSurface)
├── TitleBar (4% height, top) — TitleBarText "Community Online Tools", TitleBarRule
├── Buttons_Scroller (between the title bar and the footer) — scrollable
│   └── WrapSpacerWidget "Buttons" — category / module tiles go here
└── Footer (130 px, bottom) — FooterFull + FooterCompact status strips
```

Positioned right_ref, top_ref, priority 100. Surface colour `0.082 0.098 0.118 0.941`. The title bar fraction and the footer height
are duplicated in script constants (`TITLEBAR_HEIGHT_FRAC`, `JMCOTSideBarFooter.HEIGHT`) and must be kept in step with the layout.
Credits and version text are no longer separate widgets of this layout.

## CommunityOnlineToolsBase — Lifecycle

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/CommunityOnlineToolsBase.c`

Global accessor: `CommunityOnlineToolsBase GetCommunityOnlineToolsBase()` via `g_cotBase`.

Key lifecycle:
```c
void OnStart()
{
    // m_IsOpen = false; invoke COT_ON_OPEN(false)
    // server + multiplayer: GetPermissionsManager().LoadRoles()
    // CreateNewLog() (timestamped log file)
    // find the webhook module (none when offline)
}

void OnFinish()   { /* m_IsOpen = false, invoke COT_ON_OPEN(false), close log */ }
void OnUpdate( float timeslice )
{
    // Monitors GetDayZGame().IsLoading()
    // Once done → calls OnLoaded() once
}
void ToggleActive() / SetActive( bool ) → OnCOTActiveChanged( bool )
```

Useful utility methods (mostly `static`):
```c
static void HealEntityRecursive( Object obj, bool includeAttachments = true, bool includeCargo = true )
static void Refuel( Object obj )                                 // cars, boats, HypeTrain
static void PlaceOnSurfaceAtPosition( EntityAI e, vector pos, bool aboveWater = true )   // raycast surface align
static void ForceTransportPositionAndOrientation( Transport t, vector pos, vector ori )  // sync vehicle + preserve velocity
static void ForceDisableInputs( bool state, inout TIntArray skipIDs = null )             // disable inputs except the skipped ids
void GetHeadTransform( Object obj, out vector transform[4], bool includeOffset = false ) // bone transform
static bool IsValidWorldPosition( vector pos )                   // NaN / range / map-bounds check for client-sent positions
```

Active state per-player (server-side `m_ActiveGUIDs` map). `SetActive(bool)` triggers `OnCOTActiveChanged()`.

## Modded Classes (79 declarations)

COT declares 79 `modded class` blocks: vanilla DayZ classes, Community Framework classes (`CF_Windows`, `CF_InputBindings`,
`CF_Permission_PlayerBase`, `JMModuleManager`, `JMModuleConstructorBase`), Expansion classes (in `4_World`, see
[../systems/mod-compatibility.md](../systems/mod-compatibility.md)) and a few of its own (`JMModuleBase`, `JMModuleConstructor`).
Key vanilla ones:

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
| `MissionBaseWorld` | Holds the freecam / select-player suppression flag (`COT_TempDisableOnSelectPlayer`) shared by client and server missions |
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

**UIActionCheckbox** (an empty subclass of `UIActionToggle`; it draws the custom toggle visuals, not a vanilla `CheckBoxWidget`):
```c
cb.SetLabel( "God Mode" );         // UIActionBase
cb.SetChecked( true );             // UIActionBase; SetCheckedSilent( bool ) does not fire the callback
bool on = cb.IsChecked();
// Both the box AND the label button trigger the click event
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
txt.SetLabelHAlign( UIActionHAlign.RIGHT );
txt.SetTextHAlign( UIActionHAlign.LEFT );
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
    string value = action.GetText();    // or txt.GetText()
}
```

**UIActionEditableTextPreview** — same API plus a semi-transparent preview/autocomplete box. Tab or Enter accepts the preview text.
