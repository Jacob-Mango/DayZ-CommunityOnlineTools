# `SimpleProgressBarWidget` / `ProgressBarWidget`

Visual fill bar from `min` to `max` showing a `current` value. Non-interactive
(unlike `SliderWidget`).

## Class

```c
class SimpleProgressBarWidget extends UIWidget
{
    proto native float GetMin();
    proto native float GetMax();
    proto native float GetCurrent();
    proto native void  SetCurrent(float curr);
};

class ProgressBarWidget extends SimpleProgressBarWidget
{
    // identical script API, more complex style internally
};
```

`ProgressBarWidget` is the same script-side; its style file declares many
more visual parts (left/center/right caps, top/bottom rails, fill caps,
etc.) so it can look like a fancy chrome'd bar instead of a flat rectangle.
`SimpleProgressBarWidget` has just centre fill + base.

## Layout

```
ProgressBarWidgetClass health_bar {
 position 0 0
 size 1 8
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 style DayZHealth
 min 0
 max 100
 current 100
}
```

## Style examples

From `gui/looknfeel/dayzwidgets.styles`:

- `DayZLoading` — main-menu loading bar
- `DayZHealth` — used in HUDs

Each style XML defines named image slices for: `Left`, `Top`, `Right`,
`Bottom`, `Center`, plus `BarLeft`, `BarTop`, `BarRight`, `BarBottom`,
`BarCenter`. The "bar" slices stretch with the value; the others are
static frame.

## Setting and reading

```c
m_Bar.SetCurrent(75);
float v = m_Bar.GetCurrent();
```

`SetCurrent` clamps to `[min, max]`. No step snapping (unlike SliderWidget).

## Common COT use

COT doesn't use vanilla progress bars; it has `UIActionProgressBar` which
renders a coloured panel sized proportionally to value — gives full control
over colour transitions (e.g. green → red as a vehicle's health drops).

## Engine truth

> Defined at lines 369–379 of `scripts/1_core/proto/enwidgets.c`. Only
> 4 methods, all shared between Simple and full versions.
