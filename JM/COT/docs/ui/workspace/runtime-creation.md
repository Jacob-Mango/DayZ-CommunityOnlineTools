# Creating Widgets at Runtime

Two ways to add widgets to the UI from script.

## `CreateWidgets` — from a layout file

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(
    "MyMod/GUI/layouts/my_form.layout",
    /* parent */ parentWidget,
    /* immedUpdate */ true
);
```

- Returns the root widget of the layout
- Layouts are cached on first load; subsequent loads are faster
- All `scriptclass` instances are constructed; their `OnInit` runs

This is the **standard** way to create UI in DayZ. Write layouts in files,
load them at runtime, configure with script.

## `CreateWidget` — programmatic single widget

```c
Widget panel = g_Game.GetWorkspace().CreateWidget(
    /* type */ WidgetType.PanelWidgetTypeID,
    /* left */ 0, /* top */ 0,
    /* width */ 100, /* height */ 30,
    /* flags */ WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS
              | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE,
    /* color */ 0xFF000000,
    /* sort */ 0,
    /* parent */ parentWidget
);
```

Returns the new widget. No `scriptclass`, no `OnInit`. You have to
configure everything from script.

Used for **truly dynamic** widgets (e.g. one canvas per particle in a
visual effect) where writing a layout is overkill.

## Which one to use

| Need | Use |
|---|---|
| Standard UI building block | `CreateWidgets` from a `.layout` |
| Custom one-off widget you'd never reuse | `CreateWidget` |
| Hundreds of trivial widgets (perf-sensitive) | `CreateWidget` (lower overhead per instance) |
| Anything with `scriptclass` | `CreateWidgets` (only way to wire the class) |

## Reading a `.layout`'s children

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(path, parent);
Widget btn  = root.FindAnyWidget("save_button");
```

`FindAnyWidget` does a depth-first search by widget name. The first
match wins.

## Performance tips

### Cache by-name lookups

```c
private Widget m_SaveBtn;

override void OnInit()
{
    m_SaveBtn = layoutRoot.FindAnyWidget("save_button");
}
```

Don't call `FindAnyWidget("save_button")` in a hot path; cache the
reference once in `OnInit`.

### Batch creation with `immedUpdate=false`

When creating many widgets back-to-back:

```c
Widget w1 = ws.CreateWidgets(p1, parent, /* immedUpdate */ false);
Widget w2 = ws.CreateWidgets(p2, parent, false);
Widget w3 = ws.CreateWidgets(p3, parent, false);
parent.Update();    // single re-layout
```

vs.

```c
Widget w1 = ws.CreateWidgets(p1, parent, true);   // triggers re-layout
Widget w2 = ws.CreateWidgets(p2, parent, true);   // triggers re-layout
Widget w3 = ws.CreateWidgets(p3, parent, true);   // triggers re-layout
```

The first form is faster for big batches.

### Re-use vs recreate

For dynamic lists, prefer **rebuilding the whole subtree** over
fine-grained updates:

```c
if (m_DynamicContent)
    delete m_DynamicContent;
m_DynamicContent = UIActionManager.CreateGridSpacer(m_ContentWrapper, 1, 1);
// add new rows
```

This is simpler and usually fast enough. Only optimise (incremental
updates) if you've measured a perf problem.

## Lifetime

Widgets created via `CreateWidget` / `CreateWidgets` are owned by their
parent. When the parent is `Unlink`'d, all children unlink too.

If you `CreateWidgets` with `parent = null`, the widget is attached to
the workspace root and lives until you explicitly `Unlink` it.

## Pitfalls

### Forgetting to `Unlink`

Workspace-attached widgets (no parent) **never get cleaned up
automatically**. They accumulate. Always `Unlink` widgets you don't need.

### Calling `delete` on a widget reference

```c
delete m_Widget;   // OK — calls ~Widget which Unlinks
```

This works because `Widget` extends `Managed`. But:

```c
m_Widget.Unlink();
delete m_Widget;   // CRASH — already destroyed
```

Use one or the other, not both.

### `CreateWidgets` returns null on file error

If the path is wrong, the engine logs an error and returns `null`. Always
guard:

```c
Widget root = ws.CreateWidgets(path, parent);
if (!root)
{
    Print("Failed to load layout: " + path);
    return;
}
```

## Engine truth

> `CreateWidget` and `CreateWidgets` at lines 179–181 of
> `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native external Widget CreateWidget(WidgetType type, int left, int top,
>                                            int width, int height, WidgetFlags flags,
>                                            int color, int sort, Widget parentWidget = NULL);
> proto native external Widget CreateWidgets(string layout, Widget parentWidget = NULL,
>                                             bool immedUpdate = true);
> ```
