# Drag and Drop

DayZ has a built-in drag-and-drop system used heavily by the inventory.
A draggable widget fires a chain of events that lets you implement
move, copy, and validation logic.

## Enabling drag on a widget

```
PanelWidgetClass draggable_card {
 draggable 1
 ...
}
```

Or programmatically:

```c
widget.SetFlags(WidgetFlags.DRAGGABLE);
```

A draggable widget responds to mouse-down + drag, NOT just click.

## The drag event chain

```c
bool OnDrag(Widget w, int x, int y);
bool OnDragging(Widget w, int x, int y, Widget reciever);
bool OnDraggingOver(Widget w, int x, int y, Widget reciever);
bool OnDrop(Widget w, int x, int y, Widget reciever);
bool OnDropReceived(Widget w, int x, int y, Widget reciever);
```

Sequence for a successful drag-and-drop:

1. `OnDrag(source, x, y)` — fired once when the drag starts.
   Return `true` to allow the drag (default), `false` to cancel.
2. `OnDragging(source, x, y, hoverWidget)` — fired continuously while
   the cursor moves. `hoverWidget` is whatever's under the cursor.
3. `OnDraggingOver(target, x, y, source)` — fired on potential drop
   targets as the cursor passes over them.
4. `OnDrop(source, x, y, target)` — fired on the **source** when the
   mouse button is released.
5. `OnDropReceived(target, x, y, source)` — fired on the **target**
   when the drop completes.

## Validating the drop

In `OnDraggingOver`, return `true` to indicate the widget is a valid
drop target. The engine uses this to control cursor / highlight feedback.

```c
override bool OnDraggingOver(Widget w, int x, int y, Widget reciever)
{
    if (w == m_DropZone)
    {
        // accept only certain source types
        if (IsValidDropSource(reciever))
        {
            m_DropZone.SetColor(COLOR_HIGHLIGHT);
            return true;
        }
    }
    return false;
}
```

In `OnDropReceived`, do the actual data move:

```c
override bool OnDropReceived(Widget w, int x, int y, Widget reciever)
{
    if (w != m_DropZone) return false;

    MyData srcData;
    reciever.GetUserData(srcData);
    if (!srcData) return false;

    MoveItem(srcData);
    return true;
}
```

## Cancelling mid-drag

```c
Widget cancelled = CancelWidgetDragging();
```

Stops the current drag. Returns the widget that was being dragged.

## `GetDragWidget`

```c
Widget src = GetDragWidget();
```

Returns the widget currently being dragged (null if no drag in progress).
Useful inside arbitrary callbacks (e.g. `Update`) to react to drag state.

## Reading drop source data

User data is the typical way to pass info from source to target:

```c
// On source:
widget.SetUserData(myDataObject);

// On target's OnDropReceived:
Class data;
reciever.GetUserData(data);
MyData md;
Class.CastTo(md, data);
```

`UserID` (integer) works too for lightweight cases:

```c
widget.SetUserID(itemID);
// On target:
int id = reciever.GetUserID();
```

## Visual feedback during drag

The engine doesn't auto-render a "drag image" — you have to do it
yourself. Common pattern: in `OnDrag`, hide the source widget and create
a follow-cursor preview; in `OnDrop`, destroy the preview and re-show or
re-create the source.

The inventory does this — see
`scripts/5_mission/gui/inventory/inventory.c` for the reference impl
(complex but illustrative).

## DayZ's drag conventions

- Inventory items are draggable.
- Hotbar slots are drop targets.
- Most COT widgets are not draggable — only the main window root
  (`draggable 1` in `windowbase.layout`) for window repositioning.

## Pitfalls

### `OnDrag` not firing

The widget needs `DRAGGABLE` flag AND no `IGNOREPOINTER` flag. Children
with `IGNOREPOINTER` pass clicks through, so the drag starts on the
parent.

### Drop firing on every widget under the cursor

`OnDraggingOver` fires for every widget the cursor touches during the
drag. If you have nested widgets, multiple `OnDraggingOver` calls happen
per frame. Filter by widget identity.

### `OnDrop` vs `OnDropReceived`

`OnDrop` is on the **source** — the dragged widget. `OnDropReceived` is
on the **target**. People mix these up; the wrong one will run the wrong
logic against the wrong widget.

## Engine truth

> Defined at lines 676–680 of `scripts/1_core/proto/enwidgets.c`. The
> drag system itself is implemented in C++ — script just receives the
> events and returns true/false to influence behaviour.
