# COT UI System Style Guide

This guide documents the naming conventions, patterns, and best practices for developing UI forms and widgets in the Community Online Tools (COT) system.

## Table of Contents

1. [Naming Conventions](#naming-conventions)
2. [Form Development Patterns](#form-development-patterns)
3. [Common Patterns](#common-patterns)
4. [UIActionManager Usage](#uiactionmanager-usage)
5. [Best Practices](#best-practices)

---

## Naming Conventions

### Event Handler Methods

**Standard:** Use `OnClick_*` and `OnChange_*` prefixes for all event handlers.

```c
// Button clicks, checkbox toggles
void OnClick_Refresh( UIEvent eid, UIActionBase action )
void OnClick_Delete( UIEvent eid, UIActionBase action )
void OnClick_Save( UIEvent eid, UIActionBase action )

// Sliders, text inputs, selection boxes
void OnChange_Filter( UIEvent eid, UIActionBase action )
void OnChange_UpdateRate( UIEvent eid, UIActionBase action )
void OnChange_PlayerName( UIEvent eid, UIActionBase action )
```

**Rationale:** More descriptive than `Click_*` / `Change_*`, matches DayZ convention, clearer intent.

### Module Member Variable

**Standard:** Always use `private` visibility for the module reference.

```c
class JMMyForm: JMFormBase
{
    private JMMyModule m_Module;  // Always private
    
    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }
}
```

**Rationale:** Module should not be accessed by subclasses (none exist), proper encapsulation.

### Widget Member Variables

**Standard:** Use descriptive names with widget type suffix: `m_<Purpose><WidgetType>`

```c
// Good examples
private UIActionButton m_RefreshButton;
private UIActionSlider m_HealthSlider;
private UIActionCheckbox m_GodModeCheckbox;
private UIActionEditableText m_PlayerNameInput;
private UIActionSelectBox m_SpawnModeSelect;
private UIActionScroller m_MainScroller;

// Avoid type prefixes
private UIActionButton m_btn_Refresh;      // Don't use prefixes
private UIActionSlider m_sldr_Health;      // Type is clear from declaration
private UIActionCheckbox m_chkbx_GodMode;  // Less readable
```

**Rationale:** Type is clear from declaration, more readable, matches most existing forms.

---

## Form Development Patterns

### Basic Form Structure

```c
class JMMyForm: JMFormBase
{
    // Module reference (always private)
    private JMMyModule m_Module;
    
    // UI widgets
    private UIActionScroller m_MainScroller;
    private Widget m_ContentWrapper;
    private UIActionButton m_RefreshButton;
    
    // Required: Link form to typed module
    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }
    
    // Build static UI once
    override void OnInit()
    {
        m_MainScroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_ContentWrapper = m_MainScroller.GetContentWidget();
        
        Widget toolbar = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 2 );
        m_RefreshButton = UIActionManager.CreateButton( toolbar, "Refresh", this, "OnClick_Refresh" );
        
        m_MainScroller.UpdateScroller();
    }
    
    // Called every time form becomes visible
    override void OnShow()
    {
        m_Module.RequestData();
    }
    
    // Update permission-gated controls
    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();
        
        m_RefreshButton.UpdatePermission( "MyModule.Refresh" );
    }
    
    // Event handlers
    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;
        
        m_Module.RequestData();
    }
}
```

### Form Lifecycle Methods

| Method | When Called | Purpose |
|--------|-------------|---------|
| `SetModule(mdl)` | On construction | Link to typed module, return true if cast OK |
| `OnInit()` | Once | Build all static UI elements |
| `OnShow()` | Every time form becomes visible | Request data, refresh state |
| `OnHide()` | Every time form is hidden | Cleanup, stop timers |
| `OnClientPermissionsUpdated()` | Player permissions changed | Update permission-gated controls |
| `OnSettingsUpdated()` | Module data updated | Rebuild dynamic content |
| `OnResize(w, h)` | Window resized | Update scroller, adjust layout |

---

## Common Patterns

### 1. Confirmation Dialog with Data Storage

Use a member variable to store data between button click and confirmation callback.

```c
class JMMyForm: JMFormBase
{
    private JMMyModule m_Module;
    private ref MyButtonData m_TempData;  // Store data for confirmation
    
    void OnClick_Delete( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;
        
        // Store data from button
        if ( !Class.CastTo( m_TempData, action.GetData() ) )
            return;
        
        // Show confirmation dialog
        CreateConfirmation_Two( 
            JMConfirmationType.INFO, 
            "Delete Item", 
            "Are you sure you want to delete " + m_TempData.ItemName + "?",
            "#STR_COT_GENERIC_CANCEL", 
            "#STR_COT_GENERIC_YES", 
            "", 
            "OnConfirmation_Delete" 
        );
    }
    
    void OnConfirmation_Delete( JMConfirmation confirmation )
    {
        if ( !m_TempData )
            return;
        
        // Use stored data
        m_Module.Delete( m_TempData.ItemID );
        
        // Refresh UI
        m_Module.Load();
    }
}
```

### 2. Permission Gating

Always check permissions in `OnClientPermissionsUpdated()`.

```c
override void OnClientPermissionsUpdated()
{
    super.OnClientPermissionsUpdated();

    // Use UpdatePermission() for automatic enable/disable
    m_DeleteButton.UpdatePermission( "MyModule.Delete" );
    m_AdminButton.UpdatePermission( "MyModule.Admin" );
    m_SpawnButton.UpdatePermission( "MyModule.Spawn" );
    
    // Or manually check permissions
    bool canEdit = GetPermissionsManager().HasPermission( "MyModule.Edit" );
    m_EditButton.SetEnabled( canEdit );
}
```

### 3. Dynamic Content Rebuild

Standard pattern for rebuilding dynamic UI sections.

```c
override void OnSettingsUpdated()
{
    // Delete old content
    if ( m_DynamicContent )
        delete m_DynamicContent;
    
    // Create new container
    m_DynamicContent = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );
    
    // Populate with data
    array<ref MyData> items = m_Module.GetItems();
    foreach ( MyData item : items )
    {
        Widget row = UIActionManager.CreateGridSpacer( m_DynamicContent, 1, 2 );
        
        UIActionManager.CreateText( row, item.Name );
        
        UIActionButton btn = UIActionManager.CreateButton( row, "Delete", this, "OnClick_Delete" );
        btn.SetData( new MyButtonData( item.ID ) );
    }
    
    // Update scroller after adding/removing widgets
    m_MainScroller.UpdateScroller();
}
```

### 4. Event Handler Pattern

Always check event type first, then validate data.

```c
void OnClick_MyButton( UIEvent eid, UIActionBase action )
{
    // Always check event type first
    if ( eid != UIEvent.CLICK )
        return;
    
    // Get and validate data if needed
    MyButtonData data;
    if ( !Class.CastTo( data, action.GetData() ) )
        return;
    
    // Perform action
    m_Module.DoSomething( data.Value );
}

void OnChange_MySlider( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE )
        return;
    
    float value = action.GetCurrent();
    m_Module.UpdateValue( value );
}
```

---

## UIActionManager Usage

### Creating Widgets

```c
// Buttons
UIActionButton btn = UIActionManager.CreateButton( parent, "Label", this, "OnClick_Handler" );
btn.SetColor( COLOR_RED );  // Optional styling

// Checkboxes
UIActionCheckbox cb = UIActionManager.CreateCheckbox( parent, "Label", this, "OnClick_Handler", false );
cb.SetChecked( true );

// Sliders
UIActionSlider slider = UIActionManager.CreateSlider( parent, "Label", 0.0, 100.0, this, "OnChange_Handler" );
slider.SetCurrent( 50.0 );
slider.SetStepValue( 1.0 );
slider.SetFormat( "#STR_COT_FORMAT_NONE" );

// Selection boxes
array<string> options = { "Option 1", "Option 2", "Option 3" };
UIActionSelectBox select = UIActionManager.CreateSelectionBox( parent, "Label", options, this, "OnChange_Handler" );
select.SetSelection( 0, false );

// Text inputs
UIActionEditableText input = UIActionManager.CreateEditableText( parent, "Label", this, "OnChange_Handler" );
input.SetText( "Default value" );
input.SetOnlyNumbers( true, false );  // Numbers only, allow decimals

// Text display
UIActionText txt = UIActionManager.CreateText( parent, "Label", "Value" );

// Scrollers
UIActionScroller scroller = UIActionManager.CreateScroller( parent );
Widget content = scroller.GetContentWidget();
// ... add widgets to content ...
scroller.UpdateScroller();  // MUST call after adding/removing widgets
```

### Layout Containers

```c
// Grid spacer (rows × columns)
GridSpacerWidget grid = UIActionManager.CreateGridSpacer( parent, 2, 3 );  // 2 rows, 3 columns

// Wrap spacer (auto-wrapping)
WrapSpacerWidget wrap = UIActionManager.CreateWrapSpacer( parent );

// Panel with background color
Widget panel = UIActionManager.CreatePanel( parent, 0x80000000, 50.0 );  // Color, height
```

### Attaching Data to Widgets

```c
// Define data class
class MyButtonData: UIActionData
{
    string ItemID;
    string ItemName;
    
    void MyButtonData( string id, string name )
    {
        ItemID = id;
        ItemName = name;
    }
}

// Attach data to button
UIActionButton btn = UIActionManager.CreateButton( parent, "Delete", this, "OnClick_Delete" );
btn.SetData( new MyButtonData( "item_123", "My Item" ) );

// Retrieve data in handler
void OnClick_Delete( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;
    
    MyButtonData data;
    if ( Class.CastTo( data, action.GetData() ) )
    {
        m_Module.Delete( data.ItemID );
    }
}
```

---

## Best Practices

### 1. Always Update Scrollers

After adding or removing widgets from a scroller's content, always call `UpdateScroller()`.

```c
m_MainScroller.UpdateScroller();  // Required after widget changes
```

### 2. Use Localization Strings

Always use localization string keys for user-facing text.

```c
// Good
UIActionManager.CreateButton( parent, "#STR_COT_GENERIC_REFRESH", this, "OnClick_Refresh" );

// Avoid hardcoded strings
UIActionManager.CreateButton( parent, "Refresh", this, "OnClick_Refresh" );
```

### 3. Check Event Types

Always check the event type in handlers to avoid unexpected behavior.

```c
void OnClick_MyButton( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )  // Always check first
        return;
    
    // Handle click
}
```

### 4. Null Check Widget Casts

Always null-check when casting widgets or data.

```c
MyButtonData data;
if ( !Class.CastTo( data, action.GetData() ) )
    return;  // Data is null or wrong type
```

### 5. Clean Up Dynamic Content

Always delete old dynamic content before rebuilding.

```c
if ( m_DynamicContent )
    delete m_DynamicContent;

m_DynamicContent = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );
```

### 6. Use Color Constants

Use predefined color constants for consistency.

```c
btn.SetColor( COLOR_RED );      // Defined in JMConstants
btn.SetColor( COLOR_GREEN );
btn.SetColor( COLOR_BLUE );
btn.SetColor( COLOR_YELLOW );
```

### 7. Widget Sizing

Use fractional widths for responsive layouts.

```c
UIActionButton btn = UIActionManager.CreateButton( parent, "Label", this, "OnClick_Handler", 0.5 );  // Half width
```

---

## Example: Complete Form

```c
class JMExampleForm: JMFormBase
{
    private JMExampleModule m_Module;
    
    private UIActionScroller m_MainScroller;
    private Widget m_ContentWrapper;
    private Widget m_DynamicContent;
    
    private UIActionButton m_RefreshButton;
    private UIActionButton m_DeleteButton;
    private UIActionEditableText m_SearchInput;
    private UIActionSelectBox m_FilterSelect;
    
    private ref MyButtonData m_TempData;
    
    protected override bool SetModule( JMRenderableModuleBase mdl )
    {
        return Class.CastTo( m_Module, mdl );
    }
    
    override void OnInit()
    {
        m_MainScroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
        m_ContentWrapper = m_MainScroller.GetContentWidget();
        
        // Toolbar
        Widget toolbar = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 3 );
        m_RefreshButton = UIActionManager.CreateButton( toolbar, "#STR_COT_GENERIC_REFRESH", this, "OnClick_Refresh" );
        m_SearchInput = UIActionManager.CreateEditableText( toolbar, "#STR_COT_GENERIC_SEARCH", this, "OnChange_Search" );
        
        array<string> filterOptions = { "All", "Active", "Inactive" };
        m_FilterSelect = UIActionManager.CreateSelectionBox( toolbar, "Filter", filterOptions, this, "OnChange_Filter" );
        
        m_MainScroller.UpdateScroller();
    }
    
    override void OnShow()
    {
        m_Module.Load();
    }
    
    override void OnClientPermissionsUpdated()
    {
        super.OnClientPermissionsUpdated();
        
        m_RefreshButton.UpdatePermission( "Example.View" );
        m_DeleteButton.UpdatePermission( "Example.Delete" );
    }
    
    override void OnSettingsUpdated()
    {
        if ( m_DynamicContent )
            delete m_DynamicContent;
        
        m_DynamicContent = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );
        
        array<ref MyData> items = m_Module.GetItems();
        foreach ( MyData item : items )
        {
            Widget row = UIActionManager.CreateGridSpacer( m_DynamicContent, 1, 2 );
            
            UIActionManager.CreateText( row, item.Name );
            
            UIActionButton deleteBtn = UIActionManager.CreateButton( row, "#STR_COT_GENERIC_DELETE", this, "OnClick_Delete" );
            deleteBtn.SetData( new MyButtonData( item.ID, item.Name ) );
            deleteBtn.SetColor( COLOR_RED );
        }
        
        m_MainScroller.UpdateScroller();
    }
    
    override void OnResize( float w, float h )
    {
        if ( m_MainScroller )
            m_MainScroller.UpdateScroller();
    }
    
    void OnClick_Refresh( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;
        
        m_Module.Load();
    }
    
    void OnClick_Delete( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CLICK )
            return;
        
        if ( !Class.CastTo( m_TempData, action.GetData() ) )
            return;
        
        CreateConfirmation_Two( 
            JMConfirmationType.INFO, 
            "#STR_COT_GENERIC_CONFIRM", 
            "Delete " + m_TempData.ItemName + "?",
            "#STR_COT_GENERIC_CANCEL", 
            "#STR_COT_GENERIC_YES", 
            "", 
            "OnConfirmation_Delete" 
        );
    }
    
    void OnConfirmation_Delete( JMConfirmation confirmation )
    {
        if ( !m_TempData )
            return;
        
        m_Module.Delete( m_TempData.ItemID );
        m_Module.Load();
    }
    
    void OnChange_Search( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;
        
        m_Module.SetSearchFilter( action.GetText() );
        OnSettingsUpdated();
    }
    
    void OnChange_Filter( UIEvent eid, UIActionBase action )
    {
        if ( eid != UIEvent.CHANGE )
            return;
        
        m_Module.SetFilter( action.GetSelection() );
        OnSettingsUpdated();
    }
}
```

---

## Inline Row Layout Pattern

### Problem

`CreateWrapSpacerCompact` wraps children to a new line the moment they exceed the row width, which causes the delete button, label and spawn button to stack vertically instead of sitting on one line.

### Rule

Use **`CreateWrapSpacer`** (not `Compact`) for rows whose children must stay on the same line. Then assign fractional widths to the text and button children so the total never exceeds 1.0, and use `SetFixedSize` for any icon-only element whose width is pixel-exact.

```
CreateWrapSpacer   — children share the row; wrapping only if truly required
CreateWrapSpacerCompact — tighter spacing but wraps aggressively; only use for
                          toolbar-style rows where wrapping is acceptable
```

### Standard Entry Row Pattern

`[icon-only delete] [label text: 60%] [action button: 25%]`

```c
Widget row = UIActionManager.CreateWrapSpacer( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

UIActionConfirmInline delBtn = UIActionManager.CreateConfirmInline( row, "Delete", this, "OnClick_Delete" );
UIActionIconGrid.ApplyDeletePreset( delBtn );   // trash-can icon + red colour
delBtn.SetFixedSize( 28, 28 );                  // pixel-exact; does not participate in fraction math
delBtn.SetData( new MyButtonData( item.Name ) );

UIActionText label = UIActionManager.CreateText( row, "", item.Name );
label.SetWidth( 0.60 );                         // 60% of remaining row width
label.SetTextHAlign( UIActionHAlign.CENTER );
label.SetTextVAlign( UIActionVAlign.CENTER );

UIActionButton spawnBtn = UIActionManager.CreateButton( row, "Spawn", this, "OnClick_Spawn" );
spawnBtn.SetWidth( 0.25 );                      // 25%; total fractions ≤ 1.0
spawnBtn.SetData( new MyButtonData( item.Name ) );
```

**Width budget:** `SetFixedSize(28)` items consume absolute pixels; fractional `SetWidth` applies to the remaining width from the WrapSpacer's total. Keep the sum of all fractional widths ≤ 1.0 (there is no automatic clamping).

**Reference:** `JMLoadoutForm.c` and `JMCompensationsForm.c` — `OnSettingsUpdated()`.

---

## Icon-Only Toolbar Buttons

### Refresh Button

Use `CreateIconButton` instead of `CreateButton` for toolbar controls that need an icon and no label text. Pair with `SetFixedSize` to make it square.

```c
UIActionImageButton refreshBtn = UIActionManager.CreateIconButton( toolbar, JMConstants.ICON_CLOCKWISE, this, "OnClick_Refresh" );
refreshBtn.SetFixedSize( 28, 28 );
refreshBtn.SetTooltip( "Reload data from disk" );
```

**Spin animation on click:** `UIActionImageButton` overrides `TriggerSpin` and `Update` so calling `TriggerSpin(2)` on the returned instance spins the icon for 2 full revolutions. Cast using the concrete type:

```c
void OnClick_Refresh( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK )
        return;

    UIActionImageButton btn;
    if ( Class.CastTo( btn, action ) )
        btn.TriggerSpin( 2 );

    m_Module.Load();
}
```

> **Why the cast matters:** `TriggerSpin` is defined on `UIActionButton` but guards on `m_Icon` (the `action_icon` ImageWidget). `UIActionImageButton` uses `action_image` instead. The override in `UIActionImageButton` intercepts `TriggerSpin` before the guard check and drives `m_Image` directly.

**Reference:** `UIActionImageButton.c` — `TriggerSpin` / `Update` overrides.

---

## Additional Resources

- **JMFormBase**: `JM/COT/Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMFormBase.c`
- **UIActionBase**: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionBase.c`
- **UIActionManager**: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionManager.c`
- **Example Forms**: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/*/JM*Form.c`

---

*Last updated: 2026-05-11*
