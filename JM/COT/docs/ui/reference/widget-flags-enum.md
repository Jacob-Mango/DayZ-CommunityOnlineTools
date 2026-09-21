# WidgetFlags Enum — Full Reference

From `scripts/1_core/proto/enwidgets.c` lines 57–85.

```c
enum WidgetFlags
{
    SOURCEALPHA,
    BLEND,
    ADDITIVE,
    VISIBLE,
    NOWRAP,
    CENTER,
    VCENTER,
    HEXACTPOS,
    VEXACTPOS,
    EXACTPOS,
    HEXACTSIZE,
    VEXACTSIZE,
    EXACTSIZE,
    NOFILTER,
    RALIGN,
    STRETCH,
    FLIPU,
    FLIPV,
    CUSTOMUV,
    IGNOREPOINTER,
    DISABLED,
    NOFOCUS,
    CLIPCHILDREN,
    RENDER_ALWAYS,
    NOCLEAR,
    DRAGGABLE
};
```

## Per-flag detail

### Rendering & alpha

| Flag | Layout property | Effect |
|---|---|---|
| `SOURCEALPHA` | `"src alpha" 1` | Texture's alpha × colour alpha. Without: texture treated as opaque, only colour alpha applies. |
| `BLEND` | `mode blend` | Texture alpha-blended with surface (normal transparency). |
| `ADDITIVE` | `mode additive` | Texture added to surface (glow effects). |
| `NOFILTER` | `nofilter 1` | No texture filtering — pixel-perfect scaling. Useful for pixel art. |
| `RENDER_ALWAYS` | `"render always" 1` | Render even when off-screen / occluded. Performance cost. |
| `NOCLEAR` | `noclear 1` | Don't clear the render buffer between frames. Effects with trails. |

### Visibility & input

| Flag | Layout property | Effect |
|---|---|---|
| `VISIBLE` | `visible 1` | Widget is drawn. Default on. Toggle with `Show(true/false)`. |
| `IGNOREPOINTER` | `ignorepointer 1` | Mouse events pass through. Decorative widgets. |
| `DISABLED` | `disabled 1` | Greys out, no input. Toggle with `Enable(true/false)`. |
| `NOFOCUS` | `"no focus" 1` | Excluded from controller/keyboard navigation. |

### Position & size

| Flag | Layout property | Effect |
|---|---|---|
| `HEXACTPOS` | `hexactpos 1` | `X` in pixels |
| `VEXACTPOS` | `vexactpos 1` | `Y` in pixels |
| `EXACTPOS` | — | Convenience: both `HEXACTPOS` and `VEXACTPOS` |
| `HEXACTSIZE` | `hexactsize 1` | `W` in pixels |
| `VEXACTSIZE` | `vexactsize 1` | `H` in pixels |
| `EXACTSIZE` | — | Both |

### Text rendering

| Flag | Layout property | Effect |
|---|---|---|
| `CENTER` | `"text halign" center` | Horizontal text centring |
| `VCENTER` | `"text valign" center` | Vertical text centring |
| `RALIGN` | `"text halign" right` | Right-align text |

### Texture wrapping

| Flag | Layout property | Effect |
|---|---|---|
| `NOWRAP` | `"no wrap" 1` | Don't tile the texture |
| `STRETCH` | `"stretch mode" stretch_w_h` | Stretch to full widget size |
| `FLIPU` | `"flip u" 1` | Mirror horizontally |
| `FLIPV` | `"flip v" 1` | Mirror vertically |
| `CUSTOMUV` | `customuv 1` | Use UVs from `SetUV` instead of computed |

### Children

| Flag | Layout property | Effect |
|---|---|---|
| `CLIPCHILDREN` | `clipchildren 1` | Children cannot draw outside this widget. |

### Interaction

| Flag | Layout property | Effect |
|---|---|---|
| `DRAGGABLE` | `draggable 1` | Widget can be dragged with mouse. |

## Setting and clearing

```c
widget.SetFlags(WidgetFlags.IGNOREPOINTER);                              // adds
widget.SetFlags(WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE);        // adds multiple
widget.ClearFlags(WidgetFlags.IGNOREPOINTER);                            // removes
int current = widget.GetFlags();                                          // reads
```

`SetFlags` ADDS bits (OR); `ClearFlags` REMOVES bits (AND NOT). Neither
replaces the full bitmask.

## Engine truth

> The enum is at lines 57–85 of `scripts/1_core/proto/enwidgets.c`. The
> documentation comments on each line describe the effect; the comments
> in the table above are paraphrased from those.
