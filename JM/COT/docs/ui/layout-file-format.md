# Layout File Format (`.layout`)

A `.layout` file is a plain-text declarative tree. Each node is one widget;
properties describe how it looks and behaves; nested braces declare children.

## Minimal example

```
PanelWidgetClass my_panel {
 position 0 0
 size 1 1
 color 0 0 0 0.5
}
```

That declares one panel filling its parent, half-opaque black.

## Top-level structure

```
<WidgetClassName> <widget_name> {
 <property> <values>
 <property> <values>
 ...
 {
  <child widget>
  <child widget>
 }
}
```

- `<WidgetClassName>` is one of `PanelWidgetClass`, `TextWidgetClass`,
  `ButtonWidgetClass`, `GridSpacerWidgetClass`, etc. — the suffix `Class`
  is mandatory.
- `<widget_name>` is the identifier you'll use in `FindAnyWidget("name")`.
- Properties are **space-separated** values on a single line.
- Children go in a nested `{ ... }` block.

## Property syntax

```
position 0 0
size 0.5 30
color 0.1 0.1 0.1 0.8
text "Hello world"
"text valign" center
```

- Single-word property names: bare identifier (e.g. `position`).
- Multi-word property names: **quoted** with double quotes (e.g. `"text valign"`).
- Strings: quoted with double quotes (e.g. `text "Hello"`).
- Numbers: bare (`0.5`, `30`, `-10`).
- Colours: 4 space-separated floats in `[0, 1]` — `r g b a`.
- Vectors / points: space-separated floats — no commas.

## Properties that everyone uses

### Position and size

| Property | Meaning |
|---|---|
| `position X Y` | Top-left corner (relative or exact — see flags) |
| `size W H` | Width and height (relative or exact) |
| `hexactpos 0\|1` | `1` = `X` is pixels, `0` = `X` is fraction of parent |
| `vexactpos 0\|1` | same for `Y` |
| `hexactsize 0\|1` | `1` = `W` is pixels, `0` = `W` is fraction of parent |
| `vexactsize 0\|1` | same for `H` |

See [[coordinate-system]] for the gotchas.

### Alignment

```
halign center_ref
valign bottom_ref
```

`halign` accepts `left_ref` (default), `center_ref`, `right_ref`.
`valign` accepts `top_ref` (default), `center_ref`, `bottom_ref`.
These shift the **origin** the `position` is relative to.

### Visibility and input

```
visible 1
ignorepointer 1   // 1 = widget never receives mouse events; click passes through
clipchildren 1    // 1 = children cannot draw outside this widget's rectangle
```

### Style

```
style DayZDefaultPanel
```

References a named style in a `.styles` file. See [[styling/styles-system]].

## Widget-specific properties

Each widget class has its own set. Common ones:

### Text-bearing widgets

```
text "Hello"               // string literal
text "#STR_KEY"            // localization key
font "gui/fonts/sdf_MetronLight24"
"text halign" center       // center | left | right
"text valign" center       // center | top | bottom
"text offset" 10 0         // shift glyph rendering inside widget bounds
"exact text" 1
"exact text size" 16       // pixel-exact glyph size
"bold text" 1
"shadow size" 2
"shadow color" 0 0 0 1
"shadow offset" 1 1
"outline size" 1
"outline color" 0 0 0 1
"wrap text" 0              // 0 = single line, truncate; 1 = multi-line
```

### Image widget

```
image "JM/COT/GUI/textures/icons/trash.paa"
mode blend                 // blend | additive | none
"src alpha" 1
"no wrap" 1
"clamp mode" clamp
"stretch mode" stretch_w_h
```

### Spacers (Grid / Wrap)

```
Rows 3
Columns 2
Padding 4
Margin 0
"Size To Content H" 1
"Size To Content V" 1
```

### Scroll widget

```
"Scrollbar V" 1
"Scrollbar H" 0
```

### Lifecycle

```
scriptclass "MyScriptClass"   // engine constructs this class for the widget
```

## Comments

The format **does not officially support comments**. Some parsers tolerate
`//` lines but vanilla layouts have no comments. If you need to annotate a
layout, do it in the file that references it (the script) instead.

## Property override order

Properties later in the file override earlier ones. Style properties are
applied first, then explicit properties in the layout — so anything in the
layout overrides the style.

## Loading a layout

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(
    "JM/COT/GUI/layouts/my_form.layout",
    parentWidget  // optional; if NULL, attached to workspace root
);
```

## Engine truth

> The parser is built into the engine and is not exposed to script. There is
> no documented schema — known properties come from inspecting vanilla layouts
> in `gui/layouts/` and `dta/scripts.pbo`. See [[reference/layout-property-index]]
> for the full list collected across the project.

## Conventions in COT

- File names use `snake_case.layout` (e.g. `sidebar_flyout.layout`).
- Widget names use `snake_case` (e.g. `cat_btn`, `flyout_root`).
- Top-level node is usually a `PanelWidgetClass` with `scriptclass`
  pointing to a `JMFormBase` subclass.
- Property indentation is 1 space (vanilla style), not tabs.
