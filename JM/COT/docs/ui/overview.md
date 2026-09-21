# Widget System Overview

DayZ's UI is a hierarchy of **widgets** — C++ objects exposed to script as
`Widget` and its subclasses. Each widget has a position, size, colour,
optional content (text, image, video), and zero or more child widgets.

## The four moving parts

### 1. Widgets (the tree)

Every widget descends from `Widget` (defined in
`scripts/1_core/proto/enwidgets.c`). The tree is rooted at a
`WorkspaceWidget` provided by the engine — accessed in script via
`g_Game.GetWorkspace()`.

```
WorkspaceWidget
├── menu layouts (main menu, inventory, HUD…)
├── COT main window
│   ├── sidebar
│   └── form (built from a .layout file)
└── tooltip layer
```

### 2. Layout files (`.layout`)

A `.layout` is a plain-text declarative description of a widget subtree.
The engine parses it and instantiates the corresponding widgets, applying
every property you set. See [[layout-file-format]].

### 3. Script classes (`scriptclass`)

Any widget node in a layout can declare `scriptclass "MyClass"`. When the
layout is loaded, the engine constructs an instance of `MyClass` and
attaches it to the widget. Retrieve it with `widget.GetScript(out script)`.
COT uses this for every `UIAction*` (e.g.
`UIActionCheckbox.layout` → `scriptclass "UIActionCheckbox"`).

### 4. Event handler (`ScriptedWidgetEventHandler`)

A separate class type that receives mouse, keyboard, drag, and focus
callbacks for a widget. Attach with `widget.SetHandler(handler)`. The
`scriptclass` and the handler are independent — you can have either,
both, or neither. See [[events/event-handler]].

## Loading a layout from script

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(
    "JM/COT/GUI/layouts/sidebar_menu.layout",
    parentWidget  // optional
);
```

`CreateWidgets` returns the root widget of the layout. Children are
accessible via `root.FindAnyWidget("named_child")`.

## Finding widgets

| Method | Behaviour |
|---|---|
| `FindWidget(path)` | Find by **path** like `"row1.col2.label"` (dot-separated) |
| `FindAnyWidget(name)` | Find first descendant with that **name**, depth-first |
| `FindAnyWidgetById(id)` | Find by integer `UserID` set via `SetUserID` |

Names come from the **first identifier** after the widget class in the
layout file:

```
PanelWidgetClass my_panel { ... }
                 ^^^^^^^^ this is the name
```

## Coordinate space

Every position and size is interpreted as either **relative to the parent**
(fraction in `[0, 1]`) or **pixel-exact** (logical pixels at the engine's
reference resolution). Which one applies depends on the `*EXACTPOS` /
`*EXACTSIZE` flags. See [[coordinate-system]].

## Rendering & input

The workspace ticks every frame:

1. **Update** — every widget with a `Widget.Update()` script override gets
   ticked; animation steppers run here.
2. **Layout** — the engine resolves positions/sizes based on alignment refs.
3. **Render** — visible widgets are drawn in `SetSort` order within each parent.
4. **Input** — the engine dispatches mouse/key events to widgets under the
   cursor / with focus, walking up the tree until something returns `true`.

## Lifetime

- Widgets created with `CreateWidget` or `CreateWidgets` are owned by their
  parent.
- `widget.Unlink()` destroys the widget **and all its children**.
- Replacing a section of UI = `Unlink` the old root, `CreateWidgets` a new one.
- Script classes attached via `scriptclass` are GC'd when their widget is
  unlinked.

## Engine truth

> The full vanilla widget API lives in
> `scripts/1_core/proto/enwidgets.c` (718 lines). Every class in
> [widgets/](widgets/) corresponds to one declaration in that file.
