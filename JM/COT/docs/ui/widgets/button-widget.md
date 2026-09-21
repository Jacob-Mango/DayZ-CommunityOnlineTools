# `ButtonWidget`

A clickable widget with a 2-state visual (normal/pressed) and a text
label. Fires `OnClick` through the parent's event handler.

## Class

```c
class ButtonWidget extends UIWidget
{
    proto native bool GetState();
    proto native bool SetState(bool state);

    proto native void SetText(string text);
    proto void        GetText(out string text);

    proto native void SetTextOffset(float xoffset, float yoffset);
    proto native void SetTextHorizontalAlignment(int align);  // ALIGN_CENTER, ALIGN_LEFT, ALIGN_RIGHT
    proto native void SetTextVerticalAlignment(int align);    // ALIGN_CENTER, ALIGN_TOP, ALIGN_BOTTOM

    proto native float GetTextProportion();
    proto native void  SetTextProportion(float val);
};
```

`UIWidget` adds `SetTextColor`, `SetTextOutline`, `SetTextShadow`, etc. —
see [[../styling/fonts-and-text]].

## Two-state visual

`GetState` / `SetState` toggle between two visual states defined by the
button's style. Many styles use this for "selected" toggles or for icons
that switch appearance when toggled on. Setting state does **not** fire
`OnClick` automatically — it's purely visual.

## Layout

```
ButtonWidgetClass my_btn {
 position 0 0
 size 1 30
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 1
 style DayZButton
 text "Click me"
 font "gui/fonts/sdf_MetronLight24"
 "exact text" 1
 "exact text size" 14
 "text halign" center
 "text valign" center
}
```

## Receiving clicks

Set a `ScriptedWidgetEventHandler` on the **parent** (or on the button
itself); the engine calls `OnClick(w, x, y, button)` on the handler when
the button is pressed:

```c
class MyForm: ScriptedWidgetEventHandler
{
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_MyButton)
        {
            // handle click
            return true;
        }
        return false;
    }
}

// elsewhere:
m_MyButton.GetParent().SetHandler(this);
```

Return `true` to stop the event propagation up the tree; `false` to let
the parent's handler also see it.

## Visual styles in COT

COT wraps `ButtonWidget` in `UIActionButton` (`UIActionButton.layout`)
which adds:
- A coloured fill panel
- An optional icon
- A label text widget
- Border panels for hover effect
- A pending-confirm overlay (for `ConfirmInline`)

Use `UIActionManager.CreateButton(parent, "label", instance, "OnClick_X")`
to get this style; raw `ButtonWidgetClass` only when you need vanilla styling.

## Hiding the button text

```
"text proportion" 0
```

Reduces text size to zero — text doesn't render, but the button stays
the same height. Useful for icon-only buttons where the text is empty
but you still want consistent sizing.

## Disabling

`btn.Enable(false)` greys out the button and stops it firing clicks.
COT's `UIActionBase.Disable()` adds a darker overlay panel for stronger
disabled state.

## Alignment constants

For `SetTextHorizontalAlignment` / `SetTextVerticalAlignment`:

| Constant | Value (engine) | Meaning |
|---|---|---|
| `ALIGN_LEFT` | 0 | Left / top |
| `ALIGN_CENTER` | 1 | Centre |
| `ALIGN_RIGHT` | 2 | Right / bottom |

(These are global int constants. The horizontal alignment uses LEFT /
CENTER / RIGHT; the vertical reuses the same constants with semantic
TOP / CENTER / BOTTOM meaning.)

## Engine truth

> Defined at lines 381–405 of `scripts/1_core/proto/enwidgets.c`.
