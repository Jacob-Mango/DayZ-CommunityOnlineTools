# `EditBoxWidget`

Single-line text input. The user can type, select, copy, paste, etc.

## Class

```c
class EditBoxWidget extends UIWidget
{
    proto string GetText();
    proto native void SetText(string str);
};
```

`UIWidget` adds the colour / outline / shadow / italic / bold methods —
see [[../styling/fonts-and-text]].

## Layout

```
EditBoxWidgetClass search_box {
 position 0 0
 size 1 28
 halign center_ref
 valign center_ref
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 style Default
 "limit visible" 1     // only show characters that fit
 "exact text" 0
 font "gui/fonts/sdf_MetronLight24"
 color 0 0 0 1
}
```

## Properties unique to edit boxes

| Property | Meaning |
|---|---|
| `"limit visible" 1` | Only render characters that fit; hide overflow |
| `"max chars" N` | Limit input length to N characters |
| `"chars allowed" "0123456789."` | Whitelist of allowed characters |

## Events

- `OnChange(w, x, y, finished)` — fired on every character typed; `finished` is `true` when the user presses Enter or focus leaves.
- `OnFocus(w, x, y)` — focus gained.
- `OnFocusLost(w, x, y)` — focus lost (Tab, click outside).
- `OnKeyDown` / `OnKeyPress` — raw key events (use sparingly; engine handles most input)

> **`OnChange`'s `finished` flag is the key distinction.** Most "search
> as you type" implementations want `OnChange` regardless of `finished`,
> while "commit on Enter" wants to ignore `OnChange` calls where
> `!finished`.

## Reading the value

```c
string typed = m_EditBox.GetText();
```

`GetText` returns the **current displayed text** including any in-progress
edits.

## COT wrapper: `UIActionEditableText`

COT wraps `EditBoxWidget` in `UIActionEditableText.layout` to add:
- An optional label widget on the left
- A focus highlight panel
- The disabled overlay
- Helper methods: `SetText`, `GetText`, `SetOnlyNumbers(bool, bool)`

`SetOnlyNumbers(numbersOnly, allowDecimal)` writes appropriate
`"chars allowed"` to the underlying edit box.

## Common pitfalls

### Default text not preserved across rebuilds

`SetText` on an `EditBoxWidget` whose parent was rebuilt resets to empty.
If you rebuild a form, re-call `SetText` afterwards.

### Disabled edit box doesn't actually disable input

`Enable(false)` on a vanilla `EditBoxWidget` greys it but **doesn't always
stop input**. Use `widget.SetFlags(WidgetFlags.IGNOREPOINTER)` to be safe.

### Pasting binary or multiline content

The single-line edit box silently strips newlines from pasted text. For
multi-line input, use [[multiline-editbox-widget]].

## Engine truth

> Defined at lines 347–351 of `scripts/1_core/proto/enwidgets.c`. The
> getter `GetText()` returns by value — assigning it to a local copies.
