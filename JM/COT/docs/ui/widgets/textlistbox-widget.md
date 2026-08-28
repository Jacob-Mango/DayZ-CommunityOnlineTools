# `TextListboxWidget`

A multi-row list of text items. Each row has columns; each cell has its
own text and user data. Supports selection, removal, and visibility
scrolling.

## Class hierarchy

```c
class BaseListboxWidget extends UIWidget
{
    proto native void ClearItems();
    proto native int  GetNumItems();
    proto native void SelectRow(int row);
    proto native int  GetSelectedRow();
    proto native void RemoveRow(int row);
    proto native void EnsureVisible(int row);
};

class SimpleListboxWidget extends BaseListboxWidget {}

class TextListboxWidget extends SimpleListboxWidget
{
    proto native int  AddItem(string text, Class userData, int column, int row = -1);
    proto native void SetItem(int position, string text, Class userData, int column);
    proto bool        GetItemText(int row, int column, out string text);
    proto void        GetItemData(int row, int column, out Class data);
    proto native void SetItemColor(int row, int column, int color);
};
```

## Layout

```
TextListboxWidgetClass class_list {
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 style Editor
 lines 20             // visible rows; scrollbar appears when exceeded
 font "gui/fonts/MetronLight14"
}
```

The `lines N` property gates the height; if more rows are added, the
listbox scrolls.

## Adding items

```c
m_List.ClearItems();
m_List.AddItem("First row",  null, /* column */ 0);
m_List.AddItem("Second row", null, 0);
m_List.AddItem("Third row",  null, 0);
```

`AddItem` returns the row index. Use `row = -1` (default) to append; any
non-negative value inserts at that row.

For multi-column lists, add the same row with different columns:

```c
int row = m_List.AddItem("Name", null, 0);
m_List.SetItem(row, "Value", null, 1);
m_List.SetItem(row, "Type",  null, 2);
```

## Reading items

```c
string text;
m_List.GetItemText(/* row */ 5, /* column */ 0, text);

Class data;
m_List.GetItemData(5, 0, data);
// cast data to your type:
MyData md;
Class.CastTo(md, data);
```

## Selection

```c
int sel = m_List.GetSelectedRow();   // -1 if nothing selected
m_List.SelectRow(3);                  // programmatically select row 3
m_List.EnsureVisible(3);              // scroll so row 3 is in view
```

Listening for selection changes happens via the parent's handler:

```c
override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
{
    if (w == m_List)
    {
        UpdateDetailsPanel(row);
        return true;
    }
    return false;
}
```

The `OnItemSelected` callback fires on both click and keyboard navigation.

## Per-cell colour

```c
m_List.SetItemColor(/* row */ 5, /* column */ 0, COLOR_RED);
```

Use for status colours, dirty-marking edits, etc.

## Common pitfalls

### Forgetting `GetSelectedRow` returns -1

If nothing is selected, accessing index `-1` crashes the engine. Always
guard:

```c
int row = m_List.GetSelectedRow();
if (row < 0)
    return;
```

### `Class` user data

The `Class` user data must be a `Managed` subclass (or null). Passing
non-managed pointers leaks memory.

### Listbox doesn't auto-update on layout changes

If you change `lines` at runtime or the parent resizes, you may need to
re-add items or call `EnsureVisible` to redraw the scrollbar correctly.

## Engine truth

> Defined at lines 425–458 of `scripts/1_core/proto/enwidgets.c`. The
> base `BaseListboxWidget` class is shared by `SimpleListboxWidget` and
> `TextListboxWidget`; `GenericListboxWidget` is a third variant
> (vanilla declares the type ID but doesn't expose it to script).
