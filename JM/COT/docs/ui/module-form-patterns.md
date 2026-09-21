# Form Design & Layout Guidelines: Tabs, Panels, and Controls

This document establishes the official design pattern and layout guidelines for module forms in
Community Online Tools (COT), derived from the architecture of `JMPlayerForm`.

This is the guide for creating and maintaining module forms and their scripts. Sections 1-11 are the form
machinery; 12-15 are how a form is kept maintainable (tab classes, components, folders, code style, verification).

Everything described here is implemented. The shared machinery lives on `JMFormBase`
(`Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMFormBase.c`) and every module form inherits it —
**do not reimplement or redeclare it in a subclass.**

---

## 1. Overview & Architectural Principles

Module forms in COT follow five core design principles:

1. **Flexible Modular Panel Architecture**: Forms choose the structural panel layout suited to their workflow:
   - **Left / Right Split**: Sidebar roster/navigation on Left; Detailed controls and tabs on Right.
   - **Top / Bottom Split**: Fixed filter/toolbar header on Top; Data table/grid content on Bottom.
   - **Full Pane / Custom**: Single content frame with optional header/footer bars.
2. **Minimal Layout XML, Programmatic Scripting**: The XML `.layout` file defines only high-level structural containers (`panel_left`, `panel_right`, `panel_top`, `panel_bottom`, `panel_right_tabs`, `panel_right_content`, `panel_right_disable`). All UI controls, grids, buttons, and cards are created programmatically in Enforce Script via `UIActionManager`.
3. **Lazy / Deferred Tab Construction**: Tab containers are registered upfront, but child cards and widgets are instantiated lazily on first tab activation to eliminate initial open latency and conserve memory.
4. **Centralized Floating Overlay Management**: Context menus, dropdowns, prompts, and tooltips are registered to an overlay registry so `OnChange_Tab()` and `OnHide()` automatically dismiss all open floating popups without hardcoded manual teardowns.
5. **Declarative Permission Gating**: Controls register permission keys directly, automating state updates in `OnClientPermissionsUpdated()`.

---

## 2. Flexible Layout File Structure (`.layout`)

Forms select between **Split-Pane (Left/Right)**, **Top/Bottom**, or **Four-Pane** panel configurations depending on their data presentation needs.

### Option A: Left / Right Split-Pane Layout (e.g. Roster + Tabbed Details)
```xml
PanelWidgetClass JMMySplitForm {
 color 0 0 0 0
 position 0 0
 size 800 600
 hexactpos 0
 vexactpos 0
 hexactsize 1
 vexactsize 1
 scriptclass "JMMySplitForm"
 style COTSurface
 {
  PanelWidgetClass panel_left {
   color 0 0 0 0
   position 0 0
   size 0.3 1
   halign left_ref
   hexactpos 0
   vexactpos 0
   hexactsize 0
   vexactsize 0
   {
    FrameWidgetClass panel_left_top {
     ignorepointer 1
     position 0 0
     size 1 0.15
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
    FrameWidgetClass panel_left_bottom {
     ignorepointer 1
     position 0 0.15
     size 1 0.85
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
   }
  }
  PanelWidgetClass panel_split {
   color 0 0 0 1
   position 0.305 0
   size 3 0.98
   halign left_ref
   valign center_ref
   hexactpos 0
   vexactpos 0
   hexactsize 1
   vexactsize 0
   style COTSurface
  }
  PanelWidgetClass panel_right {
   color 0 0 0 0
   position 0.31 0
   size 0.69 1
   halign left_ref
   hexactpos 0
   vexactpos 0
   hexactsize 0
   vexactsize 0
   {
    FrameWidgetClass panel_right_tabs {
     ignorepointer 0
     position 0 0
     size 1 0.06
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
    FrameWidgetClass panel_right_content {
     ignorepointer 0
     position 0 0.06
     size 1 0.94
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
   }
  }
  PanelWidgetClass panel_right_disable {
   visible 0
   color 0.098 0.118 0.141 0.985
   position 0.305 0
   size 0.695 1
   halign left_ref
   hexactpos 0
   vexactpos 0
   hexactsize 0
   vexactsize 0
   style COTSurface
  }
 }
}
```

### Option B: Top / Bottom Split Layout (e.g. Filters/Toolbars Top + Data Grid Bottom)
```xml
PanelWidgetClass JMMyTopBottomForm {
 color 0 0 0 0
 position 0 0
 size 800 600
 hexactpos 0
 vexactpos 0
 hexactsize 1
 vexactsize 1
 scriptclass "JMMyTopBottomForm"
 style COTSurface
 {
  PanelWidgetClass panel_top {
   color 0 0 0 0
   position 0 0
   size 1 0.12
   hexactpos 0
   vexactpos 0
   hexactsize 0
   vexactsize 0
  }
  PanelWidgetClass panel_bottom {
   color 0 0 0 0
   position 0 0.12
   size 1 0.88
   hexactpos 0
   vexactpos 0
   hexactsize 0
   vexactsize 0
   {
    FrameWidgetClass panel_bottom_tabs {
     ignorepointer 0
     position 0 0
     size 1 0.06
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
    FrameWidgetClass panel_bottom_content {
     ignorepointer 0
     position 0 0.06
     size 1 0.94
     hexactpos 0
     vexactpos 0
     hexactsize 0
     vexactsize 0
    }
   }
  }
 }
}
```

### Modular Panel Selection Summary:

| Form Type | Primary Panels Used | Typical Use Case |
|---|---|---|
| **Split-Pane (Left/Right)** | `panel_left`, `panel_split`, `panel_right`, `panel_right_disable` | Roster navigation (Player Manager, Vehicle Manager, Territory Manager). |
| **Top/Bottom** | `panel_top`, `panel_bottom` | Filter/Search Header + Large Data Grid (ESP Manager, Item Stats, Loot Analysis). |
| **Full Pane / Tabbed** | `panel_right` (or root `panel`), `panel_right_tabs` | Single-page forms with section tabs (Weather Manager, Server Info). |

---

## 3. Design Tokens

The sizing tokens are `static const` members of `JMFormBase`. Every form inherits them.

```c
	//! Height of header controls - search box, filter dropdowns, sort toggle.
	static const float HEADER_CONTROL_HEIGHT = 24;

	//! Drawn height of a tab strip.
	static const int TAB_STRIP_HEIGHT = 34;

	//! Side length of icon-only buttons in toolbars.
	static const int ICON_BUTTON_PX = 32;

	//! Side length of action buttons living in card title bars.
	static const int HEADER_ACTION_PX = 28;

	//! Fixed row height for identity/property rows.
	static const int IDENTITY_ROW_HEIGHT = 32;
```

> **Do not redeclare these in a module form.** A same-named constant in a derived class is a
> *redefinition*, not an override, and Enforce rejects the script module with
> `Multiple declaration of variable`. Use them directly; they are in scope.

The reasoning behind each value is documented at the declaration in `JMFormBase.c` — read it before
changing one, since several are pinned to a matching height inside a `.layout`.

---

## 4. Script Architecture & Form Lifecycle

A module form extends `JMFormBase` and inherits the panel members, the registries, the geometry
pinning, and the tab scaffold. It supplies only the parts that are form-specific.

```c
class JMMyModuleForm: JMFormBase
{
    //! protected, not private - see section 10.
    protected JMMyModule m_Module;

    protected UIActionTabs m_Tabs;
    protected UIActionScroller m_ActionListScroller;
    protected Widget m_ActionsWrapper;

    //! m_LeftPanel, m_RightPanel, m_RightPanelDisable, m_RightTabStrip and
    //! m_RightContent are inherited from JMFormBase. Resolve them in OnCreate();
    //! do not redeclare them.

    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    override void OnCreate()
    {
        InitWidgetsLeft();
        InitWidgetsRight();
    }

    override void OnResize( float w, float h )
    {
        PinRightPanelGeometry( h );

        if ( m_ActionListScroller )
            m_ActionListScroller.UpdateScroller();
    }

    override void OnHide()
    {
        //! super.OnHide() calls CloseAllOverlays() for you.
        super.OnHide();
    }
}
```

### Geometry Pinning (`OnResize`)

`JMFormBase` provides both halves, and every tabbed form places its tabs the same way the Player form
does: resolve the strip and the content panel into the base's members in `OnCreate()`, then call one
pin method from `OnResize()`.

```c
	//! Split-pane archetype: m_RightTabStrip over m_RightContent.
	protected void PinRightPanelGeometry( float contentHeight, Widget band = null, float bandHeight = 0 );

	//! Top/bottom archetype: m_BottomTabStrip over m_BottomContent, in the half under the toolbar.
	protected void PinBottomPanelGeometry( float contentHeight, Widget band = null, float bandHeight = 0 );

	//! Generic: any fixed-height strip over a content pane. Only for a strip that is neither of the above.
	protected void PinStripGeometry( Widget strip, Widget content, float contentHeight, int stripHeight, Widget band = null, float bandHeight = 0 );
```

```c
override void OnCreate()
{
    m_BottomTabStrip = layoutRoot.FindAnyWidget( "panel_bottom_tabs" );
    m_BottomContent  = layoutRoot.FindAnyWidget( "panel_bottom_content" );
    ...
}

override void OnResize( float w, float h )
{
    PinBottomPanelGeometry( h - TOOLBAR_HEIGHT );   // or PinRightPanelGeometry( h )
}
```

`band` is an optional widget of `bandHeight` pixels between the strip and the content (Weather's mode
banner). Do not write a form's own pin arithmetic: it would not know about wrapped tab rows (section 5,
A2) and would draw the content over them. A form that sizes something from the space under the strip asks
`GetPinnedStripHeight( GetBottomTabStrip() )` instead of using `TAB_STRIP_HEIGHT`, which is the height of
one row.

`contentHeight` is in **layout pixels**, as handed to `OnResize` by the window. Never derive it from
the panel — a panel declared with `hexactsize/vexactsize 0` returns its *fraction* of the parent from
`GetSize()`, and feeding that into an exact-size `SetSize` renders an empty panel.

---

## 5. Tab Architecture

### A. Lazy / Deferred Tab Construction

Do not instantiate every tab's widgets in `OnCreate()`. Register the containers, build only the tab
that is about to be shown, and build the rest on first activation.

`JMFormBase` owns the whole lazy tab lifecycle - see [naming.md](../systems/naming.md) for the hook grammar.
Declare the tabs, override the indexed hooks (or register a `JMFormTab` per tab), and forward the strip callback to
`HandleTabChange()`.

```c
// What m_Tabs.AddTab() returned for each tab - see OnCreate below.
protected int m_TabIdGeneral;
protected int m_TabIdPosition;
protected int m_TabIdSettings;

//! The one hook the base needs: GetActiveTabIndex(), AddTab(), HandleTabChange() and SetTab*() all work from it.
protected override COT_ScriptedWidgetEventHandler GetTabStrip()
{
    return m_Tabs;
}

override void OnCreate()
{
    // ... resolve panels, create the tab strip and scroller ...
    m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );
    m_TabIdGeneral  = m_Tabs.AddTab( "General",  JMConstants.Lucide( "user" ),     m_TabGeneral );   // the strip hands out the id
    m_TabIdPosition = m_Tabs.AddTab( "Position", JMConstants.Lucide( "map-pin" ),  m_TabPosition );
    m_TabIdSettings = m_Tabs.AddTab( "Settings", JMConstants.Lucide( "settings" ), m_TabSettings );

    DeclareTabs( 3 );

    // sendEvent = false: no hooks have run for the first tab yet.
    m_Tabs.SetSelection( m_TabIdGeneral, false );
    InitTabFocus( m_TabIdGeneral );
}

void OnChange_Tab( UIEvent eid, UIActionBase action )
{
    if ( eid == UIEvent.CHANGE )
        HandleTabChange();   // CloseAllOverlays, OnTabUnfocus(prev), OnTabCreate (first time), OnTabFocus, OnTabUpdate
}

// Once per tab, the first time it is selected. Indices are reserved at runtime in strip order
// (m_TabIdGeneral = m_Tabs.AddTab( label, icon, panel ); ...) - never written as numbers, so tabs other mods append cannot
// collide. A tab that is a JMFormTab class is attached with RegisterTab( id, tab ) and reports its own
// index via tab.GetTabId(); the base forwards the hooks to it. Tabs without a class use the ids:
override protected void OnTabCreate( int tab, Widget panel )
{
    if ( tab == m_TabIdGeneral )
        BuildGeneral( panel );
    else if ( tab == m_TabIdPosition )
        BuildPosition( panel );

    // A tab created after the form already has a selection has missed every update pass.
    if ( m_SelectedInstance )
        UpdateUI();
}

override protected void OnTabUnfocus( int tab )
{
    HideHoverPanel();          // anything anchored outside the tab's own panel
}

// Repaint the focused tab; timers call UpdateActiveTab() instead of polling IsTabActive().
override protected void OnTabUpdate( int tab )
{
    if ( tab == m_TabIdPosition )
        RefreshPosition();
}
```

Tabs that own a lot of state are classes extending `JMFormTab` (`OnCreate`, `OnUpdate`, `OnFocus`, `OnUnfocus`,
`OnResize`, `OnDestruct`); register them from `OnTabCreate` and call `ResizeTabs( w, h )` from `OnResize`.

> **The convention that makes deferring safe: null-guard every access to a widget from a tab the
> caller does not own.** Anything driven by a timer or a server response — a 10Hz refresh, a disease
> mask arriving from the server — can run before the tab holding its widgets has ever been opened.
> `if ( m_Health ) m_Health.SetCurrent( ... );` is the whole discipline. A handler reached by clicking
> a control *on* that tab needs no guard: the control cannot have been clicked if it does not exist.

### A2. Tab strips wrap

Tabs are equal-width columns. When the widest tab (icon + caption) no longer fits its column, `UIActionTabs` adds a row instead of
letting icons and captions collide, spreading the tabs evenly (nine tabs become 5 + 4). Each row is its own single-row bar, 30 px
tall; do not build the rows as one multi-row grid spacer, which makes them taller than that and gets them clipped. The strip
re-checks its own width every 250 ms, so a strip in a flowing container (the Example form) just grows.

A **pinned** strip has to make room for the extra row. `PinStripGeometry` / `PinRightPanelGeometry` already do: they call
`FitTabStrips( strip )` and pin the strip that much taller. The window's layout has not always settled when `OnResize` measures, so
a strip that changes its row count later tells its form through `OnTabStripFitted()`, which pins again and hands the tabs their size again. Forms need do nothing for that.

Anything that sizes itself from the space under the strip must not assume it is exactly `TAB_STRIP_HEIGHT` tall - ask
`GetPinnedStripHeight( strip )` (Loot Analysis and the ESP object list do). Keep captions short: a wrap costs vertical space.

### B. Centralized Floating Overlay Management

Dropdowns, context menus, and value prompts anchor their popup to the **window root**, not to their
own parent, so hiding a tab does not hide them — an open list floats over whichever tab comes next.
Register each one at creation, and the base dismisses them together.

```c
	//! On JMFormBase. Public, so a component can register its own popups through m_Form.
	void AddOverlay( COT_ScriptedWidgetEventHandler control );
	void RemoveOverlay( COT_ScriptedWidgetEventHandler control );
	protected void CloseAllOverlays();
```

The registry is typed to `COT_ScriptedWidgetEventHandler` rather than `UIActionBase` because
`JMFormBase` lives in **4_World** and cannot see 5_Mission types. That class is the common ancestor of
both `JMFormBase` and `UIActionBase`, and it declares the `Close()` / `UpdatePermission()` contract the
walks call. Call sites pass a `UIActionBase` as normal — the upcast is implicit.

```c
    m_DiseaseAgent = UIActionManager.CreateDropdown( section, "#STR_...", m_Window.GetWidgetRoot(), this, "" );
    AddOverlay( m_DiseaseAgent );          // from a component: m_Form.AddOverlay( m_DiseaseAgent )
```

```c
void OnChange_Tab( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE )
        return;

    HandleTabChange();   // closes overlays, creates/focuses/updates the selected tab

    if ( m_ActionListScroller )
        m_ActionListScroller.UpdateScroller();
}
```

`super.OnHide()` calls `CloseAllOverlays()` too, so a form that chains to super needs nothing further.

> `RegisterOverlay` / `UnregisterOverlay` are the old names and still work (deprecated). Not to be confused with `JMStatics.AddOverlay( Widget )`, a *global* registry used by
> `UIActionColorPicker` / `UIActionValuePrompt` so workspace-level popups pass the "is this COT UI"
> hit-test. Different thing, different purpose.

### C. Active Tab Performance Gating

Gate 10Hz background work on the tab being visible. `IsTabActive` is on the base; it reads the
selection through a hook the form overrides, because `UIActionTabs` is a 5_Mission type the base
cannot name:

```c
	//! On the form:
	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}
```

```c
void RefreshStats( bool force = false )
{
    if ( IsTabActive( m_TabIdPosition ) )
        RefreshPositionMap();
}
```

A form that does not override the hook reports every index active, so `IsTabActive` is safe to call
from shared code.

---

## 6. Panel & Card System (`UIActionCard`)

Use `UIActionCard` to structure tab content into visual sections, and put a section's own actions in
its title bar rather than in a toolbar row above it — a row of its own costs a full line of height.

```c
private Widget InitWidgetsGeneral( Widget actionsParent )
{
    Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

    UIActionCard card = UIActionManager.CreateCard( parent, "#STR_COT_MODULE_SECTION_IDENTITY" );
    Widget cardContent = card.GetContent();

    // Premade header actions - icon, 28x28 sizing and tooltip in one line each.
    // Creation order is left-to-right: the strip packs right, so the first
    // child created ends up leftmost.
    m_RefreshBtn = card.AddRefreshButton( this, "OnClick_Refresh" );
    m_ApplyBtn   = card.AddApplyButton( this, "OnClick_Apply" );
    m_DeleteBtn  = card.AddDeleteButton( this, "OnClick_Delete" );

    Widget grid = UIActionManager.CreateGridSpacer( cardContent, 2, 2 );
    UIActionManager.CreateCheckbox( grid, "#STR_OPTION_1", this, "OnClick_Option1" );
    UIActionManager.CreateCheckbox( grid, "#STR_OPTION_2", this, "OnClick_Option2" );

    return parent;
}
```

> **Exactly one Size-To-Content-V in the chain.** `UIActionCard` already spends it on `card_content`.
> Nesting a second one resolves circularly and the card renders as a title bar with nothing under it.
> The reasoning is written out at the top of `UIActionCard.c` — read it before adding a wrapper.

---

## 7. Premade Action Buttons & Built-in Animations

Static factories on `UIActionManager`, mirrored as `Add*Button` methods on `UIActionCard` that target
the card's own header strip:

| Button Type | Creator Method (UIActionManager / UIActionCard) | Built-in Icon | Theme / Style | Universal Animation Call |
|---|---|---|---|---|
| **Refresh** | `CreateRefreshButton(...)` / `card.AddRefreshButton(...)` | `refresh-cw` | Standard Pill | `action.AnimateSpin()` (360° spin) |
| **Delete** | `CreateDeleteButton(...)` / `card.AddDeleteButton(...)` | `trash-2` | `JMTheme.DANGER` (Red) | `action.AnimateFeedback()` or `action.AnimateError()` |
| **Save** | `CreateSaveButton(...)` / `card.AddSaveButton(...)` | `save` | Standard Pill | `action.AnimateFeedback()` (green checkmark flash) |
| **Apply** | `CreateApplyButton(...)` / `card.AddApplyButton(...)` | `check` | Standard Pill | `action.AnimateFeedback()` (green checkmark flash) |
| **Copy** | `CreateCopyButton(...)` / `card.AddCopyButton(...)` | `copy` | Standard Pill | `action.AnimateFeedback()` (green checkmark flash) |
| **Paste** | `CreatePasteButton(...)` / `card.AddPasteButton(...)` | `clipboard-paste` | Standard Pill | `action.AnimateFeedback()` (green checkmark flash) |
| **Add** | `CreateAddButton(...)` / `card.AddAddButton(...)` | `plus` | Standard Pill | `action.AnimateFeedback()` |
| **Edit** | `CreateEditButton(...)` / `card.AddEditButton(...)` | `pencil` | Standard Pill | `action.AnimateFeedback()` |
| **Search** | `CreateSearchButton(...)` / `card.AddSearchButton(...)` | `search` | Standard Pill | `action.AnimateFeedback()` |
| **Lock** | `CreateLockButton(...)` / `card.AddLockButton(...)` | `lock` | Standard Pill | `action.AnimateFeedback()` |
| **Visibility** | `CreateVisibilityButton(...)` / `card.AddVisibilityButton(...)` | `eye` | Standard Pill | `action.AnimateFeedback()` |

For an icon the premade set does not cover, use the generic header helper rather than hand-rolling
the three-line create/size/tooltip sequence:

```c
	UIActionImageButton AddCardHeaderAction( string iconPath, Class instance, string callback, string tooltip = "" );
```
```c
    m_ExportBtn = card.AddCardHeaderAction( JMConstants.Lucide( "download" ), this, "OnClick_Export", "#STR_..." );
```

### Handling Click Events & Triggering Animations

`AnimateSpin()`, `AnimateFeedback()`, `AnimateSuccess()` and `AnimateError()` are declared on
`UIActionBase`, so no cast is needed:

```c
void OnClick_RefreshData( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;

    action.AnimateSpin();

    m_Module.RequestDataRefresh();
}
```

> **They are empty stubs on the base.** Real behaviour is implemented by `UIActionImageButton` and
> `UIActionButton`. Calling one on a control that does not override it is a silent no-op, not an
> error — so if an animation "does nothing", check the control's concrete type first.

---

## 8. Declarative Permission Binding

Bind the control to its permission key once, at creation. `BindPermission` evaluates it
immediately *and* records the pair, so `OnClientPermissionsUpdated()` keeps it in step from then on
with no per-control code. A control whose permission is missing is **hidden**, not merely disabled.

```c
	//! On JMFormBase:
	void BindPermission( COT_ScriptedWidgetEventHandler control, string permissionKey );
	protected void ApplyRegisteredPermissions();
```

```c
    m_GodMode = UIActionManager.CreateCheckbox( grid, "#STR_...", this, "OnClick_GodMode", false );
    BindPermission( m_GodMode, JMConstants.PERM_PLAYER_GODMODE );
```

The base `OnClientPermissionsUpdated()` walks the map for you. A form that overrides it **must** chain:

```c
override void OnClientPermissionsUpdated()
{
    super.OnClientPermissionsUpdated();

    UpdateUI();
}
```

Without the `super` call, registered controls stop tracking permission changes.

### Event Handler Signature

Binding a control controls its *enabled state*; it is not an authorization check. Always guard the
handler as well:

```c
void OnClick_GodMode( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;

    if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_GODMODE ) )
        return;

    // Action implementation...
}
```

---

## 9. Selection State & Disable Overlay Pattern

When no target (player, vehicle, territory) is selected in the navigation panel, the detail panel is
covered by `m_RightPanelDisable`. `ShowUI()` / `HideUI()` are on the base and also close the
overlays; `UpdateUI()` stays in the form, because deciding whether anything is selected is domain
logic.

```c
override void HideUI()
{
    super.HideUI();

    m_SelectedInstance = NULL;
}

void UpdateUI()
{
    if ( GetSelectedCount() == 0 )
    {
        HideUI();
        return;
    }

    ShowUI();

    // ... push the selection into the controls ...
}
```

---

## 9b. Requesting Data, Search Header, Search Filter

Three pieces every list form used to write by hand. Use the shared ones.

**Data on show.** A module that fetches its list from the server overrides `RequestData()` (calling its own named request,
which stays public for the refresh button). `JMFormBase.OnShow()` calls it every time the window opens, so the form has no
`OnShow()` of its own for this. A form that does override `OnShow()` for something else must call `super.OnShow()` or the
request is lost.

```c
//! In the module:
override void RequestData() { RequestBanList(); }
```

**Header row.** `UIActionManager.CreateSearchRow` (a compact wrap row: refresh, search box, filter button, widths as fractions) and
`UIActionManager.CreateSearchFlexRow` (a flex row: fixed-size refresh, search box takes the rest) both return a `JMSearchRow`.
Use the flex one when the search box has to end on the same right edge as full-width controls under it. Keep the form's own
`m_SearchRow` / `m_SearchBar` fields (sub-mods read them) and assign them from the result:

```c
JMSearchRow bar = UIActionManager.CreateSearchFlexRow( panel, "#STR_COT_GENERIC_SEARCH", this, "OnChange_Search", "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH", 30 );
m_SearchRow = bar.Row;          // add more controls to it, then call m_SearchRow.SetGap( ... ) again
m_SearchBar = bar.Search;
```

**Row filter.** `JMSearchMatcher` (`Scripts/1_Core`) is the one search test: all words must appear, or any of them for `a | b`,
case-insensitive, and an empty needle matches everything, so the caller needs no `if ( filter != "" )`. Build it once before
the loop, not per row:

```c
JMSearchMatcher matcher = new JMSearchMatcher( m_SearchFilter );

foreach ( JMPlayerBan ban : m_BanList )
{
    if ( !matcher.Matches( ban.PlayerName ) && !matcher.Matches( ban.SteamID ) )
        continue;
```

---

## 10. Extension & Modding Compatibility Guidelines

### `protected` Visibility Standard

For maximum compatibility with third-party sub-mods (DayZ-Expansion ships three
`modded class JMPlayerForm` files — AI, Hardline, PersonalStorage):

- **Always use `protected` visibility** (never `private`) for:
  - Form module reference (`m_Module`)
  - Target selection instance (`m_SelectedInstance`)
  - Card title header action buttons (`m_ApplyStats`, `m_RefreshStats`)
  - Layout anchor controls (`m_Stamina`, `m_Health`, identity rows)

Panel container widgets (`m_LeftPanel`, `m_RightPanel`, `m_RightPanelDisable`, `m_RightTabStrip`,
`m_RightContent`) are already `protected` on `JMFormBase`.

Members that DayZ-Expansion or the examples name (`m_ApplyStats`, `m_Stamina`, `m_RefreshStats`, `m_PlayerMenu`,
`PassesListFilters`, `RefreshMapMarkers`, `m_Map`, ...) keep their name, type and visibility when code is moved out of the
form: leave the member on the form and delegate from it (section 13). Read the field's comment before deleting anything
that looks unused.

Third-party mods extend module forms by overriding builder methods, re-sorting widgets via
`GetLayoutRoot().SetSort()`, and appending custom rows to existing container anchors. Private
visibility breaks script compilation for downstream mods.

---

## 11. Migrating an Existing Form

Applied in the same order every time:

1. Pick the archetype (below) and reshape the `.layout` to the matching structural-panel names,
   stripping hardcoded control widgets down to bare containers.
2. Delete any per-form copies of the design tokens and the panel members — they are inherited.
3. Add `OnResize()` → `PinStripGeometry` / `PinRightPanelGeometry`.
4. Where the form has tabs, convert eager init to `DeclareTabs` + `OnTabCreate` (or a `JMFormTab` per tab), and null-guard
   every timer- or response-driven access to a deferred tab's widgets.
5. `AddOverlay( ... )` every dropdown, context menu and value prompt at its creation site;
   replace hand-written teardown with `CloseAllOverlays()`.
6. `BindPermission( control, key )` at each gated control's creation site; delete manual
   permission blocks; chain `super.OnClientPermissionsUpdated()`.
7. Group loose rows into `UIActionCard`s and move per-section actions into card header buttons.
8. Mark the module ref, selection instance and header action buttons `protected`.
9. Add new `#STR_COT_*` keys to `languagecore/stringtable.csv`; reuse the shared `STR_COT_GENERIC_*`
   keys rather than adding per-module duplicates.

### Archetype assignment

| Archetype | Panels | Forms |
|---|---|---|
| **A. Split-pane (roster + tabbed detail)** | `panel_left` / `panel_split` / `panel_right` (+`_tabs`/`_content`) / `panel_right_disable` | `JMPlayerForm` (reference), `JMVehiclesForm`, `JMEntityManagerForm`, `JMRoleManagerForm` (no tab strip - its right pane is one editor driven by the left-hand mode), `JMWeatherForm` |
| **B. Top/bottom (filter header + data grid)** | `panel_top` / `panel_bottom` (+`_tabs`/`_content`) | `JMPlayerBanForm`, `JMTerritoryForm`, `JMESPForm`, `JMItemStatsForm`, `JMLootAnalysisForm`, `JMTeleportForm`, `JMObjectSpawnerForm` |
| **C. Full-pane tabbed / simple** | root `panel` (+ optional tab strip) | `JMCameraForm`, `JMLoadoutForm`, `JMWebhookForm`, `JMCompensationsForm`, `JMAntiCheatForm`, `JMNamalskEventManagerForm`, `JMExampleForm` |
| **D. Out of scope** | — | `JMMapForm` (vanilla map-widget chain), `JMMapEditorForm` (viewport is drawn, not widgets), `JMConfirmationForm` (dialog; its static buttons are correct) |

---

## 12. Tab Classes: What a Tab Owns

A tab with more than a few widgets is a class extending `JMFormTab`, in its own file, in its own folder (section 14).
The form keeps what the tabs share; **everything only one tab uses belongs to that tab.**

| Belongs to the tab | Stays on the form |
|---|---|
| Its widgets, and the `m_*` fields only its handlers touch | The tab strip, the panels inherited from `JMFormBase`, the module reference |
| Its constants (`static const`), row heights, column widths, map layer ids, menu row ids | State several tabs read (the selected player, the search text, the last resize height) |
| Its own timers, hover panels and sub-menus | The `m_TabId*` fields the strip handed back |
| The click / change handlers for its controls | Callbacks a `JMFormBase` helper resolves on the form (section 13) |

**Tab ids come from the strip, never from a constant.** `m_TabIdGeneral = m_Tabs.AddTab( ... )` reserves the index at
runtime, so a tab another mod appends cannot collide. A form has no `TAB_GENERAL = 0`; inside a tab use
`GetTabId()`:

```c
//! In the tab: is this tab the one on screen?
if ( m_Form.IsTabActive( GetTabId() ) )
    RefreshPositionMap();
```

Anything the form needs from a tab goes through a method on the tab, not through the tab's fields. The form holds the
tab by `ref` and creates it lazily, in `OnTabCreate`, on first selection (section 5.A), so every call from the form
into a tab is null-guarded.

**Constants move with their tab, and moving one is a breaking change.** A `static const` cannot forward or warn, so
list every moved constant under "Removed with no forwarder" in [deprecations.md](../systems/deprecations.md).
A `static const` that is *computed* from other consts evaluates to `0` in Enforce; expose a `static` function
instead (`GetLeftHeaderHeight()`), and assign a computed global (`JMVT_ALL`) in a constructor, not at the declaration.

---

## 13. Splitting a Form: Components

A form that owns a roster, a filter menu, a popup and a map overlay is four classes' worth of code in one. Extract each
cohesive piece into a **component**: a plain class (not a `JMFormTab`, it has no tab panel) that the form creates and
holds.

```c
//! Owns the vehicle type filter: the toolbar button, its dropdown, and the bitmask of shown types.
class JMVehiclesTypeFilter
{
    protected JMVehiclesForm m_Form;            //! plain member, NOT ref: the form holds the ref, so no cycle

    void JMVehiclesTypeFilter( JMVehiclesForm form ) { m_Form = form; }

    void Build( Widget parent, UIActionFlexRow row ) { ... }   //! creates its widgets
}

// In the form:
protected ref JMVehiclesTypeFilter m_TypeFilter;            // created in the constructor
```

Rules that came out of doing this repeatedly (player roster, weather banner, teleport filter / popup / overlays, vehicles filter, object spawner properties, camera travel panel):

1. **The component is the callback target.** `SetOnClick( this, "OnClick_X" )` and `CreateIconButton( parent, icon, this, "OnClick_X" )`
   resolve the name on the object passed, so once the widget is built inside the component, the handler moves with it.
2. **`JMFormBase` helpers are the exception.** `ConfirmAction`, `PromptInput`, `CreateAdvancedPlayerConfirm` and
   `CreateConfirmation_*` resolve their callback names **on the form**. Keep a one-line landing pad on the form that
   forwards to the component (or leave the whole chain on the form, as `JMAntiCheatForm` and the CE-save chain in
   `JMLootAnalysisForm` do).
   `CreateOverlayMenu( form, target, "Cb" )` and `CreateOverlayFilterMenu( form, anchor, registry )` take the **form
   first** and the callback target second.
3. **The component does not inherit from `JMFormBase`.** `m_Window`, `layoutRoot`, `AddOverlay`, `RemoveOverlay`,
   `GetWindow` and every other base member must be reached through `m_Form` (`m_Form.AddOverlay( menu )`,
   `m_Form.GetLayoutRoot()`, `m_Form.GetWindow()`). A protected base member needs a public getter on the form first
   (`GetModule()`, `GetPreviewItem()`). Globals with the same name as a form method (`GetPermissionsManager()`,
   `GetMousePos()`) are fine bare. **The lint does not catch a bare base-member use; the game compile does**, one error
   per build, so scan first (`scan_inherited.py`, section 15).
4. **A protected method cannot be called from another class.** If the component needs a form hook that is protected and
   overridable (`PassesListFilters`, `RefreshMapMarkers`), leave the hook on the form, delegate from it into the
   component, and add a public one-line wrapper (`FilterPlayer( player )`) for the component to call back.
5. **Forwarders keep the old signature.** When another class, a `modded class` or an `ExampleScriptOverride` calls a
   method that moved, leave a forwarder on the form with the *same* signature as before, and mark it deprecated
   (`JMDeprecated.WarnOnce( this, "..." )`) if it should go.
6. **Do not create a second declaration.** If a tab already declares the field a component is about to receive, moving it
   produces "Multiple declaration of variable". Check the target class for the name first.

**Extension points are API.** `ExampleScriptOverride/`, `modded class JMPlayerForm` in DayZ-Expansion and the
`protected` field comments are contracts (section 10). Before moving a member, `grep` the examples and
[mod-compatibility.md](../systems/mod-compatibility.md). Members named there stay where they are, and keep their
visibility.

**Where the line is.** Extract when the piece has its own widgets and state and a clear name. Do not extract
a dialog whose callbacks resolve on the form: the class would only add forwarders. Do not split a page-builder
(`JMESPActionMenu`: `Build*` paired with `Do*`) or a module's RPC handlers, which are dispatched by name on the module.

---

## 14. Where Files Go

Inside a module folder each tab has a folder, and reusable parts have `Components/`:

```
modules/Player/
    JMPlayerModule.c   JMPlayerForm.c
    Components/        JMPlayerRoster.c  JMPlayerRowMenu.c  JMPlayerStat.c ...
    Tabs/General/      JMPlayerFormTabGeneral.c  JMPlayerStatRow.c
    Tabs/Inventory/    JMPlayerFormTabInventory.c
```

A new tab goes in `Tabs/<Name>/`, named `<Prefix>FormTab<Name>.c`. A helper only that tab uses sits beside it; a piece
extracted from a form (section 13) goes in `Components/`. The full rule, what counts as a component and what is never
moved, is in [module-folder-layout.md](../systems/module-folder-layout.md); `organize_tabs.py` applies it and is safe to
re-run.

**One class per file, file name = class name.** The exception is a set of tiny subclasses of one base
(`JMPlayerStats.c`), and a file that says so in its header comment. Layouts (`GUI/layouts/`) and stringtable keys live in
other addons and are found by path/key, so they do not move with the scripts.

---

## 15. Writing and Maintaining Form Scripts

### Class layout

Members in this order, so a class reads top-down:

1. **Fields**, all at the top: `static` / `static const` fields first, then one blank line, then the member fields (see
   [conventions.md](../systems/conventions.md)). A field with no doc comment sits directly under the previous field, with no blank line
   between them. A field with a `//!` comment gets a blank line above it.
2. **Constructors** (`void ClassName()`), then the **destructor** (`void ~ClassName()`, guard with `if ( !g_Game ) return;`).
3. **Getters** (`Get*`, `Is*`, `Has*`), A-Z.
4. **Setters** (`Set*`), A-Z.
5. **Everything else**, in reading order: overrides, handlers, helpers.

`reorder.py` and `compact.py` enforce this and are idempotent; run them after any edit that adds members.

### Rules that are easy to break

- **Never delete or shorten a documentation comment** when moving or editing code. Comments travel with the member.
- No block scope: variables declared in one `if` branch are visible in the next, so use distinct names.
- No ternary `? :`, no multi-line `||` / `&&` / `+` expressions.
- Deferred tabs: null-guard every widget access from timer- or response-driven code (section 5.A).
- Every dropdown, context menu and value prompt is registered with `AddOverlay( c )` at its creation site.
- A string callback that names a method that does not exist fails **silently**. After moving a method, grep the quoted
  name and confirm the object passed as target is the class that now holds it.
- `CallLater( Method, ... )` binds to an instance: after a move it must name the component's method, and the matching
  `Remove` must match.
- Implicit object-to-string concatenation inside a `Print()` crashes the script compiler natively. Format explicitly.
- `#define` scope is per file, not per compiled layer.

### Changing what other mods can see

- Removing or renaming a public member is a breaking change. Leave a forwarder with the old signature and
  `JMDeprecated.WarnOnce( instance_or_null, "..." )`, and record it in [deprecations.md](../systems/deprecations.md).
  A `static const` cannot forward, so moving one is always recorded under "Removed with no forwarder".
- Mark the module reference, selection state and anything a sub-mod overrides `protected`, never `private` (section 10).
- New user-visible text is a `#STR_COT_*` key in `languagecore/stringtable.csv` with all 16 columns. Before deleting or
  merging a key, check every call site: two keys with different text can be deliberate.

### Verifying a change

`verify-fix` is only the lint. It does not check member lookup or signatures, so a form or component edit is done only
after a **fresh full build boots with 0 `JM/COT` errors**.

1. `./dz-project-manager.exe verify-fix` - must report `PASS`.
2. Stop any game process left from the last boot; a server that stayed up makes the next build replay the old verdict.
3. Record the time, build unpiped in the background, and wait for a log newer than that time:

   ```
   date +%s > "$TEMP/buildstart.txt"
   ./build.ps1 -NoBump --confirm build               # run_in_background
   bash Workbench/Batchfiles/cleanup/wait_build.sh "$(cat $TEMP/buildstart.txt)"
   ```
4. Read the **first** `(E)` under `JM/COT`. `Bad type 'map'` / `Bad type CF_Modules` further down are follow-ons.
5. A server boot does not run `OnCreate`, widget building or menus. Open the form in-game and try each tab, list, filter,
   menu and dialog you touched.

The tools (`reorder.py`, `compact.py`, `audit_members.py`, `move_members.py`, `extract_component.py`,
`scan_inherited.py`, `organize_tabs.py`, `wait_build.sh`) and the reasoning behind each rule are in
[cleanup-playbook.md](../systems/cleanup-playbook.md).

---

## 16. Implementation Checklist for New Module Forms

- [ ] XML `.layout` defines modular panels (`panel_left`/`panel_right` or `panel_top`/`panel_bottom`) with minimal outer framing.
- [ ] No per-form redeclaration of the design tokens or the inherited panel members.
- [ ] `OnResize()` pins the tab strip through `PinRightPanelGeometry()` / `PinBottomPanelGeometry()` (members resolved in `OnCreate()`), never with its own pin arithmetic.
- [ ] Tab building uses `DeclareTabs()` + `OnTabCreate()` (via `HandleTabChange()`) on first selection.
- [ ] Timer- and response-driven code null-guards widgets belonging to deferred tabs.
- [ ] `GetTabStrip()` overridden if the form has tabs, so `GetActiveTabIndex()` / `IsTabActive()` work.
- [ ] All popups, dropdowns and context menus registered with `AddOverlay()`.
- [ ] `OnHide()` chains `super.OnHide()`.
- [ ] Card header actions created via `card.Add*Button()` / `card.AddCardHeaderAction()`.
- [ ] Permission keys bound via `BindPermission()`, enforced again inside handlers, and
      `OnClientPermissionsUpdated()` chains `super`.
- [ ] Key controls, module references and card action buttons marked `protected`.
- [ ] Tab ids come from `AddTab()` / `GetTabId()`; no `TAB_*` constants on the form.
- [ ] Constants, state and timers used by one tab live on that tab's class.
- [ ] Each cohesive piece (roster, filter, popup, overlay) is a component with a non-`ref` `m_Form`; base-class members are reached through `m_Form`.
- [ ] Callbacks resolved by `ConfirmAction` / `PromptInput` / `CreateConfirmation_*` are methods on the form.
- [ ] Data the form shows is requested by the module's `RequestData()`, not by an `OnShow()` override; any `OnShow()` chains `super`.
- [ ] A refresh + search header comes from `CreateSearchRow` / `CreateSearchFlexRow`; a search test is a `JMSearchMatcher` (section 9b).
- [ ] Files sit in `Tabs/<Name>/` and `Components/` (section 14); members in the section 15 order.
- [ ] A fresh full build boots with 0 `JM/COT` errors, and the form has been opened in-game.
- [ ] Removed or moved public members are recorded in `deprecations.md`.
