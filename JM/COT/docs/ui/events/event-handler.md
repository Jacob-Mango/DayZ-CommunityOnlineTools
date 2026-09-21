# `ScriptedWidgetEventHandler`

The class that receives mouse, keyboard, drag, focus, and lifecycle
events for one or more widgets. Subclass it and attach with
`widget.SetHandler(handler)`.

## Full callback list

From `scripts/1_core/proto/enwidgets.c`:

```c
class ScriptedWidgetEventHandler: Managed
{
    bool OnClick(Widget w, int x, int y, int button);
    bool OnModalResult(Widget w, int x, int y, int code, int result);
    bool OnDoubleClick(Widget w, int x, int y, int button);
    bool OnSelect(Widget w, int x, int y);
    bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn);
    bool OnFocus(Widget w, int x, int y);
    bool OnFocusLost(Widget w, int x, int y);
    bool OnMouseEnter(Widget w, int x, int y);
    bool OnMouseLeave(Widget w, Widget enterW, int x, int y);
    bool OnMouseWheel(Widget w, int x, int y, int wheel);
    bool OnMouseButtonDown(Widget w, int x, int y, int button);
    bool OnMouseButtonUp(Widget w, int x, int y, int button);
    bool OnController(Widget w, int control, int value);  // controlID enum
    bool OnKeyDown(Widget w, int x, int y, int key);
    bool OnKeyUp(Widget w, int x, int y, int key);
    bool OnKeyPress(Widget w, int x, int y, int key);
    bool OnChange(Widget w, int x, int y, bool finished);
    bool OnDrag(Widget w, int x, int y);
    bool OnDragging(Widget w, int x, int y, Widget reciever);
    bool OnDraggingOver(Widget w, int x, int y, Widget reciever);
    bool OnDrop(Widget w, int x, int y, Widget reciever);
    bool OnDropReceived(Widget w, int x, int y, Widget reciever);
    bool OnResize(Widget w, int x, int y);
    bool OnChildAdd(Widget w, Widget child);
    bool OnChildRemove(Widget w, Widget child);
    bool OnUpdate(Widget w);
    bool OnEvent(EventType eventType, Widget target, int parameter0, int parameter1);
};
```

## Return value semantics

Every callback returns `bool`:

- `true` — event was **consumed**. The engine stops looking for further
  handlers up the tree.
- `false` — event **propagates**. The engine continues walking up the
  parent chain, calling each ancestor's handler.

> **Default is "not consumed".** Failing to return anything means
> `false`, and other handlers may run. If you want to fully handle an
> event, always `return true`.

## Attaching a handler

```c
class MyForm: JMFormBase  // extends ScriptedWidgetEventHandler indirectly
{
    override void OnInit()
    {
        m_RefreshBtn.SetHandler(this);   // explicitly attach
        // The COT JMFormBase auto-attaches `this` to the root layout widget.
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_RefreshBtn)
        {
            DoRefresh();
            return true;
        }
        return false;
    }
}
```

## Dispatch chain

Engine dispatches events as follows for a mouse click at `(x, y)`:

1. Find the topmost visible widget under `(x, y)` that doesn't have
   `IGNOREPOINTER` set.
2. Call that widget's handler's `OnClick`.
3. If it returns `false`, walk to the widget's parent, call its handler's
   `OnClick`. Repeat until consumed or root reached.

You only need to handle events on the **specific widget** you care about;
ancestors' handlers will see clicks on their descendants too.

## Common event signatures

### OnClick

```c
override bool OnClick(Widget w, int x, int y, int button)
{
    if (button != 0) return false;   // 0 = left, 1 = right, 2 = middle
    // …
}
```

`x, y` are screen pixels (monitor coords).

### OnChange

Fires when an input widget's value changes (slider, edit box, checkbox).

```c
override bool OnChange(Widget w, int x, int y, bool finished)
{
    if (w != m_Slider) return false;
    if (!finished) return true;       // only act on final value
    // …
}
```

### OnMouseEnter / OnMouseLeave

```c
override bool OnMouseEnter(Widget w, int x, int y)
{
    if (w == m_RowPanel)
    {
        m_RowPanel.SetColor(COLOR_HIGHLIGHT);
    }
    return false;
}

override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
{
    if (w == m_RowPanel)
    {
        m_RowPanel.SetColor(COLOR_NORMAL);
    }
    return false;
}
```

`enterW` (in `OnMouseLeave`) is the widget the cursor is moving **to**.

### OnItemSelected

Fires on a `TextListboxWidget` when selection changes.

```c
override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
{
    if (w != m_List) return false;
    UpdateDetails(row);
    return true;
}
```

### OnUpdate

Fires every frame (if you subclass to receive it). Mostly used for HUD
elements that animate continuously.

## `OnModalResult`

Specific to modal dialogs (`SetModal`). The `code` and `result` are
modal-specific return values — usually `0`/`1` for cancel/confirm.

## `OnEvent`

The generic catch-all. `eventType` is an `EventType` (TypeID); the engine
fires custom events for things like child reparenting. Rarely needed in
mods.

## Mixing handler + script class

A widget can have **both** a `scriptclass` (constructed by the engine
when the layout loads) and a `handler` (set via `SetHandler`). They're
independent — the script class is the widget's "behaviour", the handler
is the "input listener".

`JMFormBase` uses both: itself as the script class **and** as its own
handler, by extending `ScriptedWidgetEventHandler` and setting
`layoutRoot.SetHandler(this)` in `OnInit`.

## Pitfalls

### Forgetting to return true

If you handle a click but return `false`, the parent's handler also gets
called. Subtle bugs result when the parent has its own click logic.

### `OnChange` fires during drag

For sliders, `OnChange` fires continuously while the user drags. Guard
with `finished == true` to avoid spamming server RPCs.

### Listbox selection edge case

`OnItemSelected` does NOT fire if the row is already selected and clicked
again. Use `OnClick` if you need "click row to act, even if already
selected".

## Engine truth

> Defined at lines 656–686 of `scripts/1_core/proto/enwidgets.c`. The
> base class has empty bodies; you override only what you need.
