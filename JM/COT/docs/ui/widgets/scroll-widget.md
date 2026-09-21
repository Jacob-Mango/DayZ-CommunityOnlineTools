# `ScrollWidget`

Scrollable container with optional horizontal and vertical scrollbars.
Wraps any content that may exceed its visible area.

## Class

```c
class ScrollWidget extends SpacerBaseWidget
{
    proto native float GetScrollbarWidth();
    proto native bool  IsScrollbarVisible();    // reflects C++ side state

    proto native float GetContentWidth();
    proto native float GetContentHeight();

    proto native float GetHScrollPos();
    proto native float GetHScrollPos01();
    proto native bool  HScrollStep(int steps);
    proto native void  HScrollToPos(float pos);
    proto native void  HScrollToPos01(float pos01);
    proto native void  HScrollToWidget(Widget child);

    proto native float GetVScrollPos();
    proto native float GetVScrollPos01();
    proto native bool  VScrollStep(int steps);
    proto native void  VScrollToPos(float pos);
    proto native void  VScrollToPos01(float pos01);
    proto native void  VScrollToWidget(Widget child);
};
```

## Layout

```
ScrollWidgetClass main_scroll {
 ignorepointer 0
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 "Scrollbar V" 1            // show vertical scrollbar
 "Scrollbar H" 0            // hide horizontal
 {
  PanelWidgetClass content { ... }
 }
}
```

The single child is the scrollable content. It can be any widget — a
panel, a grid spacer, a wrap spacer. Children wider/taller than the
ScrollWidget overflow into the scrollable area.

## Scrolling APIs

### Get/set position in pixels

```c
float y = m_Scroll.GetVScrollPos();      // pixel offset
m_Scroll.VScrollToPos(100);                // scroll to 100 px down
```

### Get/set position as fraction `[0..1]`

```c
float pct = m_Scroll.GetVScrollPos01();   // 0 = top, 1 = bottom
m_Scroll.VScrollToPos01(0.5);              // scroll to halfway
```

### Step by lines

```c
m_Scroll.VScrollStep(1);   // step down one notch
m_Scroll.VScrollStep(-1);  // step up
```

The step size is engine-controlled (typically 20–30 px per step). Returns
`false` if already at the end.

### Scroll to a specific child

```c
m_Scroll.VScrollToWidget(m_LastEntry);
```

Most useful pattern for auto-scrolling: append a new entry, then scroll
to make it visible.

## Content dimensions

```c
float w = m_Scroll.GetContentWidth();    // pixel width of the child
float h = m_Scroll.GetContentHeight();
```

Use to detect overflow:

```c
float vw, vh;
m_Scroll.GetScreenSize(vw, vh);
bool overflows = h > vh;
```

## Scrollbar width

```c
float sbw = m_Scroll.GetScrollbarWidth();
```

Returns the width in pixels reserved for the vertical scrollbar. Used to
compute "content width minus scrollbar" for child widgets that need to
fit.

## `IsScrollbarVisible`

Returns whether the engine is currently showing the scrollbar (only
visible when content exceeds the viewport). Reflects the **C++ state**,
not just the property — useful for hiding/showing UI elements based on
scroll need.

## COT wrapper: `UIActionScroller`

```c
UIActionScroller scroller = UIActionManager.CreateScroller(parent);
Widget content = scroller.GetContentWidget();
// add children to content
scroller.UpdateScroller();  // call after adding/removing children
```

`UIActionScroller.UpdateScroller()` triggers a re-layout so the scroll
range is recomputed. **Always call it after adding or removing children**
or the scroll range may be wrong.

## Common pitfalls

### Forgetting `UpdateScroller`

After `CreateWidgets` adds new content, the engine's measured content
height may be stale. Call `m_Scroller.UpdateScroller()` to refresh.

### Scroll widget inside a tight parent

The scroll widget's size is what determines the viewport. If you place a
`ScrollWidget` inside a 100×100 frame, the viewport is 100×100. If the
frame's size depends on its parent's size, ensure the chain resolves
before the scroll widget calculates content.

### Horizontal scrolling rarely used

`"Scrollbar H" 1` enables horizontal scrolling, but most COT UIs use
vertical-only. Lists and forms scroll downward; horizontal scrolling is
reserved for unusual cases like long tables.

## Engine truth

> Defined at lines 481–502 of `scripts/1_core/proto/enwidgets.c`. The
> only widget that descends directly from `SpacerBaseWidget` (skipping
> `SpacerWidget`), so no content alignment methods.
