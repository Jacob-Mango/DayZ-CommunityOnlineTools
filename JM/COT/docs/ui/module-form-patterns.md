# Form Design & Layout Guidelines: Tabs, Panels, and Controls

This document establishes the official design pattern and layout guidelines for module forms in
Community Online Tools (COT), derived from the architecture of `JMPlayerForm`.

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
    //! m_RightContent are inherited from JMFormBase. Resolve them in OnInit();
    //! do not redeclare them.

    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    override void OnInit()
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

`JMFormBase` provides both halves:

```c
	//! Generic: any fixed-height strip over a content pane.
	protected void PinStripGeometry( Widget strip, Widget content, float contentHeight, int stripHeight );

	//! Split-pane convenience: m_RightTabStrip over m_RightContent at TAB_STRIP_HEIGHT.
	protected void PinRightPanelGeometry( float contentHeight );
```

A split-pane form calls `PinRightPanelGeometry( h )`. A top/bottom form calls `PinStripGeometry`
directly with its own toolbar and grid widgets.

`contentHeight` is in **layout pixels**, as handed to `OnResize` by the window. Never derive it from
the panel — a panel declared with `hexactsize/vexactsize 0` returns its *fraction* of the parent from
`GetSize()`, and feeding that into an exact-size `SetSize` renders an empty panel.

---

## 5. Tab Architecture

### A. Lazy / Deferred Tab Construction

Do not instantiate every tab's widgets in `OnInit()`. Register the containers, build only the tab
that is about to be shown, and build the rest on first activation.

`JMFormBase` owns the bookkeeping (`m_TabBuilt`, `InitTabState`, `ShouldBuildTab`); the form owns the
switch, because only it knows what its cases are.

```c
static const int TAB_GENERAL  = 0;
static const int TAB_POSITION = 1;
static const int TAB_SETTINGS = 2;

private void InitWidgetsRight()
{
    // ... resolve panels, create the tab strip and scroller ...
    m_Tabs = UIActionManager.CreateTabs( m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );
    m_ActionListScroller = UIActionManager.CreateScroller( m_RightContent );
    m_ActionsWrapper = m_ActionListScroller.GetContentWidget();

    // Container GridSpacers for each tab, created up front - they are cheap.
    m_TabGeneral  = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 1 );
    m_TabPosition = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 8, 1 );
    m_TabSettings = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 6, 1 );

    m_Tabs.AddContent( m_TabGeneral );
    m_Tabs.AddContent( m_TabPosition );
    m_Tabs.AddContent( m_TabSettings );

    InitTabState( 3 );

    // sendEvent = false: OnChange_Tab pokes at widgets no builder has run for
    // yet on the very first pass.
    m_Tabs.SetSelection( TAB_GENERAL, false );

    BuildTabIfNeeded( TAB_GENERAL );

    m_ActionListScroller.UpdateScroller();
}

private void BuildTabIfNeeded( int tabIdx )
{
    //! ShouldBuildTab is true exactly once per index, and marks it built.
    if ( !ShouldBuildTab( tabIdx ) )
        return;

    switch ( tabIdx )
    {
        case TAB_GENERAL:  InitWidgetsGeneral( m_TabGeneral );   break;
        case TAB_POSITION: InitWidgetsPosition( m_TabPosition );  break;
        case TAB_SETTINGS: InitWidgetsSettings( m_TabSettings );  break;
    }

    // A tab built after the form already has a selection has missed every
    // update pass so far, so it would come up blank and unpermissioned.
    if ( m_SelectedInstance )
        UpdateUI();
}
```

> **The convention that makes deferring safe: null-guard every access to a widget from a tab the
> caller does not own.** Anything driven by a timer or a server response — a 10Hz refresh, a disease
> mask arriving from the server — can run before the tab holding its widgets has ever been opened.
> `if ( m_Health ) m_Health.SetCurrent( ... );` is the whole discipline. A handler reached by clicking
> a control *on* that tab needs no guard: the control cannot have been clicked if it does not exist.

### B. Centralized Floating Overlay Management

Dropdowns, context menus, and value prompts anchor their popup to the **window root**, not to their
own parent, so hiding a tab does not hide them — an open list floats over whichever tab comes next.
Register each one at creation, and the base dismisses them together.

```c
	//! On JMFormBase:
	protected void RegisterOverlay( COT_ScriptedWidgetEventHandler control );
	protected void UnregisterOverlay( COT_ScriptedWidgetEventHandler control );
	protected void CloseAllOverlays();
```

The registry is typed to `COT_ScriptedWidgetEventHandler` rather than `UIActionBase` because
`JMFormBase` lives in **4_World** and cannot see 5_Mission types. That class is the common ancestor of
both `JMFormBase` and `UIActionBase`, and it declares the `Close()` / `UpdatePermission()` contract the
walks call. Call sites pass a `UIActionBase` as normal — the upcast is implicit.

```c
    m_DiseaseAgent = UIActionManager.CreateDropdown( section, "#STR_...", m_Window.GetWidgetRoot(), this, "" );
    RegisterOverlay( m_DiseaseAgent );
```

```c
void OnChange_Tab( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE )
        return;

    CloseAllOverlays();

    int sel = m_Tabs.GetSelection();

    BuildTabIfNeeded( sel );

    if ( m_ActionListScroller )
        m_ActionListScroller.UpdateScroller();
}
```

`super.OnHide()` calls `CloseAllOverlays()` too, so a form that chains to super needs nothing further.

> Not to be confused with `JMStatics.RegisterOverlay( Widget )`, a *global* registry used by
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
    if ( IsTabActive( TAB_POSITION ) )
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

Bind the control to its permission key once, at creation. `RegisterPermission` evaluates it
immediately *and* records the pair, so `OnClientPermissionsUpdated()` keeps it in step from then on
with no per-control code.

```c
	//! On JMFormBase:
	protected void RegisterPermission( COT_ScriptedWidgetEventHandler control, string permissionKey );
	protected void ApplyRegisteredPermissions();
```

```c
    m_GodMode = UIActionManager.CreateCheckbox( grid, "#STR_...", this, "OnClick_Godmode", false );
    RegisterPermission( m_GodMode, "Admin.Player.Godmode" );
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
void OnClick_Godmode( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;

    if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode" ) )
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
4. Where the form has tabs, convert eager init to `InitTabState` + `BuildTabIfNeeded`, and null-guard
   every timer- or response-driven access to a deferred tab's widgets.
5. `RegisterOverlay( ... )` every dropdown, context menu and value prompt at its creation site;
   replace hand-written teardown with `CloseAllOverlays()`.
6. `RegisterPermission( control, key )` at each gated control's creation site; delete manual
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

## 12. Implementation Checklist for New Module Forms

- [ ] XML `.layout` defines modular panels (`panel_left`/`panel_right` or `panel_top`/`panel_bottom`) with minimal outer framing.
- [ ] No per-form redeclaration of the design tokens or the inherited panel members.
- [ ] `OnResize()` pins exact tab strip/toolbar geometry via `PinRightPanelGeometry()` / `PinStripGeometry()`.
- [ ] Tab building uses `InitTabState()` + `ShouldBuildTab()` on first selection.
- [ ] Timer- and response-driven code null-guards widgets belonging to deferred tabs.
- [ ] `GetActiveTabIndex()` overridden if the form has tabs, so `IsTabActive()` works.
- [ ] All popups, dropdowns and context menus registered with `RegisterOverlay()`.
- [ ] `OnHide()` chains `super.OnHide()`.
- [ ] Card header actions created via `card.Add*Button()` / `card.AddCardHeaderAction()`.
- [ ] Permission keys bound via `RegisterPermission()`, enforced again inside handlers, and
      `OnClientPermissionsUpdated()` chains `super`.
- [ ] Key controls, module references and card action buttons marked `protected`.
