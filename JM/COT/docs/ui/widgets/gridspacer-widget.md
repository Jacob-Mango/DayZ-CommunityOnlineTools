# `GridSpacerWidget`

Auto-layout container that arranges children in a fixed grid of rows and
columns. The single most useful layout primitive in DayZ UI.

## Class

```c
class GridSpacerWidget extends SpacerWidget {};
```

No script-side properties beyond `SpacerWidget`'s content alignment. All
configuration is via layout properties.

## Layout properties

```
GridSpacerWidgetClass my_grid {
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 Rows 3
 Columns 2
 Padding 4              // gap between cells
 Margin 0               // gap inside the grid's outer edge
 "Size To Content H" 0  // grid width = sum of column widths or parent width
 "Size To Content V" 1  // grid height = sum of row heights
}
```

## Row × column flow

Children are added **left-to-right then top-to-bottom**. For `Rows 3
Columns 2`:

```
| 0 | 1 |
| 2 | 3 |
| 4 | 5 |
```

The 6 cells are filled in order; child 7 goes off-grid.

## How DayZ handles "wrong" child count

- **Fewer children than cells**: empty cells leave visible gaps.
- **More children than cells**: depends on `Size To Content V`. With
  `1`, the grid grows to fit (extra rows appear). Without, children
  overflow.

Either case usually indicates a bug. Match the grid declaration to the
actual child count.

## Column / row sizing

By default, all columns are **equal width** and all rows are **equal
height**. There is no per-column or per-row size declaration in the
layout file — sizing comes from the children themselves.

A child's `SetWidth(fraction)` controls its width **within** its cell,
NOT the cell width. So `SetWidth(0.5)` makes the child fill half its
cell, not half the grid.

## `Size To Content V` / `Size To Content H`

```
"Size To Content V" 1     // height = sum of row heights (auto-grow)
"Size To Content H" 0     // width = controlled by `size` property
```

When set:
- The grid's size on that axis is determined by its content.
- Useful for vertical stacks where you don't know how many rows up-front.

When unset:
- The grid's size is taken from its `size` property (relative to parent).
- Columns/rows are scaled to fit.

The **default is "Size To Content V" 1** for most COT grids (taken from
`UIActionManager.CreateGridSpacer` factory).

## Programmatic creation

```c
GridSpacerWidget grid = UIActionManager.CreateGridSpacer(parent, /* rows */ 2, /* cols */ 3);
```

Each `(rows, cols)` combination has its own layout file in
`JM/COT/GUI/layouts/uiactions/Wrappers/<rows>/GridSpacer<cols>.layout`.
Common shapes (1×1, 1×2, 2×1, 2×2, etc.) are precomputed.

## Patterns

### Vertical stack (`1, 1` with auto-grow)

```c
Widget col = UIActionManager.CreateGridSpacer(parent, 1, 1);
// Add as many children as you want — Size To Content V handles it.
```

This is the **universal "vertical stack"** in COT. The `Rows 1` hint is
ignored; children flow vertically thanks to `Size To Content V`.

### Two columns of equal width

```c
Widget two = UIActionManager.CreateGridSpacer(parent, 1, 2);
UIActionButton left  = UIActionManager.CreateButton(two, "Left",  this, "OnL");
UIActionButton right = UIActionManager.CreateButton(two, "Right", this, "OnR");
```

### Four buttons in 2×2

```c
Widget grid = UIActionManager.CreateGridSpacer(parent, 2, 2);
UIActionManager.CreateButton(grid, "A", this, "OnA");
UIActionManager.CreateButton(grid, "B", this, "OnB");
UIActionManager.CreateButton(grid, "C", this, "OnC");
UIActionManager.CreateButton(grid, "D", this, "OnD");
```

## Common pitfalls

### Cell count mismatch

```c
Widget grid = UIActionManager.CreateGridSpacer(parent, 4, 2);  // 8 cells
// But you only have 4 buttons. Result: 4 empty cells of wasted space.
```

Always match `Rows × Columns` to actual child count. See
[[../patterns/sizing-rules]].

### Trying to set per-column widths

There's no `column_width_1 0.3 column_width_2 0.7` syntax. If you need
unequal columns, use a `WrapSpacer` (which sums fractional child widths)
or nest grids inside grids.

### `SetWidth` on a child doesn't resize the column

```c
btn.SetWidth(0.5);  // makes btn 50% of its cell, NOT 50% of the grid
```

Use this for centred sub-widgets within a uniform-cell grid.

## Engine truth

> Defined at lines 473–475 of `scripts/1_core/proto/enwidgets.c`.
> Pure subclass of `SpacerWidget`, no extra methods. All the magic is
> in the C++ layout engine reading `Rows`, `Columns`, `Padding`,
> `Margin`, `Size To Content`.
