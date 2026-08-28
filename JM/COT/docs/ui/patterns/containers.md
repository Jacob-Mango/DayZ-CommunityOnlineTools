# Containers Cheat Sheet

Quick reference for "which container do I use for this?"

## Decision table

| Need | Use |
|---|---|
| Visible coloured background | `PanelWidget` |
| Invisible hierarchy node / clipping rectangle | `FrameWidget` |
| Fixed N rows × M cols layout | `GridSpacerWidget` |
| Children flow horizontally, wrap when needed | `WrapSpacerWidget` |
| Scrollable area with H/V scrollbars | `ScrollWidget` |
| Embed another `.layout` file as a single subtree | `EmbededWidget` |

## Comparison

| Container | Visible? | Auto-layout? | Clips? | Used for |
|---|---|---|---|---|
| Panel | Yes | No | Optional | Backgrounds, dividers, group containers |
| Frame | No | No | Optional | Pure grouping, ref anchor |
| GridSpacer | No | **Yes — grid** | No | Equal-sized cells, table-like data |
| WrapSpacer | No | **Yes — flow** | No | Toolbars, rows of mixed-width buttons |
| ScrollWidget | No | No (single child) | **Yes — viewport** | Anywhere content might exceed visible area |
| EmbededWidget | No | No | No | Reusing widget subtrees from separate layouts |

## "Auto-layout" caveat

GridSpacer arranges children in a fixed `Rows × Columns` grid;
WrapSpacer flows left-to-right and wraps. Neither does what CSS calls
flexbox (one growing child sharing the leftover space), but you can
approximate it:

```c
// WrapSpacer pseudo-flex:
icon.SetFixedSize(32, 32);    // takes 32 px absolute
spacer.SetWidth(1.0);         // takes 100% of REMAINING width
```

## Sizing behaviour

| Container | Default sizing |
|---|---|
| Panel | Size from layout property, no auto-grow |
| Frame | Size from layout property |
| GridSpacer | Width = parent (or sum of cols if `Size To Content H`); height = sum of rows (with `Size To Content V`) |
| WrapSpacer | Width = parent; height grows as rows wrap |
| ScrollWidget | Size from layout; content can exceed without affecting widget size |

## Nesting

Containers nest freely. Common patterns:

### Form structure

```
PanelWidget (background)
└── GridSpacer (1, 1, header + content + footer)
    ├── WrapSpacer (toolbar row)
    ├── ScrollWidget (main content)
    │   └── GridSpacer (1, 1, list of rows)
    │       ├── WrapSpacer (row 1)
    │       ├── WrapSpacer (row 2)
    │       └── ...
    └── WrapSpacer (bottom actions)
```

### Tabbed panels

```
GridSpacer (1, 1)
├── WrapSpacer (tab buttons)
├── GridSpacer (1, 1, tab content stack)
│   ├── Panel (tab 1 content, hidden when not active)
│   ├── Panel (tab 2 content)
│   └── Panel (tab 3 content)
```

Switch tabs by toggling `Show(true/false)` on each Panel.

### Inline form row

```
WrapSpacer (one row, no wrap because widths fit)
├── Icon (32 px fixed)
├── Label (fraction 0.6)
└── Button (fraction 0.4)
```

## Picking the right container

### "I have a fixed list of N buttons"

`GridSpacer(1, N)` if all the same width.
`WrapSpacer` if buttons have different widths.

### "I have a list that grows / shrinks at runtime"

`GridSpacer(1, 1)` with `"Size To Content V" 1` — children stack
vertically and the spacer grows.

### "I want a button on the left and a label filling the rest"

`WrapSpacer`:
```c
btn.SetFixedSize(32, 32);
label.SetWidth(1.0);   // fills remaining
```

### "I want two columns side by side"

`GridSpacer(1, 2)` — both columns automatically equal width.

For unequal widths, `WrapSpacer` with `SetWidth(0.3)` and `SetWidth(0.7)`.

### "Content too tall, need scrolling"

Wrap in `ScrollWidget` with `"Scrollbar V" 1`.

### "I want a sub-tree I can re-use across forms"

Make it a separate `.layout` and reference with `EmbededWidget`. See
[[embedding]].

## See also

- [[../widgets/panel-widget]] / [[../widgets/frame-widget]] for plain containers
- [[../widgets/gridspacer-widget]] for grids
- [[../widgets/wrapspacer-widget]] for flow layouts
- [[../widgets/scroll-widget]] for scrolling
- [[flow-composition]] for composing rows without overflow
