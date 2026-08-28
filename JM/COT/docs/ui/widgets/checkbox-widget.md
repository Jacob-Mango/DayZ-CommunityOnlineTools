# `CheckBoxWidget`

The **vanilla** boolean toggle. A box that the user can check/uncheck.
Fires `OnChange` (NOT `OnClick`) when toggled.

## Class

```c
class CheckBoxWidget extends UIWidget
{
    proto native void SetText(string str);
    proto native bool IsChecked();
    proto native void SetChecked(bool checked);
};
```

## Layout

```
CheckBoxWidgetClass auto_walk {
 ignorepointer 0
 position 0 0
 size 32 30
 hexactpos 1
 vexactpos 1
 hexactsize 1
 vexactsize 1
 style Editor
 checked 0
 text "Auto walk"
}
```

## Behaviour

- Clicking toggles `checked`.
- Fires `OnChange(w, x, y, finished)` — `finished` is always `true` for
  checkboxes (the value is committed immediately, no drag-and-release).
- The text label is part of the widget; positioned by the style.

## When NOT to use

The vanilla `CheckBoxWidget` has limited visual customisation (tied to
the style XML). COT replaces it with `UIActionCheckbox` (which is itself
a thin wrapper over `UIActionToggle`) for consistent look across the mod.

> **Convention:** in COT, always use
> `UIActionManager.CreateCheckbox(parent, label, instance, "OnClick_X", checked)`
> instead of raw `CheckBoxWidgetClass`. The handler signature is
> `OnClick_X(UIEvent eid, UIActionBase action)` with `eid == UIEvent.CHANGE`
> when the toggle flips.

## Visual states

Styles control the visual:
- `style Default` — DayZ's default rounded checkbox
- `style Editor` — the editor's square checkbox

Neither matches modern app aesthetics; this is why COT uses the custom
`UIActionToggle` (square fill + outline) instead.

## Engine truth

> Defined at lines 418–423 of `scripts/1_core/proto/enwidgets.c`.
> Only 3 script methods — the rest of the visual lives in the style XML.
