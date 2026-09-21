# Modal Windows

A modal widget blocks input to everything except itself and its
descendants. Used for confirmation dialogs, alert popups, and any
"user must respond" interaction.

## API

```c
proto native void SetModal(Widget w);
```

A single global modal at a time. Calling `SetModal(NULL)` clears.

## Usage

```c
// Show a modal dialog
Widget dlg = g_Game.GetWorkspace().CreateWidgets("MyMod/GUI/dialog.layout");
SetModal(dlg);

// Later, dismiss it
SetModal(NULL);
dlg.Unlink();
```

## How blocking works

While a widget is modal, the engine:
- Routes mouse / keyboard events **only** to the modal widget and its
  descendants.
- Ignores hovers and clicks on widgets outside the modal subtree.
- Still renders everything (the rest of the UI just becomes unresponsive,
  not invisible).

This is why most modal dialogs cover the screen with a darkened
background panel: visually communicate "the rest is blocked".

## COT's JMConfirmation system

`JMConfirmation` (in `JM/COT/Scripts/4_World/.../JMConfirmation.c`) wraps
`SetModal` with a standard popup that has:
- Title text
- Body message
- Up to two buttons (OK/Cancel, Yes/No)
- Optional edit-box for text input

Usage:

```c
CreateConfirmation_Two(
    JMConfirmationType.INFO,
    "Delete vehicle?",
    "This cannot be undone.",
    "#STR_COT_GENERIC_CANCEL", "",
    "#STR_COT_GENERIC_YES", "OnConfirmDelete"
);

void OnConfirmDelete(JMConfirmation confirmation)
{
    // user clicked Yes
    Exec_DeleteVehicle();
}
```

The callback is dispatched via the JMScriptInvokers system.

## Layered modals

You can stack modals — calling `SetModal(dialog2)` while `dialog1` is
modal shifts focus to `dialog2`. When `dialog2` closes, you must
manually `SetModal(dialog1)` to restore the previous level.

Most apps don't need this — single-level modal is the common case.

## Pitfalls

### Forgetting to dismiss

If you `SetModal(dlg)` and the user clicks a button that doesn't call
`SetModal(NULL)`, the UI stays blocked. Always have a cleanup path.

### Modal widget gets garbage-collected

If the modal widget is local to a method and goes out of scope (no
script reference held), the script GC may collect it even while it's
modal. Keep a reference (`m_CurrentModal = dlg;`).

### Modal blocks the host form's events

A modal that's a child of a form will receive events. A modal that's a
sibling of (or above) the form will block events to the form — but if
the form is the modal's ancestor, the form still receives events too.

For "fully blocking" modals, parent them to the workspace root, not to a
form.

## Engine truth

> Declared at line 700 of `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native void SetModal(Widget w);
> ```
> One function; no state introspection (no `GetModal`).
