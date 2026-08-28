# Layout Property Index

Every property name observed in `.layout` files across vanilla DayZ and
COT. Cross-referenced with the corresponding script API where one
exists. Properties not in this list may exist — the layout parser is
not documented exhaustively.

## Universal properties

| Property | Type | Description |
|---|---|---|
| `position X Y` | floats | Top-left position (relative or pixel) |
| `size W H` | floats | Width / height |
| `hexactpos 0\|1` | bool | X is pixel-exact |
| `vexactpos 0\|1` | bool | Y is pixel-exact |
| `hexactsize 0\|1` | bool | W is pixel-exact |
| `vexactsize 0\|1` | bool | H is pixel-exact |
| `halign` | enum | `left_ref` / `center_ref` / `right_ref` |
| `valign` | enum | `top_ref` / `center_ref` / `bottom_ref` |
| `color R G B A` | floats 0..1 | Tint colour |
| `visible 0\|1` | bool | Render the widget |
| `ignorepointer 0\|1` | bool | Pass mouse events through |
| `clipchildren 0\|1` | bool | Cull children outside bounds |
| `style "name"` | string | Named style from `.styles` |
| `scriptclass "name"` | string | Construct a script instance of this class |
| `draggable 0\|1` | bool | Widget can be dragged |
| `fixaspect` | enum | `none` / `inside` / `outside` |
| `priority N` | int | Drawing priority (similar to sort) |

## Text-bearing widgets

| Property | Type | Description |
|---|---|---|
| `text "value"` | string | Text content (`#STR_KEY` is localised) |
| `font "path"` | path | Font file |
| `"exact text" 0\|1` | bool | Pixel-exact rendering |
| `"exact text size" N` | int | Pixel height when `exact text 1` |
| `"text proportion" 0..1` | float | Glyph height as fraction of widget height |
| `"text halign" left\|center\|right` | enum | Horizontal text alignment |
| `"text valign" top\|center\|bottom` | enum | Vertical text alignment |
| `"text offset" X Y` | ints | Shift glyphs without moving widget |
| `"bold text" 0\|1` | bool | Bold rendering |
| `"italic text" 0\|1` | bool | Italic rendering |
| `"wrap text" 0\|1` | bool | Enable multi-line wrapping |
| `"text spacing" H V` | ints | Glyph spacing |
| `"text color" R G B A` | floats | Separate from widget color (UIWidget) |
| `"outline size" N` | int | Outline stroke width in px |
| `"outline color" R G B A` | floats | Outline colour |
| `"shadow size" N` | int | Shadow blur radius |
| `"shadow color" R G B A` | floats | Shadow colour |
| `"shadow opacity" 0..1` | float | Shadow alpha multiplier |
| `"shadow offset" X Y` | floats | Shadow displacement in px |
| `"text shadow color"` | floats | (UIWidget) shadow on text-bearing UIWidgets |
| `"text outline size"` | int | (UIWidget) outline on text |
| `"text outline color"` | floats | (UIWidget) outline colour |
| `"linespacing" N` | int | Pixels between lines (multi-line) |
| `"limit visible" 0\|1` | bool | EditBox: hide overflow chars |
| `"max chars" N` | int | EditBox: limit text length |
| `"chars allowed" "..."` | string | EditBox: whitelist input chars |
| `hide 0\|1` | bool | Password edit: mask glyphs |
| `checked 0\|1` | bool | CheckBox initial state |

## Image widgets

| Property | Type | Description |
|---|---|---|
| `image "path"` | path | Image file path or `set:X image:Y` |
| `mode blend\|additive\|none` | enum | Blend mode |
| `"src alpha" 0\|1` | bool | Use texture's alpha |
| `"no wrap" 0\|1` | bool | Don't tile |
| `"clamp mode" clamp\|repeat\|mirror` | enum | UV wrapping |
| `"stretch mode"` | enum | `stretch_w_h` / `stretch_uniform` / `stretch_outside` / `none` |
| `scaled 0\|1` | bool | (preview widgets) |
| `customuv 0\|1` | bool | Use custom UVs from SetUV |
| `flipu 0\|1` | bool | Mirror horizontally |
| `flipv 0\|1` | bool | Mirror vertically |
| `nofilter 0\|1` | bool | Disable texture filtering |

## Listbox

| Property | Description |
|---|---|
| `lines N` | Number of visible rows |
| `columns N` | Number of columns |

## Slider

| Property | Description |
|---|---|
| `min N` | Minimum value |
| `max N` | Maximum value |
| `current N` | Initial value |
| `step N` | Step size |

## Progress bar

| Property | Description |
|---|---|
| `min N` | Minimum value |
| `max N` | Maximum value |
| `current N` | Initial value |

## Spacer (Grid / Wrap)

| Property | Description |
|---|---|
| `Rows N` | Number of rows |
| `Columns N` | Number of columns |
| `Padding N` | Gap between cells |
| `Margin N` | Inner margin from edge |
| `"Size To Content H" 0\|1` | Width = sum of children |
| `"Size To Content V" 0\|1` | Height = sum of children |
| `align horizontal_left\|center\|right` | Content H alignment |
| `align vertical_top\|center\|bottom` | Content V alignment |

## Scroll widget

| Property | Description |
|---|---|
| `"Scrollbar H" 0\|1` | Show horizontal scrollbar |
| `"Scrollbar V" 0\|1` | Show vertical scrollbar |

## Render target widget

| Property | Description |
|---|---|
| `refresh N` | Render every N frames |

## Video widget

| Property | Description |
|---|---|
| `font "path"` | Font for subtitles |

## Less common / specialised

| Property | Description |
|---|---|
| `"no focus" 0\|1` | Excluded from controller navigation |
| `"render always" 0\|1` | Render even when culled |
| `noclear 0\|1` | Don't clear render buffer |

## Notes

- **Property names with spaces must be quoted** in the layout file:
  `"text halign"`, not `text halign`.
- **Boolean properties** accept `0` and `1`. The engine doesn't accept
  `true`/`false`.
- **Vectors** (positions, sizes, colours, offsets) are space-separated
  numbers without commas.
- **Unknown properties** are silently ignored — typos won't error, they
  just don't apply.

## Source

Compiled by inspecting:
- `scripts/1_core/proto/enwidgets.c` (the script API)
- `gui/layouts/*.layout` (vanilla examples)
- `JM/COT/GUI/layouts/**` (COT examples)
- DayZ Tools documentation
