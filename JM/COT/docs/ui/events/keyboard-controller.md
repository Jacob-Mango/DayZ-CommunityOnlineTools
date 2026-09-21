# Keyboard & Controller Input

## Key events

```c
bool OnKeyDown(Widget w, int x, int y, int key);
bool OnKeyUp(Widget w, int x, int y, int key);
bool OnKeyPress(Widget w, int x, int y, int key);
```

- `OnKeyDown` — fires once when a key is pressed (no auto-repeat)
- `OnKeyUp` — fires once when a key is released
- `OnKeyPress` — fires repeatedly while held (auto-repeat rate from the OS)

`key` is an engine key code. The full enum is in
`scripts/3_game/keycode.c`.

## Key codes (common subset)

| Code | Constant | Key |
|---|---|---|
| 1 | KeyCode.KC_ESCAPE | Esc |
| 14 | KeyCode.KC_BACK | Backspace |
| 15 | KeyCode.KC_TAB | Tab |
| 28 | KeyCode.KC_RETURN | Enter |
| 57 | KeyCode.KC_SPACE | Space |
| 200 | KeyCode.KC_UP | Up arrow |
| 208 | KeyCode.KC_DOWN | Down arrow |
| 203 | KeyCode.KC_LEFT | Left arrow |
| 205 | KeyCode.KC_RIGHT | Right arrow |
| 211 | KeyCode.KC_DELETE | Del |

Use the constants from `KeyCode`, never raw integers.

## Where events go

Key events fire on the widget that has **focus**, not the widget under
the cursor. Setting focus is explicit (see [[focus]]).

```c
override bool OnKeyDown(Widget w, int x, int y, int key)
{
    if (key == KeyCode.KC_RETURN)
    {
        Submit();
        return true;
    }
    if (key == KeyCode.KC_ESCAPE)
    {
        Cancel();
        return true;
    }
    return false;
}
```

## Common pattern: input shortcut

For a form-wide shortcut, listen on the form's root widget:

```c
override void OnInit()
{
    layoutRoot.SetHandler(this);
}

override bool OnKeyPress(Widget w, int x, int y, int key)
{
    if (key == KeyCode.KC_F5)
    {
        DoRefresh();
        return true;
    }
    return false;
}
```

## Controller events

```c
bool OnController(Widget w, int control, int value);
```

Fires for D-pad, face buttons, sticks. `control` is a `ControlID`:

```c
enum ControlID
{
    CID_NONE        = 0,
    CID_SELECT      = 1,   // A / X button
    CID_BACK,              // B / Circle
    CID_LEFT,              // D-pad left
    CID_RIGHT,
    CID_UP,
    CID_DOWN,
    CID_MENU,              // Start
    CID_DRAG,              // Y / Triangle (probably)
    CID_TABLEFT,           // LB / L1
    CID_TABRIGHT,          // RB / R1
    CID_RADIALMENU,
    CID_COUNT
};
```

`value` semantics depend on control: digital buttons send `0`/`1`,
analogue triggers send `0..255`.

```c
override bool OnController(Widget w, int control, int value)
{
    if (control == ControlID.CID_TABRIGHT && value > 0)
    {
        NextTab();
        return true;
    }
    return false;
}
```

## Focus and controllers

`SetActiveWindow(widget, true)` makes a widget the controller-navigation
root. The D-pad / left stick moves focus among focusable children
(widgets where `IsFocusWidget` returns true and `NOFOCUS` flag is clear).

## Pitfalls

### Forgetting to consume the key

If your form handles `OnKeyDown` for Esc but returns `false`, the
parent's menu manager also sees it and may close the entire menu.
**Always `return true` for handled keys.**

### Text input and key events

When a `EditBoxWidget` has focus, the engine consumes printable-character
keystrokes for text entry; only non-printing keys (Esc, F-keys, arrows
when at line ends) bubble up to your handler.

### Controller key codes

Some controllers send synthetic key codes (e.g. the D-pad on Steam Input
masquerades as arrow keys). Test on real hardware.

## Engine truth

> Defined at lines 639–654 of `scripts/1_core/proto/enwidgets.c` for
> the `ControlID` enum; key callbacks at lines 672–674.
