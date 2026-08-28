# Z-Order and `SetSort`

Z-order — which widget paints on top of which — is controlled by the
**sibling order** within each parent and the **sort value** on each
widget.

## How z-order is determined

For a group of sibling widgets sharing the same parent:

1. Sort by `SetSort(N)` value, **higher draws on top**.
2. Among siblings with equal sort, draw in **declaration order** (later
   in the layout = on top).

For widgets in **different** subtrees, z-order is determined by the
order of their ancestors. There is **no** global z-buffer.

## The big gotcha: `SetSort` is sibling-scoped

```c
widget.SetSort(9999);
```

This raises `widget` above its siblings — but cannot raise it above
widgets in a different parent's subtree.

```
Workspace
├── Form A
│   ├── Tooltip          ← SetSort(9999) on this only beats Form A's other children
│   └── Button
└── Form B
    └── Other content    ← Form B's content can still draw on top if Form B has higher sort than Form A
```

To bring a tooltip / dropdown above EVERYTHING, you must:

1. Re-parent it to the workspace root, OR
2. Attach it to the highest common ancestor of "things it should beat",
   then `SetSort` high among those siblings.

## The COT dropdown / tooltip pattern

`UIActionTooltip` and `UIActionDropdown` create their popup widget as a
**child of the workspace root**, not as a child of the source widget.

```c
m_ListPanel = g_Game.GetWorkspace().CreateWidgets("…/dropdown_list.layout", NULL);
m_ListPanel.SetSort(9999);
```

Because the popup is now a workspace-level sibling of every form, it
draws above everything within forms.

## When to use SetSort

| Situation | SetSort needed? |
|---|---|
| Showing a button on top of a panel in the same form | No — declaration order handles it |
| Highlight overlay over a row | No — declaration order |
| Tooltip above any UI in any form | Yes, and parent it to workspace root |
| Dropdown list above a list box | Yes, and parent to root or to layoutRoot |
| Active tab visually overlapping inactive tabs | Yes, between the tabs (`SetSort` higher) |

## SetSort values

The engine treats sort as a plain int. There's no defined "max" value;
9999 is just a convention for "should beat everything in this parent".

```c
widget.SetSort(0);     // default
widget.SetSort(100);   // above siblings with sort < 100
widget.SetSort(9999);  // above siblings with sort < 9999
widget.SetSort(-1);    // below default-sort siblings
```

Negative values work and put the widget at the back.

## `GetSort`

```c
int sort = widget.GetSort();
```

Useful for "set my sort to one above my sibling's":

```c
widget.SetSort( otherSibling.GetSort() + 1 );
```

## The "raise the form" trick

To bring a whole form to the front when clicked:

```c
override bool OnMouseButtonDown(Widget w, int x, int y, int button)
{
    // bump form's sort above all sibling forms
    int maxSort = 0;
    Widget root = g_Game.GetWorkspace();
    Widget sibling = root.GetChildren();
    while ( sibling )
    {
        maxSort = Math.Max(maxSort, sibling.GetSort());
        sibling = sibling.GetSibling();
    }
    m_FormRoot.SetSort(maxSort + 1);
    return false;
}
```

This is how the COT window system implements "click to bring to front"
(see `JMWindowManager`).

## Pitfalls

### Setting sort on the wrong widget

If your dropdown popup is a **descendant** of the source widget, setting
sort on it only beats other descendants — not unrelated forms. Re-parent
the popup higher.

### Sort survives across `Show(false)` / `Show(true)`

Hidden widgets retain their sort. When you `Show(true)`, they appear
back at their stored sort. No need to re-call `SetSort`.

### Children inherit parent's z-position

A child widget always draws on top of its parent's background but
behind/in-front of its siblings according to sort. Cross-parent z-order
is the parent's z-order, not the child's.

## Engine truth

> `SetSort` declared at lines 129–131 of
> `scripts/1_core/proto/enwidgets.c`:
> ```
> proto native int  GetSort();
> proto native void SetSort(int sort, bool immedUpdate = true);
> ```
