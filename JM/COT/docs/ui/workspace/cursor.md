# Cursor Control

DayZ exposes the OS cursor through script — visibility, custom image,
and current widget under cursor.

## Show / hide the cursor

```c
ShowCursorWidget(true);   // show
ShowCursorWidget(false);  // hide
```

While a UI menu is open (inventory, ESC menu, COT main window), the
engine usually shows the cursor automatically. For HUDs or sub-modes
where you want the cursor visible without a full menu, call manually.

## Setting a custom cursor image

```c
SetCursorWidget(myCursorWidget);
```

The `myCursorWidget` should be a widget (typically an `ImageWidget`)
that gets re-positioned to follow the cursor. Once set, the engine moves
this widget to the cursor's screen position every frame.

To revert to the default OS cursor:

```c
SetCursorWidget(NULL);
ShowCursorWidget(false);
ShowCursorWidget(true);   // re-show with default cursor
```

## Reading the widget under the cursor

```c
Widget under = GetWidgetUnderCursor();
```

Returns the topmost non-`IGNOREPOINTER` widget the cursor is currently
over. Useful for tooltips, custom hover effects, or "what would I click
right now?" logic.

```c
override void Update(float timeSlice)
{
    Widget under = GetWidgetUnderCursor();
    if (under == m_TargetButton)
        StartHighlight();
    else
        StopHighlight();
}
```

## Reading current cursor position

DayZ uses `GetMousePos`:

```c
int x, y;
GetMousePos(x, y);   // screen pixels
```

Returns the cursor's screen coords as integers. Use to position
follow-cursor UI like tooltips.

## Reporting cursor to a specific widget root

For multi-workspace setups (rare in DayZ), you can manually report
mouse events:

```c
ReportMouse(/* mousex */ x, /* mousey */ y, /* root */ specificWidget);
```

The engine then dispatches enter/leave events as if the cursor is at
`(x, y)` for the given subtree. Used by the editor's multi-window setup.

## Cursor in the game world

Inside the actual game (not in a menu), the cursor is hidden by default.
The engine controls visibility based on UI state. If you want a custom
cursor visible during gameplay, you'll need a frame-by-frame
`ShowCursorWidget(true)` to override the engine's hide calls.

## Pitfalls

### `GetMousePos` requires the engine to know about the cursor

If the cursor is hidden and the engine has marked the input as "in-world",
`GetMousePos` may return stale values. Reliable only when a UI menu is
visible.

### Custom cursor widget needs the right flags

If your cursor widget intercepts clicks, it blocks the real underlying
widget from receiving them. Set `IGNOREPOINTER` on the cursor widget:

```
ImageWidgetClass cursor {
 ignorepointer 1
 image "MyMod/textures/cursor.paa"
 size 24 24
}
```

### Don't call ShowCursorWidget every frame

Each call is a state transition. Set once when the mode changes, not
in `Update`.

## Engine truth

> Cursor proto declarations at lines 687–693 of
> `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native void SetCursorWidget(Widget cursor);
> proto native void ShowCursorWidget(bool show);
> proto native Widget GetWidgetUnderCursor();
> proto native Widget CancelWidgetDragging();
> proto native Widget GetDragWidget();
> proto native void ReportMouse(int mousex, int mousey, Widget rootWidget);
> ```
