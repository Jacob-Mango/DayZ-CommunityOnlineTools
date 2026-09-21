# Sizing Rules

The tricky part of DayZ UI is sizing: each axis (X, Y) can independently
be **fractional** (proportion of parent) or **pixel-exact**, and the
choice is on **flags** on the widget itself, not on the parent.

## The four axis switches

| Flag | Default | When set | When unset |
|---|---|---|---|
| `hexactpos` | 0 | `X` in pixels | `X` is fraction of parent width |
| `vexactpos` | 0 | `Y` in pixels | `Y` is fraction of parent height |
| `hexactsize` | 0 | `W` in pixels | `W` is fraction of parent width |
| `vexactsize` | 0 | `H` in pixels | `H` is fraction of parent height |

These can be mixed in any combination per widget.

## The five core configurations

### 1. Fill parent (most common container)

```
position 0 0
size 1 1
hexactpos 0
vexactpos 0
hexactsize 0
vexactsize 0
```

The widget exactly fills its parent. Use for backgrounds, full-form
panels, scrollers.

### 2. Fixed pixel widget (icons, buttons, dividers)

```
position 0 0
size 32 32
hexactpos 1
vexactpos 1
hexactsize 1
vexactsize 1
```

A 32×32 pixel widget. Doesn't scale with parent.

### 3. Full-width strip with fixed height

```
position 0 0
size 1 28
hexactpos 0    // X fractional
vexactpos 1    // Y pixel
hexactsize 0   // W fractional (100%)
vexactsize 1   // H pixel (28)
```

A 28-pixel-tall row spanning full width. Used for toolbars, dividers.

### 4. Half-width column

```
position 0 0
size 0.5 1
hexactpos 0
vexactpos 0
hexactsize 0
vexactsize 0
```

50% wide, full height. Used for left/right pane splits.

### 5. Anchored corner widget

```
position -5 -5         // 5 px in from bottom-right
size 100 30
halign right_ref
valign bottom_ref
hexactpos 1
vexactpos 1
hexactsize 1
vexactsize 1
```

A 100×30 button anchored 5 px inside the parent's bottom-right corner.

## Mixing fractional and exact

In one parent, you can have:

```
Parent panel (size 1 1)
├── child A (size 0.5 1)   // 50% width
├── child B (size 100 1)   // 100 px width (with hexactsize 1)
└── child C (size 1 30)    // 100% width (overlaps A and B!)
```

The engine doesn't subtract A and B from C — they're independent boxes
positioned within the parent's coordinate space. If you want C to fit
"after" A and B, use a `GridSpacer` or `WrapSpacer` to lay them out.

## How fractional widths sum

In a `WrapSpacer`, fractional widths divvy up the **remaining space**
after fixed-pixel siblings consume their absolute width.

```
WrapSpacer parent (400 px wide)
├── icon (fixed 32 px)
├── label (SetWidth 0.6)
└── button (SetWidth 0.4)

// icon takes 32 px.
// Remaining = 400 - 32 = 368 px.
// label gets 0.6 × 368 = 220.8 px.
// button gets 0.4 × 368 = 147.2 px.
// Total: 32 + 221 + 147 = 400 ✓
```

If `0.6 + 0.4 = 1.0`, the row fits exactly. If `0.7 + 0.4 = 1.1`, the
last child wraps.

In a `GridSpacer`, fractional widths set the child's width **within its
cell**, NOT the column width. Each cell is the same width (1 / N of the
spacer's width).

## When to choose fractional vs exact

| Use fractional when | Use exact when |
|---|---|
| The widget should scale with parent | The widget has fixed-size art (icon) |
| You're filling 50% / 25% / 100% of a container | You want a 32×32 icon button |
| You don't know the parent's pixel size | The widget contains a fixed-size element (e.g. 24×24 image) |
| You want it responsive to window resize | You're matching a precise pixel design |

## `SetFixedSize` helper

COT's `UIActionBase.SetFixedSize(w, h)` flips the right flags and sets
the size:

```c
btn.SetFixedSize(32, 32);
```

Equivalent to:

```c
btn.GetLayoutRoot().SetFlags(WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE);
btn.GetLayoutRoot().SetSize(32, 32);
```

## `SetWidth` helper

COT's `UIActionBase.SetWidth(fraction)` is the fractional equivalent:

```c
btn.SetWidth(0.5);
```

Calls `SetSize(0.5, currentH)` and ensures the H-size flag is fractional
(`hexactsize 0`).

## Pitfalls

### "My fixed-size widget got tiny"

You set `size 32 32` but kept `hexactsize 0`. Result: 32 × parent width
= 32 × 400 = 12800 px — overflows to fill the screen. Or, with size 0.5,
50% of parent width = 200 px instead of 32 px.

Always set `hexactsize 1` AND `vexactsize 1` for pixel-exact sizes.

### "Fractions don't add up to 100% visually"

WrapSpacer has padding/margin around children by default. Subtract the
padding from the parent width before computing fractions, or use
`CreateWrapSpacerCompact` (zero padding).

### "I set SetWidth(0.5) on a GridSpacer child but it still fills the cell"

GridSpacer cells are uniform-width. `SetWidth` inside a cell only shrinks
the child within that cell — it doesn't change the cell. To have
unequal columns, use a `WrapSpacer` instead.

## Coordinate diagram

```
Parent (any container)
┌─────────────────────────────────┐
│                                 │
│   position (X, Y) of child      │
│   ↘                             │
│    ┌──── size (W, H) ─────┐     │
│    │                      │     │
│    │       child          │     │
│    │                      │     │
│    └──────────────────────┘     │
│                                 │
└─────────────────────────────────┘

X, Y are measured from the parent's TOP-LEFT (default halign/valign).
W, H are the child's dimensions.
Both interpreted per the *EXACTPOS / *EXACTSIZE flags.
```

## See also

- [[../coordinate-system]] for the underlying math
- [[../widget-flags]] for the flag bits
- [[alignment-anchors]] for moving the anchor origin
