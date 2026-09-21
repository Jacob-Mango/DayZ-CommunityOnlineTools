# Fonts and Text Styling

DayZ uses **SDF** (Signed Distance Field) fonts for sharp rendering at
any size, plus traditional bitmap fonts. Most COT UI uses SDF.

## Font paths

Fonts live under `gui/fonts/`. Common ones:

| Path | Description |
|---|---|
| `gui/fonts/MetronBook` | Bitmap regular |
| `gui/fonts/MetronBook-Bold` | Bitmap bold |
| `gui/fonts/MetronLight14` | Bitmap light, pre-sized 14 |
| `gui/fonts/sdf_MetronLight24` | **SDF light** — scales cleanly |
| `gui/fonts/sdf_MetronBook72` | SDF book, large reference |
| `gui/fonts/EveSansNeue_Regular` | DayZ's typing font (rare) |

The `sdf_` prefix means SDF — these can scale to any size without
pixelation. The trailing number is the **source rendering size** (often
unrelated to display size).

## Specifying a font in a layout

```
font "gui/fonts/sdf_MetronLight24"
```

The path matches the file location; no extension. The engine looks for
`.fnt`, `.font`, or `.sdf` files.

## Sizing text

### Exact-size mode (recommended)

```
"exact text" 1
"exact text size" 16
```

Renders glyphs at exactly 16 pixels regardless of widget size. Sharpest,
most predictable.

### Proportion mode

```
"exact text" 0
"text proportion" 0.6
```

Glyphs render at `0.6 × widget_height`. Resizes with the widget.
Useful for HUD elements that scale with screen size; problematic when
the widget's height isn't text-sized.

### `SetTextExactSize`

```c
m_Label.SetTextExactSize(20);
```

Programmatic equivalent of `"exact text size" 20`. Setting `0` reverts
to the original style size.

## Bold / italic

```
"bold text" 1
"italic text" 1
```

Or in script:

```c
m_Label.SetBold(true);
m_Label.SetItalic(false);
```

These work for SDF fonts. Bitmap fonts have a single weight per file —
use the appropriate font path (e.g. `MetronBook-Bold` for bold bitmap).

## Outline

```
"outline size" 1
"outline color" 0 0 0 1
```

Or:

```c
m_Label.SetOutline(/* size px */ 1, /* ARGB */ 0xFF000000);
```

Draws a stroke around each glyph. Use 1–2 px for legibility against busy
backgrounds.

> **Property name pitfall:** the property is `"outline size"`, NOT
> `"text outline"`. Older DayZ docs got this wrong.

## Shadow

```
"shadow size" 2
"shadow color" 0 0 0 1
"shadow opacity" 1
"shadow offset" 1 1
```

Or:

```c
m_Label.SetShadow(/* size */ 2, /* color */ 0xFF000000,
                  /* opacity */ 1.0,
                  /* offsetX */ 1.0, /* offsetY */ 1.0);
```

`offset` is in pixels, relative to the glyph position. Positive Y is
down.

## Text colour

```
color 1 0.9 0.8 1     // RGBA in 0..1
```

Or on a UIWidget-extending text widget:

```c
m_Label.SetTextColor(ARGB(255, 255, 200, 200));
```

The plain `Widget.SetColor` works too but tints the entire widget
(including any background colour the style adds).

## Horizontal alignment

```
"text halign" left      // left | center | right
"text valign" center    // top  | center | bottom
```

Or via flags (older API):

```c
m_Label.SetFlags(WidgetFlags.CENTER);   // horizontal centering
m_Label.SetFlags(WidgetFlags.VCENTER);  // vertical centering
m_Label.SetFlags(WidgetFlags.RALIGN);   // right-align
```

## Glyph offset within the widget

```
"text offset" 10 0
```

Or:

```c
m_Label.SetTextOffset(10, 0);
```

Shifts glyphs without moving the widget. The widget's bounds stay where
they were; the text simply renders 10 px to the right of where it would.

> **Use `text offset`** to leave space for a leading icon **without
> overflowing the widget**. This is the difference between
> `position 28 0` (widget overflows by 28 px) and
> `position 0 0 "text offset" 28 0` (widget fits in parent; glyphs are
> offset 28 px inside).

## Text spacing

```c
m_Label.SetTextSpacing(/* horiz */ 2, /* vert */ 0);
```

Adds extra pixels between glyphs (horiz) and between lines (vert). Use
sparingly — kerning is generally better left to the font.

## Multi-line text

`TextWidget` is single-line. Long text gets clipped at the right edge.
Use [[../widgets/multiline-text-widget]] for wrapping.

```
"wrap text" 1
```

Toggle on `MultilineTextWidget` to enable wrapping (default on for that
class, off for `TextWidget`).

## Bidirectional / RTL

DayZ has limited RTL (Hebrew/Arabic) support. Text rendering is LTR
by default. There's no script API to switch direction.

## Format strings

```c
m_Label.SetTextFormat("Player %1 has %2 items", playerName, count);
```

Substitutes `%1`, `%2`, … with the args (up to 9). Translates the format
string if it starts with `#STR_`.

## Localisation

```
text "#STR_COT_PLAYER_LIST_HEADER"
```

Localisation keys are resolved at layout-load time. From script, wrap
with `Widget.TranslateString`:

```c
m_Label.SetText( Widget.TranslateString(key) );
```

Missing keys render as the key itself (e.g. `#STR_MISSING`).

## Engine truth

> Text-related properties are read by the engine's layout parser. Method
> declarations at lines 189–217 of `scripts/1_core/proto/enwidgets.c`
> (`TextWidget` class).
