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
`scriptclass` must match your form class name. Inner widget name (e.g. `panel`) is referenced in `OnInit()`.

## Form Class Pattern

```c
class JMMyForm : JMFormBase
{
    private JMMyModule m_Module;
    private UIActionScroller m_Scroller;
    private Widget m_Content;

    // Required: link form to typed module
    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }

    // Build static UI once
    override void OnInit()
    {
        m_Scroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_Content = m_Scroller.GetContentWidget();

        Widget toolbar = UIActionManager.CreateGridSpacer( m_Content, 1, 2 );
        UIActionManager.CreateButton( toolbar, "Refresh", this, "OnClick_Refresh" );

        UIActionManager.CreateEditableTextPreview( m_Content, "Search...", this, "OnChange_Search" );

        m_Scroller.UpdateScroller();
    }

    // Called every time form becomes visible
    override void OnShow()
    {
        super.OnShow();
        m_Module.RequestData();
    }

    // Permission-gated controls
    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();
        
        m_ActionButton.UpdatePermission( "Admin.MyModule.Action" );
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
| `SetModule(mdl)` | On construction — link to typed module, return true if cast OK |
| `OnInit()` | Once — build all static UI elements |
| `OnShow()` | Every time form becomes visible |
| `OnHide()` | Every time form is hidden |
| `OnFocus()` | Form receives focus |
| `OnUnfocus()` | Form loses focus |
| `OnSettingsUpdated()` | COT settings changed |
| `OnClientPermissionsUpdated()` | Player permissions changed — rebuild permission-gated controls |
| `Update()` | Per-frame (not called automatically — must be wired up) |

Key members available in subclass:
```c
Widget layoutRoot;     // root widget (set by engine)
CF_Window m_Window;    // parent window
```

## UIActionBase — Event Handler Signature

All callbacks have the same signature:
```c
void OnSomething( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK ) return;   // filter event type
}
```

Common `UIEvent` values: `CLICK`, `CHANGED`, `FOCUS`, `UNFOCUS`

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
// In OnClientPermissionsUpdated():
m_Button.UpdatePermission( "Admin.MyModule.Action" );
// Shows/hides a disable overlay over the widget automatically

// Or manually:
m_Button.SetEnabled( GetPermissionsManager().HasPermission( "Admin.MyModule.Action" ) );
```

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
UIActionButton         CreateButton( Widget parent, string label, Class instance, string funcname, float width = 0 )
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
UIActionDropdownList CreateDropdownList( Widget parent, ... )
```

### Display
```c
UIActionText   CreateText( Widget parent, string text )
UIActionImage  CreateImage( Widget parent, string imagePath )
```

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
