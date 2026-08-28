# `PanelWidget`

The default solid-colour container. Has no special script methods beyond
`Widget`'s base — its job is to be a visible rectangle with children.

## When to use

- Background for a section
- Dividers (thin coloured panels)
- Wrapper to apply `clipchildren` to a group
- The container holding rows of controls

## Class

`PanelWidget` extends `Widget`. No type-specific API.
*(Actually a `UIWidget` in the engine — same effect.)*

## Layout properties

```
PanelWidgetClass my_panel {
 position 0 0
 size 1 0.5
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 color 0.1 0.1 0.1 0.8       // ARGB-as-floats, alpha 0.8
 style DayZDefaultPanel       // optional named style
 ignorepointer 1              // pass clicks through
 clipchildren 1               // children clip to this rectangle
 visible 1
 draggable 0
 {
  // children …
 }
}
```

## Typical uses

### Section background

```
PanelWidgetClass section_bg {
 ignorepointer 1
 color 0 0 0 0.3
 size 1 1
 {
  TextWidgetClass header { text "Section Title" }
  // … content
 }
}
```

### Thin divider

```
PanelWidgetClass divider {
 ignorepointer 1
 color 0.13 0.13 0.13 1
 position 0 0
 size 1 1
 vexactsize 1
 // height set by parent or programmatically
}
```

From script:

```c
Widget div = UIActionManager.CreatePanel(parent, 0xFF222222, /* height px */ 2);
```

`UIActionManager.CreatePanel(parent, color, height)` is the COT shortcut
that creates a `UIPanel.layout` with `size 1 1` and `"Size To Content V" 1`.

### Group with clipping

```
PanelWidgetClass clip_area {
 clipchildren 1
 size 200 30
 hexactsize 1
 vexactsize 1
 {
  TextWidgetClass long_label { size 500 30 text "Will be clipped at 200px" }
 }
}
```

## Style names commonly used

- `DayZDefaultPanel` — the standard background
- `rover_sim_colorable` — accepts `SetColor` tinting properly (without this, custom colours may not take effect on some panels)
- `Empty` — no style, fully transparent

## Engine truth

> The engine type ID is `PanelWidgetTypeID` (declared in the WidgetType
> enum). Script-side, `PanelWidget` doesn't have its own declaration in
> `enwidgets.c` — it inherits all methods from `Widget` / `UIWidget`.
