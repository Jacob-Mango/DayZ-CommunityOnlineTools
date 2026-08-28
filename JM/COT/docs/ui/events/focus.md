# Focus Management

DayZ has two related but distinct concepts:

- **Focus** — the widget that receives keyboard / controller events.
- **Active window** — the navigation root from which D-pad / Tab moves
  focus between focusable children.

## Setting focus

```c
SetFocus(widget);              // give this widget input focus
Widget focused = GetFocus();   // currently focused widget
```

Focus moves automatically on:
- Mouse click on a focusable widget
- Tab key (engine handles cycling among focusable siblings)
- D-pad navigation when an active window is set

You explicitly call `SetFocus(NULL)` to clear focus.

## Focusable widgets

A widget is focusable if:
- It's an input widget (`EditBox`, `Button`, `Slider`, `CheckBox`, etc.)
- The `NOFOCUS` flag is **not** set
- It's visible and not disabled

To make a custom widget focusable, override `IsFocusWidget` in its
script class:

```c
class MyCustomWidget: UIActionBase
{
    override bool IsFocusWidget(Widget widget)
    {
        return widget == m_InnerButton;
    }
}
```

The default `UIActionBase.IsFocusWidget` returns `false`.

## Focus callbacks

```c
override bool OnFocus(Widget w, int x, int y)
{
    if (w == m_EditBox)
    {
        ShowSuggestions();
    }
    return false;
}

override bool OnFocusLost(Widget w, int x, int y)
{
    if (w == m_EditBox)
    {
        HideSuggestions();
    }
    return false;
}
```

Common uses: show/hide autocomplete dropdowns, commit edited values on
focus loss, change widget colour to indicate focus state.

## Setting the active window

```c
SetActiveWindow(myFormRoot, /* resetFocus */ true);
```

`resetFocus = true` automatically focuses the first focusable child.
Without an active window, controller D-pad navigation does nothing.

```c
override void OnShow()
{
    SetActiveWindow(layoutRoot, true);
}
```

The COT main window calls `SetActiveWindow` when displayed so the user
can navigate with a controller.

## Modal interactions

```c
SetModal(modalDialogRoot);
```

Blocks input to everything outside the modal. The modal's handler still
sees events; everything else doesn't. To dismiss, call `SetModal(NULL)`.

> COT's `JMConfirmation` system uses `SetModal` for the
> Yes/No / OK / Cancel popups.

## Pitfalls

### Focus visually lost after rebuild

If you `delete m_DynamicContent` and re-create it, the previously
focused widget no longer exists. Engine focus is now on a destroyed
widget — events stop firing. Re-call `SetFocus` on the new equivalent
widget, or `SetActiveWindow` to reset.

### Inputs not receiving keys

Common cause: the widget isn't actually focused. Click on it once or
call `SetFocus(widget)` explicitly.

### Controller navigation skipping a widget

Either the widget has `NOFOCUS` set, or it's not registered as a
focusable input class. Override `IsFocusWidget` and ensure the flag is
clear.

## Engine truth

> Focus / active-window / modal proto functions are declared at lines
> 695–702 of `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native bool SetActiveWindow(Widget w, bool resetFocus);
> proto native void SetFocus(Widget w);
> proto native void SetModal(Widget w);
> proto native Widget GetFocus();
> ```
