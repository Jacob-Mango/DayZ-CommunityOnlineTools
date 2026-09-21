# `MultilineEditBoxWidget`

Multi-line text input — like a `<textarea>` element. Supports per-line
access and the caret position.

## Class

```c
class MultilineEditBoxWidget extends TextWidget
{
    proto native int  GetLinesCount();
    proto native int  GetCarriageLine();
    proto native int  GetCarriagePos();
    proto void        GetText(out string text);
    proto native void SetLine(int line, string text);
    proto void        GetLine(int line, out string text);
};
```

Note that it extends `TextWidget`, **not** `EditBoxWidget`. There's no
shared base for editable text widgets.

## Layout

```
MultilineEditBoxWidgetClass description {
 position 0 0
 size 1 100
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 font "gui/fonts/sdf_MetronLight24"
 "exact text" 1
 "exact text size" 14
 "wrap text" 1
 style DayZNormal
}
```

## Per-line access

```c
int lines = m_EditBox.GetLinesCount();
string line0;
m_EditBox.GetLine(0, line0);

m_EditBox.SetLine(2, "Replaces line 2");
```

`SetLine` rewrites a specific line; the rest are untouched.

## Reading the full text

`GetText` returns the entire content with embedded newlines:

```c
string content;
m_EditBox.GetText(content);
```

## Caret position

```c
int line = m_EditBox.GetCarriageLine();   // 0-indexed
int col  = m_EditBox.GetCarriagePos();    // column within that line
```

Useful for syntax-highlighting markers or auto-complete popups anchored
to the caret.

## Events

Same as [[editbox-widget]]:
- `OnChange(w, x, y, finished)` — fires on each keypress; `finished = true` on focus loss
- `OnFocus` / `OnFocusLost`

The line and caret position will already be updated by the time `OnChange`
fires.

## Common pitfalls

### Tab inserts a tab character

Pressing Tab in a focused multi-line edit box inserts `\t` into the text,
not advances focus. To support Tab-to-next-field, you have to intercept
`OnKeyDown` and manually call `SetFocus` on the next widget.

### Pasting clears formatting silently

Newlines are preserved, but Word/web formatting is stripped on paste.

## Engine truth

> Defined at lines 313–321 of `scripts/1_core/proto/enwidgets.c`. Note
> the unusual class hierarchy: extends `TextWidget` (not `EditBoxWidget`),
> so methods like `IsChecked` don't exist on it.
