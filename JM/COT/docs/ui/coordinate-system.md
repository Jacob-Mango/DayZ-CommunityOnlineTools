# Coordinate System

Position and size in DayZ widgets are either **fractional** (proportion of
the parent) or **pixel-exact** (logical pixels at the engine's reference
resolution). The toggle between the two is per-axis and lives on **flags**.

## The four flags

| Flag | Meaning when set (`1`) |
|---|---|
| `hexactpos` | `X` is in pixels |
| `vexactpos` | `Y` is in pixels |
| `hexactsize` | `W` is in pixels |
| `vexactsize` | `H` is in pixels |

When unset (`0`), the value is a **fraction of the parent's resolved size**
(`0..1`, but values outside that range are allowed — they just go beyond
the parent's box).

These can be mixed freely: a widget can be at a fractional X but pixel Y,
or be sized pixel-wide and fraction-tall.

## Examples

### Fill the parent

```
position 0 0
size 1 1
hexactpos 0
vexactpos 0
hexactsize 0
vexactsize 0
```

### 32px square in the top-left corner

```
position 0 0
size 32 32
hexactpos 1
vexactpos 1
hexactsize 1
vexactsize 1
```

### 100% wide, 28px tall toolbar at the top

```
position 0 0
size 1 28
hexactpos 0   // X is fractional
vexactpos 1   // Y is in pixels
hexactsize 0  // W is fractional
vexactsize 1  // H is in pixels
```

### Centered, 50% wide

```
position 0 0
size 0.5 1
halign center_ref
```

`halign center_ref` shifts the position **origin** to the parent's
horizontal centre, so `position 0` now means "centred". See
[[patterns/alignment-anchors]].

## Pixel resolution

"Pixels" here means **logical pixels** at the engine's reference resolution
(approximately 1920×1080). The renderer scales the workspace to fit the
actual screen, so a `size 32 32` panel doesn't physically become 32 monitor
pixels on a 4K screen — it stays 32 logical pixels.

## When to use which

| Situation | Use |
|---|---|
| The widget should grow with its parent (responsive) | Fractional |
| The widget has fixed art and must stay one size | Exact |
| An icon button (typically 32×32) | Exact |
| A row inside a parent that may resize | Fractional W, exact H |
| Text label that should fit content | Exact H, fractional W |

## Mixing fractional and exact in one parent

A parent with three children: a fixed icon, a stretching middle, and a
fixed button on the right is **awkward** to express in pure fractional
math, because you don't know what fraction "leftover after 32px" is.

DayZ doesn't have CSS `flex: 1`. Instead, `WrapSpacer` (and to a lesser
extent `GridSpacer`) lets siblings declare fixed pixel sizes and others
declare fractions of the **remaining** space.

```c
// WrapSpacer flow:
icon.SetFixedSize(32, 32);   // consumes 32 px
search.SetWidth(1.0);        // takes 100% of what's left after icons
```

This is the COT convention for [[patterns/flow-composition]].

## Position relative to alignment anchor

The `halign` and `valign` properties pick the **anchor** point on the parent:

| Anchor | Where the widget's matching corner lands when `position 0 0` |
|---|---|
| `halign left_ref` | Parent's left edge (default) |
| `halign center_ref` | Parent's horizontal centre |
| `halign right_ref` | Parent's right edge |
| `valign top_ref` | Parent's top edge (default) |
| `valign center_ref` | Parent's vertical centre |
| `valign bottom_ref` | Parent's bottom edge |

`position X Y` is then added to that anchor position. With
`halign right_ref` and `position 10 0`, the widget's right edge is `10 px`
to the right of the parent's right edge — i.e. it overflows by 10 px. To
sit 10 px **inside** the right edge, use `position -10 0`.

## `SetPos` / `SetSize` from script

```c
widget.SetPos(0.5, 0.5);     // overrides position (interpreted same way as layout)
widget.SetSize(100, 30);     // overrides size
widget.SetScreenPos(x, y);   // overrides in monitor pixels (rare)
widget.SetScreenSize(w, h);  // same
```

The "interpreted same way as layout" caveat is important: whether your
arguments are fractions or pixels depends on the **flags currently set**.
If `hexactpos 1` was declared in the layout, `SetPos(0.5, 0)` means
"0.5 pixels". If you need to change interpretation at runtime, call
`SetFlags(WidgetFlags.HEXACTPOS, true)` or `ClearFlags(...)` first.

## Common pitfall: overflow

A child with `size 1 H` plus a `position X 0` extends past its parent by
`X` (when `hexactpos 1 hexactsize 0`). This causes overlapping content
when the parent has multiple horizontally-tiled children.

Use `"text offset"` to shift **only the rendered glyphs** inside the
widget without enlarging the widget itself. See [[patterns/clipping]].

## Engine truth

> Position and size accessors:
> ```
> proto native void SetPos(float x, float y, bool immedUpdate = true);
> proto native void SetSize(float w, float h, bool immedUpdate = true);
> proto void       GetPos(out float x, out float y);
> proto void       GetSize(out float width, out float height);
> proto void       GetScreenPos(out float x, out float y);
> proto void       GetScreenSize(out float width, out float height);
> ```
> from `scripts/1_core/proto/enwidgets.c`.
>
> `GetScreenPos` / `GetScreenSize` always return monitor pixels regardless
> of the flags, useful for hit-testing.
