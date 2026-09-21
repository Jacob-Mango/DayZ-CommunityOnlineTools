# `PasswordEditBoxWidget`

Single-line text input with optional character masking.

## Class

```c
class PasswordEditBoxWidget extends EditBoxWidget
{
    proto native void SetHideText(bool hide);
};
```

Inherits everything from [[editbox-widget]].

## Behaviour

- When `SetHideText(true)`, each character renders as `•` (or whatever
  the style defines as the mask glyph).
- When `SetHideText(false)`, renders normally — useful for a
  "show password" toggle.
- `GetText()` always returns the **actual** text regardless of masking.

## Layout

```
PasswordEditBoxWidgetClass password {
 position 0 0
 size 1 28
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 style Default
 hide 1                 // start with masking enabled
}
```

## Common use

Server password prompts on the main menu, the rare admin password input.
COT doesn't use this widget (admin auth is permission-based, not
password-based).

## Engine truth

> Defined at lines 353–356 of `scripts/1_core/proto/enwidgets.c`.
> Only one method on top of `EditBoxWidget`.
