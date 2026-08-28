# `ImageWidget`

Renders a single image (or one of up to 8 loaded images) with optional
masking, custom UVs, and tinting.

## Class

```c
class ImageWidget extends Widget
{
    proto native bool LoadImageFile(int num, string name, bool noCache = false);
    proto native void SetImageTexture(int image, RTTextureWidget texture);
    proto void GetImageSize(int image, out int sx, out int sy);

    proto native bool SetImage(int num);  // switch to image slot
    proto native int  GetImage();

    proto native void SetUV(float uv[4][2]);   // custom UVs (4 corners × 2)

    proto native bool LoadMaskTexture(string resource);
    proto native float GetMaskProgress();
    proto native void  SetMaskProgress(float value);   // 0..1
    proto native float GetMaskTransitionWidth();
    proto native void  SetMaskTransitionWidth(float value);  // 0..1
};
```

## Loading images

Each `ImageWidget` has 8 image slots, indexed 0–7. You can load multiple
images and switch between them without re-creating the widget.

```c
m_Icon.LoadImageFile(0, "JM/COT/GUI/textures/icons/trash.paa");
m_Icon.LoadImageFile(1, "JM/COT/GUI/textures/icons/refresh.paa");
m_Icon.SetImage(0);   // show trash
// later:
m_Icon.SetImage(1);   // switch to refresh — no reload
```

Use `noCache = true` for one-shot images you don't want to keep in memory.

## Layout

```
ImageWidgetClass icon {
 ignorepointer 1
 position 0 0
 size 24 24
 halign center_ref
 valign center_ref
 hexactsize 1
 vexactsize 1
 image "JM/COT/GUI/textures/icons/trash.paa"
 mode blend             // blend | additive | none
 "src alpha" 1          // honour the texture's alpha channel
 "no wrap" 1            // do not tile the texture
 "clamp mode" clamp     // clamp | repeat | mirror
 "stretch mode" stretch_w_h
 color 1 0 0 1          // tint red
}
```

## Stretch modes

| Mode | Effect |
|---|---|
| `stretch_w_h` | Stretch independently on both axes (may distort) |
| `stretch_uniform` | Scale to fit, preserve aspect (may letterbox) |
| `stretch_outside` | Scale to cover, preserve aspect (may crop) |
| `none` | No stretching, image renders at native size |

## Tinting

`SetColor` tints the image. White (`0xFFFFFFFF`) = original colours.
Coloured = multiplied with the texture pixels. Alpha component fades the
whole image.

For monochrome icons drawn in white, this lets you re-colour at runtime:

```c
m_Icon.SetColor(COLOR_RED);     // makes the white icon red
m_Icon.SetColor(COLOR_GREEN);
```

## Custom UVs

`SetUV(float[4][2])` lets you specify the 4 corners' UV coords. Useful
for animating sprites in an atlas:

```c
float uv[4][2] = {
    {0.0, 0.0},  // top-left
    {0.5, 0.0},  // top-right
    {0.5, 1.0},  // bottom-right
    {0.0, 1.0}   // bottom-left
};
m_Icon.SetUV(uv);
```

Requires `WidgetFlags.CUSTOMUV` to be set (in layout: `customuv 1`).

## Alpha masking

Used for radial-fill effects, screen wipes, dissolves.

```c
m_Icon.LoadMaskTexture("path/to/mask.paa");
m_Icon.SetMaskProgress(0.5);            // 50% revealed
m_Icon.SetMaskTransitionWidth(0.1);     // 10% smooth band
```

For progress `p` and width `w`:
- Mask pixels < `p` → opaque
- Mask pixels in `[p, p+w]` → smoothly transitioning
- Mask pixels > `p+w` → transparent

Used by the inventory progress effects and the health overlay.

## Image formats

DayZ uses **PAA** textures. Convert PNG → PAA with the Image-To-Paa tool
in the DayZ Tools distribution. PAA supports:

- DXT1 (no alpha)
- DXT5 (4-bit alpha)
- Uncompressed RGBA (largest, sharpest)

## Imageset references

Icons in a packed atlas use the `set:` syntax:

```c
m_Icon.LoadImageFile(0, "set:dayz_gui image:icon_warning");
```

Or in layout:

```
image "set:dayz_gui image:icon_warning"
```

See [[../styling/imagesets]] for managing imagesets.

## Engine truth

> Defined at lines 247–311 of `scripts/1_core/proto/enwidgets.c`.
> `LoadImageFile` returns `false` on failure — check the return value if
> dynamic image paths are involved.
