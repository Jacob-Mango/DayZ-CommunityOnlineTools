---
name: UI and Forms
description: Layout file format, JMFormBase lifecycle, UIActionBase data system, UIActionManager full API, confirmation dialogs, permission gating
type: project
---

## Layout File Format

Location: `JM/COT/GUI/layouts/my_form.layout`

```xml
PanelWidgetClass JMMyForm {
    color 0 0 0 0
    position 0 0
    size 800 600
    scriptclass "JMMyForm"
    style DayZDefaultPanel
    {
        FrameWidgetClass panel {
            position 0 0
            size 1 1
        }
    }
}
```
`scriptclass` must match your form class name. Inner widget name (e.g. `panel`) is referenced in `OnCreate()`.

## Form Class Pattern

```c
class JMMyForm : JMFormBase
{
    protected JMMyModule m_Module;
    protected UIActionScroller m_Scroller;
    protected Widget m_Content;

    // Required: link form to typed module
    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    // Build static UI once (the base calls the legacy OnInit(); new code overrides OnCreate)
    override void OnCreate()
    {
        m_Scroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_Content = m_Scroller.GetContentWidget();

        Widget toolbar = UIActionManager.CreateGridSpacer( m_Content, 1, 2 );
        UIActionManager.CreateButton( toolbar, "Refresh", this, "OnClick_Refresh" );

        UIActionManager.CreateEditableTextPreview( m_Content, "Search...", this, "OnChange_Search" );

        // Bind once; re-evaluated on every permission change (see permissions.md)
        BindPermission( m_ActionButton, JMConstants.PERM_MYMODULE_ACTION );

        m_Scroller.UpdateScroller();
    }

    // No OnShow override needed: the base OnShow() calls m_ModuleBase.RequestData(), so the module's
    // RequestData() override is what fetches data every time the form becomes visible. An OnShow override
    // must call super.OnShow() or that request is lost.

    // Only needed for logic BindPermission cannot express; call super so registered controls keep tracking
    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();
    }

    // Event handlers
    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK ) return;
        m_Module.RequestData();
    }

    // Populate data from server
    void PopulateList( array<ref SomeData> items )
    {
        // Rebuild dynamic section
        m_Scroller.UpdateScroller();   // call after adding/removing widgets
    }
}
```

## JMFormBase Lifecycle

| Method | When called |
|--------|------------|
| `SetModule(mdl)` | From `Init()` — link to typed module, return true if cast OK (the rest of `Init()` is skipped otherwise) |
| `OnCreate()` | Once — build all static UI elements. Legacy name `OnInit()` still works (see [../systems/deprecations.md](../systems/deprecations.md)); `OnDestruct()` runs when the form is freed |
| `OnClientPermissionsUpdated()` | Called by `Init()` right after creation, and again on every permission change |
| `OnShow()` | After the first `OnClientPermissionsUpdated()`, and every time the form becomes visible. The base asks the module for data (`RequestData()`) |
| `OnHide()` | Every time form is hidden (the base closes all overlays) |
| `OnFocus()` / `OnUnfocus()` | Form gains / loses focus |
| `OnSettingsUpdated()` | COT settings changed |
| `Update()` | Per-frame (not called automatically — must be wired up) |

`Init( CF_Window wdw, JMRenderableModuleBase mdl )` runs them in that order: `SetModule` -> `OnCreate` -> (`OnInit` if the
override never reached the base) -> `OnClientPermissionsUpdated` -> `OnShow`. A form that splits itself into tabs also
gets the tab hooks (`OnTabCreate`, `OnTabFocus`, ...) - see [../systems/naming.md](../systems/naming.md).

Key members available in subclass:
```c
Widget layoutRoot;     // protected: root widget (set by engine)
CF_Window m_Window;    // protected: parent window handle (typed CF_Window; also `window` when CF_WINDOWS is undefined)
```

## UIActionBase — Event Handler Signature

All callbacks have the same signature:
```c
void OnSomething( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK ) return;   // filter event type
}
```

`UIEvent` values (append-only, compared across the client/server boundary): `CLICK`, `CLICK_RIGHTSIDE`, `CLICK_LEFTSIDE`, `CHANGE`, `MOUSEWHEEL`,
`CLICK_RIGHT` (right button pressed), `MOUSE_ENTER`, `MOUSE_LEAVE`, `DOUBLE_CLICK`. Note `CHANGE`, not `CHANGED`.

### Attaching Data to an Action

Use `UIActionData` to carry context with any widget:
```c
class MyRowData : UIActionData
{
    string SteamID;
    string PlayerName;
}

// When creating button:
UIActionButton btn = UIActionManager.CreateButton( parent, "Unban", this, "OnClick_Unban" );
MyRowData data = new MyRowData();
data.SteamID = ban.SteamID;
btn.SetData( data );

// In handler:
void OnClick_Unban( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK ) return;
    MyRowData data;
    if ( Class.CastTo( data, action.GetData() ) )
        m_Module.UnbanPlayer( data.SteamID );
}
```

### Permission Gating on a Widget

```c
// Once, in OnCreate(): tracked, and re-evaluated on every OnClientPermissionsUpdated()
BindPermission( m_Button, JMConstants.PERM_MYMODULE_ACTION );

// Or straight on the control when it is rebuilt per refresh (untracked):
m_Button.UpdatePermission( JMConstants.PERM_MYMODULE_ACTION );   // shows/hides a disable overlay automatically

// Or a permission at creation: UIActionManager.CreateButton( parent, label, this, "OnClick_X", 1, JMConstants.PERM_MYMODULE_ACTION )
```

Always pass the `JMConstants.PERM_*` constant, never a string literal (see [../systems/permissions.md](../systems/permissions.md)).

## UIActionManager — Full Widget API

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionManager.c`

### Containers / Layout
```c
UIActionScroller  CreateScroller( Widget parent )
GridSpacerWidget  CreateGridSpacer( Widget parent, int rows, int cols )
WrapSpacerWidget  CreateWrapSpacer( Widget parent, WidgetAlignment halign, WidgetAlignment valign )
Widget            CreatePanel( Widget parent, int color, float height )
Widget            CreateActionRows( Widget parent )
```

### Buttons
```c
UIActionButton         CreateButton( Widget parent, string label, Class instance, string funcname, float width = 1, string permission = "" )
UIActionButtonToggle   CreateButtonToggle( Widget parent, string off, string on, Class instance, string func )
UIActionNavigateButton CreateNavButton( Widget parent, string label, string imgRight, string imgLeft, Class instance, string func )
```

### Text Inputs
```c
UIActionEditableText         CreateEditableText( Widget parent, string label, Class instance, string func )
UIActionEditableTextPreview  CreateEditableTextPreview( Widget parent, string label, Class instance, string func, string text="", string button="" )
UIActionEditableRichText     CreateEditableRichText( Widget parent, string label, Class instance, string func )
UIActionEditableVector       CreateEditableVector( Widget parent, string label, Class instance, string func )
```

### Selection / Toggle
```c
UIActionCheckbox    CreateCheckbox( Widget parent, string label, Class instance, string func )
UIActionSlider      CreateSlider( Widget parent, string label, float min, float max, Class instance, string func )
UIActionSelectBox   CreateSelectionBox( Widget parent, string label, array<string> options, Class instance, string func )
UIActionDropdownList CreateDropdownBox( Widget parent, Widget dropDownParent, string label, array<string> values, ... )
```

### Display
```c
UIActionText   CreateText( Widget parent, string text )
UIActionImage  CreateImage( Widget parent, string imagePath )
```

### More factories

The lists above are the basic set, and parent, label and callback arguments are required as shown. `UIActionManager` has many
more; read the file for the exact signature (most take `Class instance = NULL, string funcname = ""` last):

- **Structure:** `CreateCard`, `CreateScrollCard`, `CreateSection`, `CreateSectionHeader`, `CreateSectionHeaderAction`,
  `CreateCollapsibleSection`, `CreateFoldPanel`, `CreateScrollableSection`, `CreateLabeledRow`, `CreateRow`, `CreateDivider`,
  `CreateSpacer`, `CreateSpacerPx`, `CreateEmptyState`, `CreateWrapSpacerCompact` / `CreateWrapSpacerFit`
- **Buttons:** `CreateImageButton`, `CreateIconButton`, `CreateImageButtonToggle`, `CreateFeedbackButton`, `CreateButtonPair`,
  `CreateHeaderButton`, and the stock icon buttons (`CreateRefreshButton`, `CreateDeleteButton`, `CreateSaveButton`,
  `CreateApplyButton`, `CreateCopyButton`, `CreatePasteButton`, `CreateAddButton`, `CreateEditButton`, `CreateSearchButton`, ...)
- **Inputs:** `CreateSearchBox`, `CreateSpinner`, `CreateToggle`, `CreateToggleSwitch`, `CreateSyncedSlider`, `CreateSliderRange`,
  `CreateTimePicker`, `CreateColorPicker`, `CreateDropdown`, `CreateMultiSelectList`, `CreateItemList`, `CreateConfirmInline`
- **Display:** `CreateBadge`, `CreateProgressBar`, `CreateKeyValueList`, `CreateDataTable`, `CreatePaginator`, `CreateFilterBar`,
  `CreateIconGrid`, `CreateLogView`, `CreateStepList`, `CreateMap` / `CreateMapFill`, `CreateTooltip`, `CreateLabeledValue`
- **Menus and popups:** `CreateContextMenu`, `CreateFilterMenu`, `CreateValuePrompt`, and the form-aware
  `CreateOverlayMenu( form, ... )`, `CreateOverlayFilterMenu( form, ... )`, `CreateOverlayPrompt( form, ... )`
  (see [filter-menus.md](filter-menus.md))
- **Tabs:** `CreateTabStrip( parent, instance, fn )` + `AddTab( label, icon, panel )`. `CreateTabs( parent, labels, ... )` is deprecated.
- **Search headers:** `CreateSearchRow`, `CreateSearchFlexRow`

`CreateSlider( parent, label, min, max, instance, funcname, sliderWidth = -1 )` and
`CreateButton( parent, label, instance, funcname, width = 1, permission = "" )` are the two whose trailing arguments differ most from the older docs.

### UIActionScroller Usage
```c
UIActionScroller scroller = UIActionManager.CreateScroller( parent );
Widget content = scroller.GetContentWidget();  // add children here
// ... build UI inside content ...
scroller.UpdateScroller();   // MUST call after adding/removing items
```

## Confirmation Dialogs (JMFormBase helpers)

For actions that need confirmation before executing:

```c
// Single button (informational)
JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title,
    string message, string callBackOneName, string callBackOne, int btnIdOffset = -1 )

// Two buttons (Yes/No)
JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title,
    string message,
    string callBackOneName, string callBackOne,
    string callBackTwoName, string callBackTwo,
    int btnIdOffset = -1 )

// Three buttons
JMConfirmation CreateConfirmation_Three( ... )

// Smart player-context confirmation (handles multi-select, single, self)
JMConfirmation CreateAdvancedPlayerConfirm(
    string title,
    string callbackMultiPlayers,   // N players selected
    string callbackSinglePlayer,   // 1 other player selected
    string callbackSelf,           // self
    bool confirmSelf = true,
    bool executeCallbackOnNoConfirmation = true )

// Object-context confirmation
JMConfirmation CreateAdvancedObjectConfirm(
    string callbackMultiObjects,
    string callbackSingleObject,
    bool executeCallbackOnNoConfirmation = true )
```
