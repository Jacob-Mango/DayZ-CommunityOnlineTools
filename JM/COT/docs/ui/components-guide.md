# COT UI Action Components Guide

A comprehensive developer reference documenting all `UIAction` components in Community Online Tools: when to use each control, API methods, parameters, `UIEvent` dispatch rules, and code examples.

---

## Overview

All form controls in COT extend `UIActionBase` and are instantiated through static factory methods on `UIActionManager`. 

### Key Principles
1. **Never use `new UIActionX()` directly.** Always construct controls using `UIActionManager.CreateX(...)`. `UIActionManager` instantiates the layout XML, binds the script handle, sets default handlers, and performs memory verification.
2. **Event Dispatch:** Interactive components trigger `CallEvent(UIEvent.EVENT_NAME)` when acted on by the user. Handlers receive `(UIEvent eid, UIActionBase action)`.
3. **Data Binding:** Pass target instance and method name (e.g. `this, "OnClick_Button"`) during creation.

---

## 1. Buttons & Triggers

### `UIActionButton`
* **Purpose:** Standard text button for triggering immediate commands (e.g., "Apply", "Reset", "Delete").
* **When to use:** Primary action triggers within cards or toolbar footers.
* **Factory:** `UIActionManager.CreateButton( parent, label, target, callback )`
* **API Methods:** `SetText(string label)`, `SetColor(int color)`, `SetEnabled(bool enable)`
* **Event:** `UIEvent.CLICK`
```c
UIActionButton btn = UIActionManager.CreateButton( card, "Heal Player", this, "OnClick_Heal" );

void OnClick_Heal( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CLICK ) return;
    // Execute action...
}
```

### `UIActionImageButton` / `UIActionImageButtonToggle`
* **Purpose:** Icon-only button or toggleable icon button (e.g., Lucide glyphs, trash icons, lock icons).
* **When to use:** Compact toolbars, table row actions, or header action buttons.
* **Factory:** `UIActionManager.CreateImageButton( parent, iconPath, target, callback )`
* **API Methods:** `SetIcon(string path)`, `SetToggle(bool state)`, `AnimateError()`
* **Event:** `UIEvent.CLICK` / `UIEvent.CHANGE`

### `UIActionConfirmInline`
* **Purpose:** Two-step destructive action button ("Delete" -> "Confirm Delete?").
* **When to use:** Destructive operations (clear inventory, ban player, wipe base) where accidental clicks must be prevented without a full modal dialog.
* **Factory:** `UIActionManager.CreateConfirmInline( parent, label, confirmLabel, target, callback )`
* **Event:** `UIEvent.CHANGE` (Fires only after the user clicks the secondary confirmation state).

---

## 2. Selection & Input Controls

### `UIActionInput` / `UIActionEditableText`
* **Purpose:** Single-line text input field with optional label and search/clear buttons.
* **When to use:** Name fields, GUID inputs, coordinate text boxes, search filters.
* **Factory:** `UIActionManager.CreateInput( parent, label, defaultText, target, callback )`
* **API Methods:** `GetText()`, `SetText(string val)`, `SetPlaceholder(string text)`
* **Event:** `UIEvent.CHANGE` (Fires on enter key or focus loss) / `UIEvent.VALUE` (Fires on character change)

### `UIActionSearchBox`
* **Purpose:** Specialized text input with built-in search icon and instant clear button (`X`).
* **When to use:** Top toolbar search bars above lists (e.g., Object Spawner class filter, Player Roster search).
* **Factory:** `UIActionManager.CreateSearchBox( parent, placeholder, target, callback )`
* **Event:** `UIEvent.CHANGE` / `UIEvent.VALUE`

### `UIActionEditableVector`
* **Purpose:** Triple numeric input fields for `vector(X, Y, Z)` coordinates or orientations.
* **When to use:** Teleport coordinates, object position/rotation controls in Map Editor.
* **Factory:** `UIActionManager.CreateEditableVector( parent, label, defaultVector, target, callback )`
* **API Methods:** `GetValue()`, `SetValue(vector pos)`
* **Event:** `UIEvent.CHANGE`

---

## 3. Sliders & Numeric Controls

### `UIActionSlider`
* **Purpose:** Continuous numeric range slider with an integrated live value label.
* **When to use:** Stat adjustments (Health, Blood, Water, Energy), time-of-day selection, ESP distance range.
* **Factory:** `UIActionManager.CreateSlider( parent, label, min, max, defaultVal, step, formatStr, target, callback )`
* **API Methods:** `GetValue()`, `SetValue(float val)`, `SetMinMax(float min, float max)`
* **Event:** `UIEvent.CHANGE` (Fires on mouse release) / `UIEvent.VALUE` (Fires continuously while dragging)
```c
m_HealthSlider = UIActionManager.CreateSlider( p, "Health", 0, 100, 100, 1, "%1 %", this, "OnChange_Health" );
```

### `UIActionSliderRange`
* **Purpose:** Dual-thumb range slider for min/max boundary selection.
* **When to use:** Filtering objects by minimum and maximum distance or quantity.
* **Factory:** `UIActionManager.CreateSliderRange( parent, label, min, max, target, callback )`
* **API Methods:** `GetLowValue()`, `GetHighValue()`, `SetLowHigh(float low, float high)`
* **Event:** `UIEvent.CHANGE`

### `UIActionSpinner`
* **Purpose:** Step numeric selector with increment (`+`) and decrement (`-`) buttons.
* **When to use:** Discrete integer counts (e.g., spawn item quantity, ammo count).
* **Factory:** `UIActionManager.CreateSpinner( parent, label, min, max, defaultVal, step, target, callback )`
* **Event:** `UIEvent.CHANGE`

---

## 4. Toggles & Checkboxes

### `UIActionToggle` / `UIActionCheckbox`
* **Purpose:** Boolean toggle switch or standard checkbox control.
* **When to use:** Setting flags (Godmode, Freeze, Invisibility, Enable Webhook, Auto-refuel).
* **Factory:** `UIActionManager.CreateToggle( parent, label, defaultState, target, callback )`
* **API Methods:** `IsChecked()`, `SetChecked(bool state)`
* **Event:** `UIEvent.CHANGE`
```c
m_GodmodeToggle = UIActionManager.CreateToggle( p, "Godmode", false, this, "OnToggle_Godmode" );

void OnToggle_Godmode( UIEvent eid, UIActionBase action )
{
    if ( eid != UIEvent.CHANGE ) return;
    bool enabled = m_GodmodeToggle.IsChecked();
    // Dispatch RPC...
}
```

---

## 5. Menus, Lists & Dropdowns

### `UIActionSelectBox` / `UIActionDropdown`
* **Purpose:** Single-selection dropdown list control.
* **When to use:** Selecting a weather preset, active spectate player, or camera bookmark.
* **Factory:** `UIActionManager.CreateSelectBox( parent, label, optionsArray, defaultIndex, target, callback )`
* **API Methods:** `GetSelection()`, `GetSelectionString()`, `SetSelections(array<string> options)`
* **Event:** `UIEvent.CHANGE`

### `UIActionContextMenu`
* **Purpose:** Floating right-click popup context menu.
* **When to use:** Right-click actions on player roster rows, vehicle map markers, inventory items, or ESP entities.
* **Factory:** `UIActionManager.CreateOverlayMenu( form, target, callback )`
* **API Methods:** `ClearItems()`, `AddItem(id, label, icon, color)`, `SetItemEnabled(id, bool)`, `OpenAt(x, y)`, `GetLastClickedId()`
* **Event:** `UIEvent.CLICK`
```c
m_Menu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_Menu" );
m_Menu.ClearItems();
m_Menu.AddItem( "heal", "Heal Player", JMConstants.Lucide("heart-pulse") );
m_Menu.AddItem( "tp", "Teleport Here", JMConstants.Lucide("footprints") );
m_Menu.OpenAt( mouseX, mouseY );
```

### `UIActionFilterMenu`
* **Purpose:** Multi-page drill-down dropdown filter menu (stack of named pages with back navigation and toggle-all).
* **When to use:** Roster filter dropdowns (Dead/Uncon/Hurt/Sick), Vehicle type filters, Teleport category filters.
* **Factory:** `UIActionManager.CreateOverlayFilterMenu( form, ownerWidget, registryScope )`
* **API Methods:** `AddPage(...)`, `AddToggleRow(...)`, `ToggleAt(ownerWidget)`
* **Event:** Intercepted by page callbacks (`BuildFn` / `ChangeFn`).

---

## 6. Containers, Layouts & Popups

### `UIActionCollapsibleSection`
* **Purpose:** Card container with a clickable header that expands/collapses content.
* **When to use:** Grouping form settings into logical cards (e.g., Player General Stats, Environment Controls).
* **Factory:** `UIActionManager.CreateCollapsibleSection( parent, title )`
* **API Methods:** `GetContainer()`, `SetExpanded(bool expand)`, `AddHeaderButton(...)`

### `UIActionFlexRow`
* **Purpose:** Horizontal flex layout container for arranging controls side-by-side.
* **When to use:** Placing "Save" and "Cancel" buttons side-by-side or pairing an input box with an action button.
* **Factory:** `UIActionManager.CreateFlexRow( parent )`
* **API Methods:** `GetContainer()`

### `UIActionValuePrompt`
* **Purpose:** Floating overlay prompt with a text box, slider, or dropdown for value input.
* **When to use:** Asking for scale values, ban duration, custom location name, or player message text.
* **Factory:** `UIActionManager.CreateOverlayPrompt( form, target, callback )`
* **API Methods:** `OpenText(...)`, `OpenSlider(...)`, `OpenSelect(...)`
* **Event:** `UIEvent.CONFIRM` / `UIEvent.CANCEL`

---

## Component Selection Matrix

| Use Case | Recommended Component | Event Type |
|---|---|---|
| Simple trigger action | `UIActionButton` | `UIEvent.CLICK` |
| Destructive operation | `UIActionConfirmInline` | `UIEvent.CHANGE` |
| Icon toolbar action | `UIActionImageButton` | `UIEvent.CLICK` |
| Boolean setting flag | `UIActionToggle` | `UIEvent.CHANGE` |
| Numeric stat value | `UIActionSlider` | `UIEvent.CHANGE` / `VALUE` |
| Single item selection | `UIActionSelectBox` | `UIEvent.CHANGE` |
| Right-click options | `UIActionContextMenu` | `UIEvent.CLICK` |
| Filter dropdown with pages | `UIActionFilterMenu` | Custom Callbacks |
| Grouping controls | `UIActionCollapsibleSection` | N/A |
| Side-by-side alignment | `UIActionFlexRow` | N/A |
