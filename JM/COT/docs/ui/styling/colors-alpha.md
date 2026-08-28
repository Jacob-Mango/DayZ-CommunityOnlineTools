# Colours and Alpha

DayZ colours are 32-bit **ARGB integers** in script, but
**RGBA floats** in `.layout` files. Watch for the order swap.

## ARGB integer format

```
0xAARRGGBB
```

- High byte = alpha (0x00 transparent, 0xFF opaque)
- Next 3 bytes = R, G, B

```c
int red    = 0xFFFF0000;   // opaque red
int green  = 0xFF00FF00;
int blue   = 0xFF0000FF;
int clear  = 0x00000000;   // fully transparent
```

## Building colours from components

```c
int color = ARGB(/* a */ 255, /* r */ 255, /* g */ 0, /* b */ 0);
```

`ARGB(a, r, g, b)` is a global helper. Components are `0..255`.

## Predefined constants

Common ones (from `JMConstants.c` and engine):

```c
COLOR_WHITE        = 0xFFFFFFFF
COLOR_BLACK        = 0xFF000000
COLOR_RED          = 0xFFFF0000
COLOR_GREEN        = 0xFF00FF00
COLOR_BLUE         = 0xFF0000FF
COLOR_YELLOW       = 0xFFFFFF00
COLOR_GRAY         = 0xFF808080
COLOR_TRANSPARENT  = 0x00000000
```

Browse `JMConstants.c` for DayZ-themed colour constants (food / blood /
liquid colours, etc.).

## Layout file: RGBA float format

```
color 1 0.5 0 0.8
```

Four floats in `[0, 1]`:
- R = 1.0 (full red)
- G = 0.5 (half green)
- B = 0.0 (no blue)
- A = 0.8 (80% opaque)

> **Note the ordering: RGBA in layouts, but ARGB in script integers.**
> Easy to swap them and end up with the wrong colour.

## Setting colour from script

```c
widget.SetColor(0xFFFF0000);             // ARGB int
widget.SetColor(ARGB(255, 255, 0, 0));   // equivalent
```

## Setting alpha independently

```c
widget.SetAlpha(0.5);   // 50% — multiplied with current colour alpha
```

`SetAlpha` doesn't touch the RGB; it overrides just the alpha channel.

> **Don't mix `SetColor` and `SetAlpha` carelessly.** If `SetColor`
> uses alpha 0xCC and then `SetAlpha(1.0)` is called, the widget becomes
> fully opaque. To preserve `SetColor`'s alpha, only call `SetAlpha`
> when you specifically want to override it.

## Text widget colour vs widget colour

`TextWidget` has two colour properties:

- `widget.SetColor(c)` — the widget's overall tint (affects styled
  visuals like the focus highlight).
- `widget.SetTextColor(c)` (UIWidget method) — the glyph colour only.

```c
m_Label.SetTextColor(0xFFFF0000);   // red text
m_Label.SetColor(0x00000000);        // transparent widget background
```

## Image widget tinting

`ImageWidget.SetColor` multiplies the image with the colour. White
images can be tinted any colour; coloured images get darkened/blended.

```c
m_Icon.SetColor(COLOR_GREEN);   // a white icon becomes green
```

Use the `ARGB(a, r, g, b)` helper to keep alpha intact:

```c
m_Icon.SetColor(ARGB(255, 0, 255, 0));   // fully opaque green
m_Icon.SetColor(ARGB(128, 0, 255, 0));   // half-transparent green
```

## Global brightness (LV / lighting)

```c
Widget.SetLV(-5);                // dims all widgets to ~70%
Widget.SetTextLV(-3);            // dims only text
Widget.SetObjectLighting(0.5);   // dims 3D RenderTarget contents
```

Used by night-vision shaders, blackout effects, accessibility settings.
Values are negative (0 = normal, -15 = darkest).

## Engine truth

> Colour methods at lines 144–149 of `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native void SetColor(int color);
> proto native int  GetColor();
> proto native void SetAlpha(float alpha);
> proto native float GetAlpha();
> ```
> Global lighting at lines 114–119.
