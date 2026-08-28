# `FrameWidget`

An **invisible** hierarchy node. Used purely for layout grouping and
optionally for clipping. Has no visual representation of its own.

## When to use

- Wrap a group of widgets without adding any background
- Apply `clipchildren` without painting a panel
- Provide a layout reference for positioning (anchor point)

## How it differs from `PanelWidget`

| Aspect | PanelWidget | FrameWidget |
|---|---|---|
| Visible | Yes (paints `color`) | No (always transparent) |
| Children | Yes | Yes |
| `clipchildren` | Yes | Yes |
| Style support | Yes | No |
| Default colour | None (you set it) | N/A |

Choose `FrameWidget` when you don't want a background; choose
`PanelWidget` with `color 0 0 0 0` when you might add one later.

## Layout example

```
FrameWidgetClass content_root {
 ignorepointer 1
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 {
  PanelWidgetClass left_pane  { size 0.3 1 halign left_ref }
  PanelWidgetClass right_pane { size 0.7 1 halign right_ref }
 }
}
```

The frame here groups two panels without adding visual noise.

## `fixaspect` property

`FrameWidget` (and a few others) accepts a `fixaspect` property:

```
fixaspect none      // default
fixaspect inside    // letterbox: scale to fit, preserve aspect
fixaspect outside   // pan-and-scan: cover, preserve aspect
```

This is used by full-screen layouts to handle ultrawide vs 16:9 vs 4:3
without distorting content.

## Engine truth

> The vanilla docs say `FrameWidget` is a "dummy frame, used as hierarchy
> node and clipper" (see `WidgetType` comments in `enwidgets.c`).
> `EmbededWidget` is the cousin used for embedding **another layout file**
> as a single child — see [[../patterns/embedding]].
