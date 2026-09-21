# `TextWidget`

Single-line text rendering. Supports custom font, size, alignment,
outline, shadow, bold / italic.

## Class

```c
class TextWidget extends Widget
{
    proto native void SetTextSpacing(int horiz, int vert);
    proto native void SetTextExactSize(int size);     // 0 = original size
    proto native void SetTextOffset(int left, int top);
    proto native void SetText(string text, bool immedUpdate = true);
    proto native void SetOutline(int outlineSize, int argb = 0xFF000000);
    proto native int  GetOutlineSize();
    proto native int  GetOutlineColor();
    proto native void SetShadow(int shadowSize, int shadowARGB = 0xFF000000,
                                float shadowOpacity = 1, float shadowOffsetX = 0, float shadowOffsetY = 0);
    proto native int  GetShadowSize();
    proto native int  GetShadowColor();
    proto native float GetShadowOpacity();
    proto void GetShadowOffset(out float sx, out float sy);
    proto native void SetItalic(bool italic);
    proto native bool GetItalic();
    proto native void SetBold(bool bold);
    proto native bool GetBold();
    proto void GetTextSize(out int sx, out int sy);
    proto void SetTextFormat(string text, …);
    proto native float GetTextProportion();
    proto native void  SetTextProportion(float val);
};
```

## Layout properties

```
TextWidgetClass label {
 ignorepointer 1
 position 0 0
 size 1 20
 valign center_ref
 vexactsize 1
 text "Hello"                // string literal, or "#STR_KEY" for localisation
 font "gui/fonts/sdf_MetronLight24"
 "bold text" 1
 "exact text" 1              // use pixel-exact rendering (sharp at any size)
 "exact text size" 16        // pixel height
 "text halign" center        // center | left | right
 "text valign" center        // center | top | bottom
 "text offset" 10 0          // shift glyphs without moving widget
 "shadow size" 2
 "shadow color" 0 0 0 1
 "shadow opacity" 1
 "shadow offset" 1 1
 "outline size" 1
 "outline color" 0 0 0 1
 "wrap text" 0               // 0 = single-line, no wrapping
}
```

## Two ways to size text

### `"exact text" 1` + `"exact text size" N`

Text renders at exactly N pixels regardless of the widget's box height.
This is what COT uses (`sdf_MetronLight24` font at size 14–16).

### `"exact text" 0` (default)

Text scales with the widget's height — `"text proportion"` controls the
ratio (0..1) between widget height and text height.

The exact-size mode produces sharper, more predictable output. Always
use it unless you specifically want text that resizes with its container.

## Positioning text inside the widget

Two independent properties:

- **Widget position** (`position X Y` + alignment) places the widget's box.
- **`"text offset" X Y`** shifts the rendered glyphs within the box.

```
TextWidgetClass label {
 position 0 0
 size 1 30          // widget is full width × 30 px
 "text offset" 28 0 // text inside is offset 28 px right
 "text valign" center
}
```

The widget itself still spans full width, but the rendered text starts
28 px in. This is the **correct** way to leave space for a leading icon
without making the widget overflow.

> **Wrong way that bites everyone:** use `position 28 0 size 1 1`. The
> widget then extends from x=28 to x=28+parent_width, overflowing 28 px
> past the parent. Use `"text offset"` instead.

## Outline and shadow

DayZ supports both simultaneously.

```
"outline size" 1
"outline color" 0 0 0 1

"shadow size" 2
"shadow color" 0 0 0 1
"shadow offset" 1 1
```

`outline` draws a stroke around each glyph (good for high-contrast UI on
busy backgrounds). `shadow` draws an offset blurred copy underneath (good
for readability over varied content).

> **Property name pitfall:** `"text shadow"` does not exist. The correct
> property is `"shadow size"`. Likewise `"text outline"` is wrong;
> `"outline size"` is right. Both are easy to misremember because the
> getter / setter methods are `SetShadow` / `SetOutline` (no `text` prefix).

## Localisation

`text "#STR_KEY"` in a layout is auto-translated at load time. From
script, you must translate manually:

```c
m_Label.SetText( Widget.TranslateString("#STR_KEY") );
```

Unknown keys render as the key itself (e.g. `#STR_MISSING`).

## Measuring rendered text

```c
int w, h;
m_Label.GetTextSize(w, h);
```

Returns the pixel size of the **currently rendered** text. Use this for
marquee scrolling (when text wider than widget) — see
`JMSidebarScrollEntry` in `JMCOTSideBarCategory.c`.

## `SetTextFormat`

```c
m_Label.SetTextFormat("Hello %1, you have %2 items", playerName, itemCount);
```

DayZ's `printf`-style formatter. Substitutes `%1`, `%2`, etc. with the
varargs (up to 9 parameters). Translates the format string itself if it
starts with `#STR_`.

## Common pitfall: line wrapping

`TextWidget` is single-line. Long text gets clipped at the widget's right
edge (if `clipchildren` on parent) or overflows past it.

For wrapped multi-line text, use [[multiline-text-widget]]:

```
MultilineTextWidgetClass paragraph {
 size 1 1
 text "Long content that needs wrapping…"
 "wrap text" 1
}
```

## Engine truth

> `TextWidget` is declared at lines 189–217 of
> `scripts/1_core/proto/enwidgets.c`. The `Set*` getters / setters
> mirror the layout properties with slightly different names (e.g.
> `SetOutline` corresponds to `"outline size"`, `SetTextOffset` to
> `"text offset"`).
