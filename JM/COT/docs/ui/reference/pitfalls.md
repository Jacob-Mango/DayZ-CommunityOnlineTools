# Common Pitfalls

Bugs every DayZ mod author has hit at least once. Cross-referenced to
the docs that explain the underlying behaviour.

## "Engine crashes (CTD) when my custom-styled panel first becomes visible"

**Symptom:** Layout loads without errors; script init runs fine; widget
becomes visible → instant access violation (engine CTD with no script
trace).

**Cause:** A `<Style>` referenced by the layout (`style MyStyle`) is not
registered under the widget class that uses it. DayZ resolves styles by
`(widget class, style name)`. The widget loads anyway, but the render
path looks up the state block and finds nothing → null deref.

**Common mistake:** Defining a style only under `<Widget Name="PanelWidget">`
but then using `style MyStyle` on a `GridSpacerWidgetClass` or
`WrapSpacerWidgetClass`.

**Fix:** Define the style under EVERY widget type the layouts use it on.
See [[../styling/styles-system]] (the "Crash trap" section).

Also: `FrameWidget` does NOT support styles. Putting `style X` on
`FrameWidgetClass` is a silent bug — remove it or change the widget to
`PanelWidgetClass`.

---

## "My widget is huge / tiny / the wrong size"

**Symptom:** `SetSize(32, 32)` makes a widget either 32 px (good) or
32 × parent's width (very bad).

**Cause:** `hexactsize` / `vexactsize` flags determine whether `32`
means pixels or fractions. Without setting the flags, fractional is
default — so `SetSize(32, 32)` means "3200% × 3200% of parent".

**Fix:** Always set `hexactsize 1 vexactsize 1` (in layout) or call
`widget.SetFlags(WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE)` before
`SetSize`. COT's `UIActionBase.SetFixedSize(w, h)` does this for you.

See [[../coordinate-system]].

---

## "My label overflows into the next column"

**Symptom:** A label widget's text bleeds past its rightmost boundary
into adjacent widgets.

**Cause:** The widget itself is wider than its visible area. A common
mistake is `position 28 0 size 1 1` — widget origin at x=28, width 100%
of parent → right edge at x=28+parent_width, overflowing 28 px.

**Fix:** Use `"text offset"` instead of `position` to shift glyphs
without moving the widget:

```
position 0 0
size 1 1
"text offset" 28 0
```

See [[../widgets/text-widget]] for the difference.

---

## "Refresh icon doesn't spin"

**Symptom:** Calling `btn.TriggerSpin(2)` on a `UIActionImageButton`
appears to do nothing.

**Cause:** The base `UIActionButton.TriggerSpin` checks `m_Icon` (the
`action_icon` ImageWidget) which doesn't exist on `UIActionImageButton`
(which uses `action_image` named `m_Image` instead).

**Fix:** `UIActionImageButton` overrides `TriggerSpin` and `Update` —
ensure you're calling it on the correct subclass and that the override
is present (was added in the icon-only refresh button rework).

---

## "ConfirmInline shows cropped text on a 32px button"

**Symptom:** A 32×32 ConfirmInline button shows "Confir" / "Canc" when
clicked because the Confirm/Cancel sub-buttons get ~16px each.

**Fix:** `UIActionIconGrid.ApplyDeletePreset` now auto-calls
`UseIconConfirmLabels()` which swaps to `✓` / `✕` symbols. If you set
icon-only ConfirmInline manually without `ApplyDeletePreset`, call
`btn.UseIconConfirmLabels()` yourself.

---

## "My GridSpacer cells are empty / overflow"

**Symptom:** A `CreateGridSpacer(parent, 5, 2)` with 8 children leaves
2 empty cells; with 11 children, 1 overflows.

**Cause:** `Rows × Columns` must match the actual child count for clean
layout. Mismatched grids leave gaps or overflow rows.

**Fix:** Count children, set grid to match. For variable counts, use
a `1, 1` grid (which auto-grows vertically via `Size To Content V`).

See [[../widgets/gridspacer-widget]].

---

## "Dropdown / tooltip hidden behind list"

**Symptom:** Opening a dropdown shows the popup behind the listbox below
it.

**Cause:** `SetSort(N)` only reorders siblings within the same parent.
A dropdown popup that's a child of one panel can't render above a
listbox in a different panel — sort is sibling-scoped.

**Fix:** Parent the dropdown popup to the workspace root (or to the
form's `layoutRoot`):

```c
m_Popup = g_Game.GetWorkspace().CreateWidgets("…/list.layout", NULL);
m_Popup.SetSort(9999);
```

See [[../patterns/z-order]].

---

## "Checkbox fires CHANGE event during construction"

**Symptom:** Creating a checkbox with `checked=true` triggers your
`OnChange` handler before the form is fully built, leading to crashes
in handlers that reference yet-uninitialized fields.

**Cause:** `CreateCheckbox` previously called `SetChecked(true)` which
fires `UIEvent.CHANGE` synchronously.

**Fix:** Factory now uses `SetCheckedSilent(true)`. If you have custom
checkbox construction code, use `SetCheckedSilent` for the initial
value and only fire CHANGE on user interaction.

---

## "Refresh button stops working after I rebuild content"

**Symptom:** First click of refresh works; after a `delete content;
content = CreateGridSpacer(...)` rebuild, the button doesn't fire.

**Cause:** If the button is inside the rebuilt subtree, `delete` destroys
it. Subsequent calls on the now-dead pointer silently fail.

**Fix:** Keep the toolbar (and the refresh button) **outside** the
rebuilt subtree. Use:

```c
// In OnInit, build once:
m_Toolbar  = … with refresh btn …;
m_Content  = …;   // dynamic subtree

// On rebuild, only rebuild content:
if (m_Content) delete m_Content;
m_Content = UIActionManager.CreateGridSpacer(parent, 1, 1);
// m_Toolbar still alive, button still works
```

---

## "ApplyShape() resets the alpha and my check flickers"

**Symptom:** Initialising a checkbox as `checked=true` causes a brief
flash of the check icon at full alpha, then animating from 0 → 1.

**Cause:** Animation system + explicit `SetAlpha(1.0)` race. Explicit
calls snap to the final value, but the next animation tick reads its
stale internal Value (0) and overrides.

**Fix:** Use `SnapVisuals()` (added in the audit) which sets both the
animation's `Value` AND the widget property to match. For initialisation,
this avoids the flicker.

---

## "Setting `SetWidth(1.0)` on three buttons makes them wrap"

**Symptom:** Three siblings in a `WrapSpacer` each with `SetWidth(1.0)`
each take 100% width → 3 rows of one button each.

**Cause:** Fractional widths in a `WrapSpacer` sum to 1.0 to fit on one
line. `SetWidth(1.0)` says "I want all the remaining width".

**Fix:** Use fractions that sum to ≤ 1.0:

```c
btn1.SetWidth(0.33);
btn2.SetWidth(0.33);
btn3.SetWidth(0.34);
```

Or use a `GridSpacer(1, 3)` for guaranteed equal cells.

---

## "Hover effect persists when widget becomes invisible"

**Symptom:** Hovering a widget shows highlight; calling `Show(false)`
removes the widget but the highlight stays.

**Cause:** `OnMouseLeave` doesn't fire when a widget is hidden mid-hover.
The handler's state thinks the mouse is still over it.

**Fix:** Manually call your hover-out logic when hiding:

```c
m_Widget.Show(false);
if (m_IsHovered)
{
    OnMouseLeave(m_Widget, NULL, 0, 0);   // synthesise the leave
}
```

---

## "OnClick fires twice"

**Symptom:** Setting a handler on both the parent and the button causes
`OnClick` to fire twice — once for the button, once for the parent.

**Cause:** Both handlers see the event. The parent's runs after the
button's because the button's didn't return `true` to consume.

**Fix:** Either return `true` from the button's `OnClick` to stop
propagation, or only attach the handler at one level.

---

## "GetWidgetUnderCursor returns null when I expect a widget"

**Symptom:** The cursor is visibly over a widget but
`GetWidgetUnderCursor()` returns null.

**Cause:** The widget has `IGNOREPOINTER` set, so the engine treats it
as transparent for input purposes. The cursor "sees through" it to
whatever's underneath (or nothing).

**Fix:** Clear `IGNOREPOINTER` if you want the widget to be hit-testable.
Or check the ancestor / nearby widget instead.

---

## "Layout file changes don't apply"

**Symptom:** Edited a `.layout` file but the in-game UI still looks the
same.

**Cause:** Layouts are loaded from the mod pack at game launch. If you
edit the file but the engine has the old version cached (or the pack
hasn't been re-packed), the change won't appear.

**Fix:**
- For in-development testing: launch with `-noPause -filePatching` and
  ensure the layout file is in a loose unpacked directory.
- For released mods: re-pack the PBO.
- For COT specifically: the script path is symlinked to `DayZ Projects`,
  so edits there go live. Layouts are loaded fresh each game start.

---

## See also

- [[../coordinate-system]]
- [[../widget-flags]]
- [[../patterns/sizing-rules]]
- [[../patterns/z-order]]
- The COT-specific [`styling/style-guide.md`](../../styling/style-guide.md)
