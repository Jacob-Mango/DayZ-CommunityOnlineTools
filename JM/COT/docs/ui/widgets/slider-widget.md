# `SliderWidget`

A value picker with a draggable handle. Continuous or step-based values
between configurable min and max.

## Class

```c
class SliderWidget extends UIWidget
{
    proto native void SetMinMax(float minimum, float maximum);
    proto native float GetMin();
    proto native float GetMax();
    proto native float GetCurrent();
    proto native void  SetCurrent(float curr);
    proto native float GetStep();
    proto native void  SetStep(float step);
};
```

## Layout

```
SliderWidgetClass quantity {
 position 0 0
 size 1 24
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 style Default
 min 0
 max 100
 current 50
 step 1
}
```

## Behaviour

- The user drags the handle or clicks the track.
- Fires `OnChange(w, x, y, finished)`:
  - `finished = false` during the drag (every position update)
  - `finished = true` when the user releases the mouse
- `SetCurrent(v)` clamps `v` to `[min, max]` and snaps to the nearest
  step. So setting `current = 73` with `step = 5` results in `75`.

## Reading the value

```c
float v = m_Slider.GetCurrent();
```

Always returns the snapped value (already step-aligned).

## Live vs final-value semantics

```c
override bool OnChange(Widget w, int x, int y, bool finished)
{
    if (w != m_Slider) return false;

    if (finished)
    {
        // Commit on drop — send RPC, save preference, etc.
        SendUpdateRPC();
    }
    else
    {
        // Live preview while dragging — update local visuals only.
        UpdatePreview(m_Slider.GetCurrent());
    }
    return true;
}
```

This pattern avoids spamming the server with intermediate values.

## COT wrapper: `UIActionSlider`

`UIActionSlider.layout` wraps the vanilla slider with:
- A label widget to the left (`SetLabel(name)`)
- A value readout to the right (formatted via `SetFormat`)
- A coloured fill bar overlay
- The standard disabled overlay

`UIActionManager.CreateSlider(parent, "label", min, max, instance, "OnChange_X")`
is the COT way.

## Style names

- `style Default` — DayZ's default horizontal slider
- `style Vertical` — uncommon, vertical slider

Sliders inherit styling from the `dayzwidgets.styles` definitions.

## Engine truth

> Defined at lines 358–367 of `scripts/1_core/proto/enwidgets.c`.
