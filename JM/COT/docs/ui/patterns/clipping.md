# Clipping

When children overflow their parent's rectangle, the parent decides
whether to clip them or let them draw beyond. Default is **no
clipping** — overflow is allowed.

## Enabling clipping

Set the `CLIPCHILDREN` flag (layout: `clipchildren 1`):

```
PanelWidgetClass clip_box {
 clipchildren 1
 size 200 30
 hexactsize 1
 vexactsize 1
 {
  TextWidgetClass overflow_text {
   size 500 30          // wider than parent
   text "Long text that gets cropped at the parent's right edge"
  }
 }
}
```

The text is now clipped to the 200 × 30 rectangle of the parent.

## Which widgets clip by default

- `FrameWidget` — does NOT clip by default (`clipchildren 0`); add the
  flag to enable.
- `PanelWidget` — does NOT clip by default.
- `ScrollWidget` — DOES clip (it has to, to implement the viewport).

## When you need clipping

### Lists with finite visible area

A scrolling list has 50 items but only 20 fit. The `ScrollWidget`
already clips; nothing extra needed.

### Text marquee

To scroll a long title within a fixed-width header:

```
PanelWidgetClass clip {
 clipchildren 1
 size 1 30
 {
  TextWidgetClass title {
   size 500 30
   text "Long title that animates with SetPos to scroll horizontally"
  }
 }
}
```

The text widget is positioned via `SetPos(-offset, 0)` to scroll it left
under the clip rectangle. See `JMSidebarScrollEntry` for the COT
implementation.

### Image masks / partial reveals

Combine `clipchildren` with `SetPos` animation to slide content under
a fixed window.

## What clipping does NOT do

- **Doesn't change layout** — children still occupy their full declared
  size; the layout engine doesn't know they're being cropped.
- **Doesn't prevent input** — children outside the parent rectangle
  still receive mouse events if they're under the cursor (unless the
  child is `IGNOREPOINTER`).
- **Doesn't cascade** — only the direct parent's clip applies. If
  grandparent has clipping but parent doesn't, the child can overflow
  the parent (but not the grandparent).

## Overflow vs scrolling

| Behaviour | Configure with |
|---|---|
| Child draws past parent's bounds | Default (no flag) |
| Child cropped at parent's bounds | `clipchildren 1` |
| Child scrolls within a viewport with scrollbar | Use `ScrollWidget` |

The third is what you want for most "long content in fixed pane" cases.

## Frame as a pure clip container

If you want clipping without a visible background:

```
FrameWidgetClass clip_only {
 clipchildren 1
 ignorepointer 1     // don't intercept clicks
 size 1 1
 {
  // children that will be clipped to this frame's size
 }
}
```

`FrameWidget` is invisible by design; adding `clipchildren` gives a pure
clip region.

## Pitfall: clipping vs anchor

Negative-positioned children (e.g. `position -10 0`) are clipped if
they're outside the parent's bounds. To intentionally show a child
hanging off the edge (like a tab that overlaps an adjacent panel),
**don't** set `clipchildren`.

## Engine truth

> `CLIPCHILDREN` is a flag in the `WidgetFlags` enum (line 81 of
> `scripts/1_core/proto/enwidgets.c`). The layout property is
> `clipchildren` (no underscore). Some vanilla layouts use
> `clip-children` — the parser accepts both.
