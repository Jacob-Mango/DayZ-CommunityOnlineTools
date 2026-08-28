# Alignment Anchors

`halign` and `valign` change the **origin point** from which the
widget's `position` is measured. By default, the origin is the parent's
top-left corner.

## The six anchors

| Anchor | Origin location on parent |
|---|---|
| `halign left_ref` (default) | Left edge |
| `halign center_ref` | Horizontal centre |
| `halign right_ref` | Right edge |
| `valign top_ref` (default) | Top edge |
| `valign center_ref` | Vertical centre |
| `valign bottom_ref` | Bottom edge |

`halign` and `valign` are independent — combine to anchor at any of the
9 corners / centers.

## How position is interpreted

With `halign left_ref` (default):
- `position 0 0` → widget's **left edge** lands at the parent's left edge
- `position 10 0` → 10 px / 10% to the right of the parent's left edge

With `halign right_ref` the offset runs **inward**, toward the parent's centre:
- `position 0 0` → widget's **right edge** lands at the parent's right edge
- `position 10 0` → widget's right edge sits 10 px INSIDE the parent
- `position -10 0` → widget pushed 10 px PAST the parent's right edge (overflow)

The sign is the opposite of what `left_ref` habits suggest, and getting it
backwards is how a right-hand icon ends up half-clipped by the parent edge.
`GUI/layouts/windowbase.layout` is the reference: the pin / minimise / close
buttons are 25 px wide at `position 57 / 31 / 6`, evenly spaced inward from the
title bar's right edge. `valign bottom_ref` mirrors this - positive y is UP,
into the parent.

With `halign center_ref`:
- `position 0 0` → widget's **horizontal centre** lines up with parent's centre
- `position 10 0` → widget shifted 10 px to the right of centre

The same logic applies vertically with `valign`.

## Visual cheat sheet

```
Parent (100 × 100)

left_ref / top_ref:           center_ref / top_ref:        right_ref / top_ref:
┌──W──┐                              ┌──W──┐                            ┌──W──┐
└─────┘   .   .   .   .   .   .   .  └──┬──┘   .   .   .   .   .   .   .└─────┘
position (0, 0)                      position (0, 0)                    position (0, 0)

left_ref / center_ref:        center_ref / center_ref:     right_ref / center_ref:
┌──W──┐                              ┌──W──┐                            ┌──W──┐
│     │←                            ←│     │→                          →│     │
└─────┘                              └──┬──┘                            └─────┘

left_ref / bottom_ref:        center_ref / bottom_ref:     right_ref / bottom_ref:
┌──W──┐                              ┌──W──┐                            ┌──W──┐
└─────┘                              └──┬──┘                            └─────┘
```

## Examples

### Centered popup

```
PanelWidgetClass dialog {
 position 0 0
 size 400 200
 halign center_ref
 valign center_ref
 hexactpos 1
 vexactpos 1
 hexactsize 1
 vexactsize 1
}
```

The dialog is centred regardless of parent size. To shift up by 50 px:
`position 0 -50`.

### Bottom-right "close" button

```
ButtonWidgetClass close {
 position 10 10
 size 30 30
 halign right_ref
 valign bottom_ref
 hexactpos 1
 vexactpos 1
 hexactsize 1
 vexactsize 1
}
```

10 px inset from the bottom-right corner.

### Status bar at the bottom

```
PanelWidgetClass status {
 position 0 0
 size 1 24
 valign bottom_ref
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
}
```

100% wide, 24 px tall, pinned to bottom.

## Sibling vs ref anchors

DayZ has another type of anchor: **sibling-relative**, e.g.
`halign right_of_sibling`. These let you position a widget relative to
its previous sibling's edge.

Common values:
- `right_of_sibling` — to the right of the previous sibling
- `below_sibling` — below the previous sibling

These are less common in COT but appear in some vanilla layouts for chains
of buttons that abut each other.

## When alignment helps vs hurts

**Helps:**
- Centring overlays / popups
- Anchoring controls to corners
- Building "stick to right edge" layouts without computing offsets

**Hurts:**
- Combined with negative positions, it's easy to overflow the parent
- Mixed with auto-layout spacers, the anchor can be ignored (the spacer
  controls the position directly)

## Within auto-layout containers

In `GridSpacerWidget` and `WrapSpacerWidget`, **children's
`halign` / `valign` are usually ignored**. The container decides each
child's position based on the row/col index and flow.

Use `SetContentAlignmentH` / `SetContentAlignmentV` on the **container**
to control how children align inside their allotted cell.

```c
spacer.SetContentAlignmentH(WidgetAlignment.WA_CENTER);
spacer.SetContentAlignmentV(WidgetAlignment.WA_CENTER);
```

## Engine truth

> The anchor system is part of the layout engine. The values
> `left_ref`, `right_ref`, `center_ref`, `top_ref`, `bottom_ref` are
> string tokens parsed by the layout loader; they're not in the script
> enum. They're documented across vanilla layout files in
> `gui/layouts/`.
