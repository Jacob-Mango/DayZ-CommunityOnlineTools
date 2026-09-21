# `SpacerWidget` (base) and `SpacerBaseWidget`

The base for `GridSpacerWidget` and `WrapSpacerWidget`. Adds content
alignment but no auto-layout on its own.

## Class hierarchy

```c
class SpacerBaseWidget extends UIWidget
{
    proto native void AddChildAfter(Widget child, Widget after, bool immedUpdate = true);
};

class SpacerWidget extends SpacerBaseWidget
{
    proto native WidgetAlignment GetContentAlignmentH();
    proto native void            SetContentAlignmentH(WidgetAlignment alignment);
    proto native WidgetAlignment GetContentAlignmentV();
    proto native void            SetContentAlignmentV(WidgetAlignment alignment);
};

class GridSpacerWidget extends SpacerWidget {}
class WrapSpacerWidget extends SpacerWidget {}
class ScrollWidget    extends SpacerBaseWidget { ... };
```

Plain `SpacerWidget` isn't typically used in layouts — go straight to
`GridSpacer` or `WrapSpacer`.

## Content alignment

```c
spacer.SetContentAlignmentH(WidgetAlignment.WA_LEFT);    // WA_LEFT, WA_CENTER, WA_RIGHT
spacer.SetContentAlignmentV(WidgetAlignment.WA_TOP);     // WA_TOP, WA_CENTER, WA_BOTTOM
```

Controls how children are aligned **within** the spacer when total
children size is smaller than the spacer's size. The same enum values
overlap (WA_LEFT = WA_TOP = 0) — see [[../widget-flags]].

In a layout:

```
SpacerWidgetClass row {
 align horizontal_center
 align vertical_center
 // …
}
```

## `AddChildAfter`

```c
spacer.AddChildAfter(newChild, existingChild);
```

Inserts `newChild` immediately after `existingChild` in the sibling list.
Useful when you want to insert mid-list without re-creating the whole
spacer.

## Engine truth

> Defined at lines 460–471 of `scripts/1_core/proto/enwidgets.c`. The
> spacer base classes are not directly used in layouts; their subclasses
> (`GridSpacerWidget`, `WrapSpacerWidget`) are. See those for layout
> properties.
