# `WorkspaceWidget`

The root widget of the UI hierarchy. Owned by the engine — you don't
create it, you reach it via `g_Game.GetWorkspace()`. It's the factory
for new widgets.

## Class

```c
class WorkspaceWidget: Widget
{
    proto native external Widget CreateWidget(
        WidgetType type,
        int left, int top,
        int width, int height,
        WidgetFlags flags,
        int color,
        int sort,
        Widget parentWidget = NULL
    );

    proto native external Widget CreateWidgets(
        string layout,
        Widget parentWidget = NULL,
        bool immedUpdate = true
    );
};
```

## Accessing the workspace

```c
WorkspaceWidget ws = g_Game.GetWorkspace();
```

There's typically one workspace per game (covering the whole screen).
The HUD, menus, COT main window, and tooltips are all children of it.

## Creating widgets from a layout

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(
    "JM/COT/GUI/layouts/sidebar_menu.layout",
    /* parent */ null  // null = attach to workspace root
);
```

Returns the root widget of the layout — you then `FindAnyWidget` named
children. **Pass a non-null `parentWidget`** to attach to a specific
subtree instead of the workspace root.

## Creating widgets programmatically

```c
Widget panel = g_Game.GetWorkspace().CreateWidget(
    WidgetType.PanelWidgetTypeID,
    /* left */ 100, /* top */ 100,
    /* width */ 300, /* height */ 200,
    /* flags */ WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS
              | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE,
    /* color (ARGB) */ 0xFF333333,
    /* sort */ 0,
    /* parent */ parentWidget
);
```

Rarely used in COT — layout files are easier to maintain.

## `WidgetType` enum

The first argument to `CreateWidget`. See [[../reference/widget-type-ids]]
for the full list. Common ones:

```c
WidgetType.PanelWidgetTypeID
WidgetType.FrameWidgetTypeID
WidgetType.TextWidgetTypeID
WidgetType.ImageWidgetTypeID
WidgetType.ButtonWidgetTypeID
WidgetType.GridSpacerWidgetTypeID
```

## When you'd use `CreateWidget` over `CreateWidgets`

- You need to add a single widget without writing a layout file.
- You're generating a widget tree dynamically based on data
  (programmatically iterate, create children, position them).
- You're prototyping and don't want to spawn an editor.

For anything reusable or with more than 3–4 properties, write a layout
file instead.

## Layout immediate update

```c
g_Game.GetWorkspace().CreateWidgets(path, parent, /* immedUpdate */ false);
```

If you're going to make many changes to the new tree immediately after
creating it, set `immedUpdate = false` and call `widget.Update()` once
at the end to trigger a single layout pass instead of one per change.

## Workspace size

```c
float w, h;
g_Game.GetWorkspace().GetScreenSize(w, h);
```

Returns the monitor resolution. Useful for clamping window positions to
the screen.

## Engine truth

> Defined at lines 176–182 of `scripts/1_core/proto/enwidgets.c`. The
> `external` keyword on `CreateWidget` and `CreateWidgets` means these
> are C++ implementations that allocate memory the engine owns — script
> just holds a reference.
