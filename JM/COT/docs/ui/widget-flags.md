# WidgetFlags

`WidgetFlags` is a bitmask that controls rendering, input, and sizing
behaviour. It's set via `widget.SetFlags(flags)` or read via
`widget.GetFlags()`. Layout files don't set the raw bitmask directly —
each flag has a corresponding layout property (e.g. `hexactpos 1` sets
the `HEXACTPOS` bit).

## The full enum

From `scripts/1_core/proto/enwidgets.c`:

| Flag | Meaning |
|---|---|
| `SOURCEALPHA` | Take alpha from texture × colour alpha. If unset, treat texture as opaque, only colour alpha applies. |
| `BLEND` | Texture is alpha-blended with the surface. |
| `ADDITIVE` | Texture is added to the surface based on alpha. |
| `VISIBLE` | Widget is visible (`Show(true)` toggles this). |
| `NOWRAP` | Don't wrap texture (single-tile). |
| `CENTER` | Centre text horizontally (TextWidget). |
| `VCENTER` | Centre text vertically (TextWidget). |
| `HEXACTPOS` | `X` is in pixels, not fraction. |
| `VEXACTPOS` | `Y` is in pixels. |
| `EXACTPOS` | Convenience: both `HEXACTPOS` and `VEXACTPOS`. |
| `HEXACTSIZE` | `W` is in pixels. |
| `VEXACTSIZE` | `H` is in pixels. |
| `EXACTSIZE` | Convenience: both. |
| `NOFILTER` | Disable texture filtering (no blur on scaling). |
| `RALIGN` | Right-align text. |
| `STRETCH` | Stretch texture to full widget size. |
| `FLIPU` | Flip texture horizontally. |
| `FLIPV` | Flip texture vertically. |
| `CUSTOMUV` | Use UVs set by `SetUV()` instead of computed. |
| `IGNOREPOINTER` | Widget never receives mouse input — clicks pass through. |
| `DISABLED` | Widget is disabled — greyed out, no input. |
| `NOFOCUS` | Widget cannot receive focus (controller/keyboard navigation skips it). |
| `CLIPCHILDREN` | Children cannot draw outside this widget's rectangle. |
| `RENDER_ALWAYS` | Render even when off-screen / behind opaque widgets. |
| `NOCLEAR` | Don't clear render buffer between frames. |
| `DRAGGABLE` | Widget can be dragged with the mouse. |

## Setting and clearing flags

```c
widget.SetFlags(WidgetFlags.IGNOREPOINTER);   // adds the bit
widget.ClearFlags(WidgetFlags.IGNOREPOINTER); // removes the bit
widget.SetFlags(WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS);
```

> **Engine quirk:** `SetFlags` is documented as "ADDS the value to the
> existing flag" and `ClearFlags` "SUBSTRACTS" — they're additive
> bitmask operations, NOT replace. Calling `SetFlags(0)` does not clear
> all flags; it's a no-op. Use `ClearFlags(0xFFFFFFFF)` if you really need
> to zero everything (rare).

## Layout property mapping

Most flags have a layout property:

| Layout property | Flag |
|---|---|
| `hexactpos 1` | `HEXACTPOS` |
| `vexactpos 1` | `VEXACTPOS` |
| `hexactsize 1` | `HEXACTSIZE` |
| `vexactsize 1` | `VEXACTSIZE` |
| `visible 1` | `VISIBLE` |
| `ignorepointer 1` | `IGNOREPOINTER` |
| `clipchildren 1` | `CLIPCHILDREN` |
| `draggable 1` | `DRAGGABLE` |
| `mode blend` | `BLEND` |
| `mode additive` | `ADDITIVE` |
| `"src alpha" 1` | `SOURCEALPHA` |
| `"no wrap" 1` | `NOWRAP` |
| `"no focus" 1` | `NOFOCUS` |
| `nofilter 1` | `NOFILTER` |

## Flags you'll touch often

### `IGNOREPOINTER`
The most-used flag. Set on **decorative** widgets (text labels, background
panels, dividers) so the underlying clickable widget receives the click.

```
TextWidgetClass title {
 ignorepointer 1   // user can click "through" the title to the button below
 text "Click me"
}
```

### `CLIPCHILDREN`
Prevents children from drawing outside the parent rectangle. **Not on
by default** — overflow is allowed. Set on tooltips, scrollable lists,
clipped marquees.

```
PanelWidgetClass clip_box {
 clipchildren 1
 size 200 30
 {
  TextWidgetClass long_text {
   size 500 30    // wider than parent → would overflow without clipchildren
   text "Very long line that gets cropped at the panel boundary"
  }
 }
}
```

### `DRAGGABLE`
The widget can be picked up with the mouse. The engine emits `OnDrag` /
`OnDragging` / `OnDrop` events. Set on window headers, the COT main
window root, etc.

### `HEXACTSIZE` / `VEXACTSIZE` mismatch
The most common runtime bug: setting `SetSize(32, 32)` on a widget whose
flags say "size is fractional" results in a widget 32× the parent's width.
Use `SetFixedSize` in COT (`UIActionBase.SetFixedSize`) or call
`SetFlags(WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE)` before
`SetSize`.

## Engine truth

> The enum is defined at lines 57–85 of
> `scripts/1_core/proto/enwidgets.c`. The set/clear methods:
> ```
> proto native int GetFlags();
> proto native int SetFlags(int flags, bool immedUpdate = true); //! ADDS
> proto native int ClearFlags(int flags, bool immedUpdate = true); //! SUBSTRACTS
> ```
