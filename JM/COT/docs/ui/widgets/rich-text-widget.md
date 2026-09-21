# `RichTextWidget`

Multi-line text **with inline images** and per-region styling. Used for
chat, tooltips with icons, and complex text blocks like the inspect menu.

## Class

```c
class RichTextWidget extends TextWidget
{
    proto native float GetContentHeight();
    proto native float GetContentOffset();
    proto native void  SetContentOffset(float offset, bool snapToLine = false);
    proto native void  ElideText(int line, float maxWidth, string str);
    proto native int   GetNumLines();
    proto native void  SetLinesVisibility(int lineFrom, int lineTo, bool visible);
    proto native float GetLineWidth(int line);
    proto native float SetLineBreakingOverride(int mode);
};
```

## Inline image markup

```
<image set="dayz_gui" name="icon_warning" size="24"/> Warning text
```

- `set` — name of an `.imageset` (see [[../styling/imagesets]])
- `name` — slice name within the imageset
- `size` — vertical pixel size (width scales proportionally)

You can also embed images by path:

```
<img path="JM/COT/GUI/textures/icons/trash.paa" width="20" height="20"/>
```

## Inline styling

```
<linebreak/>
<center>centred line</center>
<image set="..." name="..."/>
```

The full markup vocabulary is engine-internal and not officially
documented; the above tags are the ones used in vanilla layouts.

## Scrolling content

`RichTextWidget` has its own scroll offset, separate from `ScrollWidget`:

```c
float maxOffset = m_Rich.GetContentHeight() - widget_height;
m_Rich.SetContentOffset(maxOffset);     // scroll to bottom
m_Rich.SetContentOffset(0);              // scroll to top
m_Rich.SetContentOffset(50, true);       // scroll to 50 px, snap to nearest line
```

Useful when the rich text widget has more lines than fit visually and you
want to programmatically scroll (chat auto-scroll to latest message).

## Hiding lines

```c
m_Rich.SetLinesVisibility(5, 9, false);   // hide lines 5..9
m_Rich.SetLinesVisibility(0, m_Rich.GetNumLines(), true);  // show all
```

## Line-by-line metrics

```c
int n = m_Rich.GetNumLines();
for (int i = 0; i < n; i++)
{
    float w = m_Rich.GetLineWidth(i);
    // …
}
```

## Eliding text

```c
m_Rich.ElideText(0, 200.0, "Very long string that should be …-truncated");
```

Replaces line 0 with a truncated version that fits in 200 pixels of width.
The engine adds an ellipsis at the cut point.

## Layout

```
RichTextWidgetClass content {
 size 1 1
 text "Welcome <image set=\"dayz_gui\" name=\"icon_check\" size=\"16\"/> back!"
 "wrap text" 1
}
```

## Performance note

Rich text is significantly more expensive to lay out than plain text,
especially with many inline images. For HUDs that update every frame,
prefer separate `TextWidget` + `ImageWidget` combinations.

## Engine truth

> Defined at lines 224–234 of `scripts/1_core/proto/enwidgets.c`.
