# Active Window & Focus Roots

The **active window** is the root of controller / keyboard navigation.
Setting one tells the engine "this is where focus lives now"; the D-pad
and Tab key cycle focus among focusable descendants.

## API

```c
proto native bool SetActiveWindow(Widget w, bool resetFocus);
```

- `w` — the new root widget for navigation
- `resetFocus = true` — focus the first focusable child immediately
- Returns `true` on success (the widget can become active)

## Why it matters

Without an active window:
- D-pad navigation does nothing
- Tab key doesn't cycle focus
- Controllers can't use the UI

So forms that need to support gamepad / keyboard navigation MUST call
`SetActiveWindow` when they appear.

## Standard pattern

```c
override void OnShow()
{
    SetActiveWindow(layoutRoot, /* resetFocus */ true);
}
```

This is the COT convention: when a form becomes visible, claim it as
the active window. The first focusable widget gets focus automatically.

## Multiple forms open at once

`SetActiveWindow` is global — there's one active window at a time.
Switching between forms means calling `SetActiveWindow` on the new
form's root.

For COT's main window with multiple panels, the convention is:
- The main window root is the active window
- Sub-panels are not separate active windows
- D-pad navigation cycles among all focusable widgets in the main window

## Focusable widgets

D-pad / Tab cycles among widgets that:
- Are inputs (`Button`, `EditBox`, `Slider`, `CheckBox`)
- Don't have `NOFOCUS` flag set
- Are visible and enabled

The engine traverses the active window's tree in **layout order** —
roughly top-to-bottom, left-to-right.

## Skipping a widget in navigation

```
ButtonWidgetClass decorative {
 "no focus" 1     // or: nofocus 1
}
```

Or in script:

```c
widget.SetFlags(WidgetFlags.NOFOCUS);
```

The widget can still be clicked but won't be reached via Tab / D-pad.

## Combined with focus()

```c
SetActiveWindow(formRoot, /* resetFocus */ false);
SetFocus(m_FirstField);
```

`resetFocus = false` skips the auto-focus, then you explicitly set
focus to a specific widget.

## Pitfalls

### Forgetting to call on `OnShow`

If a form is hidden and re-shown, the active window may still be the
previous form. Controller users see "input doesn't go anywhere".
Always re-claim on `OnShow`.

### Setting active window to a non-input widget

`SetActiveWindow(plainPanel, true)` with no focusable children returns
without focus. D-pad still does nothing because there's nothing to focus
on.

### Modal + active window

`SetModal` blocks input outside the modal subtree, but doesn't change
the active window. If you show a modal, the navigation root stays on the
form below; D-pad navigation may still try to reach the form's widgets
and "succeed" (focus changes) but click won't reach. Combine both:

```c
SetModal(dialogRoot);
SetActiveWindow(dialogRoot, true);
// On dismiss:
SetModal(NULL);
SetActiveWindow(formRoot, true);   // restore previous
```

## Engine truth

> Declared at lines 694–698 of `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native bool SetActiveWindow(Widget w, bool resetFocus);
> proto native void SetFocus(Widget w);
> proto native Widget GetFocus();
> ```
