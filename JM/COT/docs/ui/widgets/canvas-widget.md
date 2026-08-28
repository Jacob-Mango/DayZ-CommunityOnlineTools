# `CanvasWidget`

Script-drawn line primitive. Used for arbitrary 2D line drawing on top
of the UI.

## Class

```c
class CanvasWidget extends Widget
{
    proto native void DrawLine(float x1, float y1, float x2, float y2, float width, int color);
    proto native void Clear();
};
```

Only lines — no rectangles, circles, polygons, or arcs. If you need
filled shapes, layer `PanelWidget`s instead.

## Layout

```
CanvasWidgetClass overlay {
 ignorepointer 1     // typically — pass clicks through to underlying widgets
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
}
```

## Drawing lines

```c
// Clear previous frame's lines
m_Canvas.Clear();

// Draw a 2-pixel red line from (10,10) to (200,200)
m_Canvas.DrawLine(10, 10, 200, 200, 2.0, ARGB(255, 255, 0, 0));
```

**Coordinates are in monitor pixels relative to the widget's screen
position**, not fractions of the widget. `(0,0)` is the widget's
top-left corner.

## Lifetime of drawn lines

Lines persist until `Clear()` is called. The canvas does not auto-clear
between frames. For animated lines (e.g. mouse-drag selection box):

```c
override void Update(float timeSlice)
{
    m_Canvas.Clear();
    m_Canvas.DrawLine(m_StartX, m_StartY, m_CurrentX, m_StartY, 1, COLOR_WHITE);
    m_Canvas.DrawLine(m_CurrentX, m_StartY, m_CurrentX, m_CurrentY, 1, COLOR_WHITE);
    // …
}
```

## Use cases in DayZ

- ESP overlays — bone-to-bone skeleton lines (COT uses
  `JMESPModule.DrawSkeleton`)
- Drag-to-select rectangles
- Connection arrows between UI elements
- Crosshair / target marker custom shapes

## Approximating shapes

The COT map module uses many `DrawLine` calls in a loop to approximate
a circle (12 segments for safe-zones).

```c
int segments = 12;
float step = Math.PI2 / segments;
for (int i = 0; i < segments; i++)
{
    float a1 = step * i;
    float a2 = step * (i + 1);
    vector p1 = center + Vector(cos(a1), sin(a1), 0) * radius;
    vector p2 = center + Vector(cos(a2), sin(a2), 0) * radius;
    m_Canvas.DrawLine(p1[0], p1[2], p2[0], p2[2], 1, color);
}
```

## Performance

`DrawLine` is cheap individually but each call is a draw command. Drawing
hundreds of lines per frame is fine; thousands gets expensive. Batch
clears with `Clear()` rather than redrawing every frame.

## Engine truth

> Defined at lines 341–345 of `scripts/1_core/proto/enwidgets.c`. Only
> two methods. The colour is the standard ARGB int (alpha in high byte).
