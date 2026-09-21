# Mouse Events

Every mouse interaction routes through the event handler. Below is the
flow per gesture and what to override.

## Click vs button-down/up

The engine fires three events on a left-click:

1. `OnMouseButtonDown(w, x, y, button)` — finger goes down
2. `OnMouseButtonUp(w, x, y, button)` — finger comes up
3. `OnClick(w, x, y, button)` — only if down and up were on the same widget

For most cases, override `OnClick`. Override the button-down/up pair when
you need to track press-and-hold or distinguish from drags.

## Button codes

| `button` value | Mouse button |
|---|---|
| `0` | Left |
| `1` | Right |
| `2` | Middle |
| `3` | Side button 1 |
| `4` | Side button 2 |

```c
override bool OnClick(Widget w, int x, int y, int button)
{
    if (button == 0)
    {
        // left click
    }
    else if (button == 1)
    {
        // right click — typically context menu
    }
    return true;
}
```

## Double-click

```c
override bool OnDoubleClick(Widget w, int x, int y, int button)
{
    // Triggers when two clicks happen within the engine's double-click window
    // (~300ms). `OnClick` also fires for both clicks — `OnDoubleClick` is fired
    // *after* the second `OnClick`.
}
```

Common pattern: single click selects, double click confirms / opens.

## Hover (enter / leave)

```c
override bool OnMouseEnter(Widget w, int x, int y)
{
    if (w == m_Button)
    {
        ShowHighlight();
    }
    return false;
}

override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
{
    if (w == m_Button)
    {
        HideHighlight();
    }
    return false;
}
```

`enterW` (in `OnMouseLeave`) is the widget being entered next. Use it to
detect "moving from button A to button B" without seeing a flash of
de-hover.

> **Hover and children:** entering a child widget fires `OnMouseLeave`
> on the parent (if the child isn't `IGNOREPOINTER`) — this is sometimes
> surprising. Set `ignorepointer 1` on decorative children to suppress.

## Wheel

```c
override bool OnMouseWheel(Widget w, int x, int y, int wheel)
{
    if (w == m_PreviewImage)
    {
        ZoomBy(wheel * 0.1);
        return true;
    }
    return false;
}
```

`wheel` is `+1` (scroll up / away) or `-1` (scroll down / toward). The
engine doesn't expose multi-step deltas — repeated events fire for fast
scrolling.

## Cursor position

Use `GetMousePos(out int x, out int y)` (global) for absolute screen
coords outside an event handler.

Inside event handlers, `(x, y)` are screen coords already. To convert to
local widget coords:

```c
float screenX, screenY;
w.GetScreenPos(screenX, screenY);
int localX = x - (int)screenX;
int localY = y - (int)screenY;
```

## Hit-testing manually

```c
Widget under = GetWidgetUnderCursor();
```

Returns the topmost non-`IGNOREPOINTER` widget under the mouse.
`GetDragWidget()` returns the currently-dragged widget (or null).

## Click-through with `IGNOREPOINTER`

Set on decorative children to pass clicks through to the parent.

```
TextWidgetClass title {
 ignorepointer 1
 text "Section"
}
```

Without this, clicking the title text doesn't reach the button below it
in z-order.

## Cancelling drags

```c
Widget cancelled = CancelWidgetDragging();
```

Aborts any in-progress drag. Returns the widget that was being dragged.

## Pitfalls

### `OnClick` not firing on a widget with a child

If a child widget under the cursor has its own handler that returns
`true` from `OnClick`, the parent's `OnClick` doesn't fire. Either:
- Return `false` from the child to let the parent see it too, or
- Handle the click at the deepest widget that needs it.

### Double-click vs two clicks

`OnDoubleClick` fires **in addition to** the second `OnClick`. If you
have logic in both, you'll get both — guard one or the other.

### Wheel events on non-scrollable widgets

`OnMouseWheel` fires on whichever widget is under the cursor. If your
parent scroller wants the wheel and the cursor is over a child button,
the child gets it first. Either consume on the child or set
`IGNOREPOINTER` on the child to forward.

## Engine truth

> The dispatch logic isn't exposed to script; the engine's input system
> walks the widget tree under the cursor and calls handlers in order.
> Source: empirical observation against vanilla layouts.
