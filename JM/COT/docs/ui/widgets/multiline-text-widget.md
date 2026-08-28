# `MultilineTextWidget`

Multi-line variant of `TextWidget`. Wraps text into multiple lines and
adds line-breaking mode control.

## Class

```c
class MultilineTextWidget extends TextWidget
{
    proto native float SetLineBreakingOverride(int mode);
};
```

Everything from `TextWidget` works on `MultilineTextWidget` plus the line
break override.

## Layout properties

```
MultilineTextWidgetClass paragraph {
 size 1 1
 text "Long content that wraps across multiple lines as the widget grows in height."
 font "gui/fonts/sdf_MetronLight24"
 "exact text" 1
 "exact text size" 14
 "wrap text" 1            // enable wrapping (default 1 for this class)
 "linespacing" 2          // gap between lines in pixels
}
```

## Line-breaking modes

```
LINEBREAK_DEFAULT  // engine picks based on locale
LINEBREAK_WESTERN  // word-based, breaks on spaces and punctuation
LINEBREAK_ASIAN    // character-based, breaks anywhere (no spaces in CJK)
```

```c
m_Para.SetLineBreakingOverride(LINEBREAK_WESTERN);
```

Override per widget when displaying mixed content (e.g. a chat log that
contains both English and Chinese messages) and you want consistent
breaking behaviour.

## Sizing

`MultilineTextWidget` does **not** auto-resize to fit content. You set
the widget's box and it wraps content into that box. If the content
exceeds the box, it's clipped (or overflows if `clipchildren 0` on parent).

For auto-growing multi-line content (e.g. dynamic chat messages), put the
widget inside a `WrapSpacer` or `GridSpacer` with `"Size To Content V" 1`.

## Choosing between TextWidget and MultilineTextWidget

| Need | Choose |
|---|---|
| Single line, may be truncated | `TextWidget` |
| Multiple lines, wraps automatically | `MultilineTextWidget` |
| Text with inline images / colours | `RichTextWidget` |
| User-editable text (read AND write) | `EditBoxWidget` / `MultilineEditBoxWidget` |

## Engine truth

> Defined at lines 219–222 of `scripts/1_core/proto/enwidgets.c`.
> The `SetLineBreakingOverride` return type is documented as `float` in
> the engine declaration but functionally returns void / status — the
> return value is ignored in practice.
