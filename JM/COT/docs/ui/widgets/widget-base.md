# `Widget` — the base class

Every widget descends from `Widget`. The base class defines what every
widget can do: name, position, size, colour, hierarchy, visibility, input
flags, and the script-handler binding.

## Class declaration

```c
class Widget: Managed
{
    proto void ~Widget();
    proto private void Widget();
    // … methods listed below
};
```

`Widget` is **`Managed`** — script keeps a reference, engine controls the
lifetime. You cannot `new Widget()` from script; the engine constructs
widgets and hands you references.

## Identity

```c
proto native owned string GetName();
proto native void SetName(string name);
proto native owned string GetTypeName();   // e.g. "PanelWidget"
proto native WidgetType GetTypeID();        // e.g. PanelWidgetTypeID
proto native bool IsControlClass();         // true for input widgets
proto native owned string GetStyleName();   // the style name from layout
```

Names come from the layout file's first identifier after the widget class.
Two siblings can have the same name — `FindAnyWidget` returns the first hit.

## Hierarchy

```c
proto native Widget GetParent();
proto native Widget GetChildren();    // first child
proto native Widget GetSibling();     // next sibling
proto native void   AddChild(Widget child, bool immedUpdate = true);
proto native void   RemoveChild(Widget child);
proto native void   Unlink();          // destroys widget + all children
```

Iterate children with the sibling chain:

```c
Widget child = parent.GetChildren();
while ( child )
{
    Widget next = child.GetSibling();
    // … do something with child
    child = next;
}
```

## Find

```c
proto native Widget FindWidget(string pathname);     // by path "row.col.label"
proto native Widget FindAnyWidget(string pathname);  // by name (depth-first)
proto native Widget FindAnyWidgetById(int user_id);  // by SetUserID
```

`FindAnyWidget` searches descendants only, not the widget itself.

## Position and size

```c
proto native void SetPos(float x, float y, bool immedUpdate = true);
proto native void SetSize(float w, float h, bool immedUpdate = true);
proto native void SetScreenPos(float x, float y, bool immedUpdate = true);
proto native void SetScreenSize(float w, float h, bool immedUpdate = true);
proto void GetPos(out float x, out float y);
proto void GetSize(out float width, out float height);
proto void GetScreenPos(out float x, out float y);
proto void GetScreenSize(out float width, out float height);
```

`SetPos` / `SetSize` use the current `*EXACTPOS` / `*EXACTSIZE` flag
interpretation. `SetScreenPos` / `SetScreenSize` are always in monitor
pixels. See [[../coordinate-system]].

## Colour and alpha

```c
proto native void SetColor(int color);   // 0xAARRGGBB
proto native int  GetColor();
proto native void SetAlpha(float alpha); // 0..1
proto native float GetAlpha();
```

`SetColor` includes alpha in the high byte. `SetAlpha` overrides just the
alpha component. Use `ARGB(a, r, g, b)` helper to build colours.

## Rotation

```c
proto native void SetRotation(float roll, float pitch, float yaw, bool immedUpdate = true);
proto native vector GetRotation();   // returns (roll, pitch, yaw)
```

For 2D rotation use `SetRotation(0, 0, angle)`. COT uses this for the
refresh-icon spin animation in `UIActionImageButton.Update`.

## Transform

```c
proto native void SetTransform(vector mat[4], bool immedUpdate = true);
```

Direct matrix transform. Rarely needed; rotation + position usually
suffice.

## Visibility & flags

```c
proto native void Show(bool show, bool immedUpdate = true);   // toggle VISIBLE
proto native void Enable(bool enable);                         // toggle DISABLED
proto native bool IsVisible();
proto native bool IsVisibleHierarchy();   // visible & all ancestors visible

proto native int  GetFlags();
proto native int  SetFlags(int flags, bool immedUpdate = true);   // ADDS bits
proto native int  ClearFlags(int flags, bool immedUpdate = true); // CLEARS bits
```

See [[../widget-flags]] for the full flag list.

## Sort (z-order within parent)

```c
proto native int  GetSort();
proto native void SetSort(int sort, bool immedUpdate = true);
```

Higher sort = drawn on top of siblings. The engine respects sibling order
when sort values are equal. Used to bring tooltips and dropdowns above
other widgets in the **same parent only** — `SetSort` cannot raise above
widgets in a different parent subtree.

## User data

```c
proto void GetUserData(out Class data);   // bound by SetUserData
proto native void SetUserData(Class data);
proto native int  GetUserID();
proto native void SetUserID(int id);
```

`UserData` is arbitrary `Class`-derived data attached to a widget;
`UserID` is an int. COT uses `SetUserData` to associate row data with row
widgets (see `UIActionData` in `JM/COT/Scripts/4_World/.../UIActionData.c`).

## Script class binding

```c
proto void GetScript(out Class data);
```

If the widget's layout had `scriptclass "MyClass"`, this returns the
constructed `MyClass` instance. Cast to your expected type:

```c
JMObjectSpawnerForm form;
widget.GetScript(form);
```

## Event handler

```c
proto native void SetHandler(ScriptedWidgetEventHandler eventHandler);
```

Attaches a handler that receives mouse / keyboard / focus / drag callbacks
for this widget. See [[../events/event-handler]] for the full callback list.

## Lifecycle

```c
proto native volatile void Update();
proto native void Unlink();
```

`Update()` is called by the engine each frame if the script class overrides
it. `Unlink()` destroys the widget and all children.

## Translation

```c
proto static string TranslateString(string stringId);
```

Resolves `#STR_*` localisation keys. `text "#STR_KEY"` in a layout is
auto-translated; calling `SetText(string)` from script is not — wrap it:

```c
m_Label.SetText( Widget.TranslateString(stringId) );
```

## Global rendering knobs (static)

```c
proto static void SetLV(float lv);              // -15..0, dims widgets
proto static void SetTextLV(float lv);          // -15..0, dims text
proto static void SetObjectLighting(float l);   // 0..1, RenderTargetWidget brightness
```

Used by the engine for the night-vision / blackout effects. Rarely touched
by mods.

## Engine truth

> The `Widget` class is declared at line 107 of
> `scripts/1_core/proto/enwidgets.c`. Every other widget class is a
> subclass that adds type-specific methods on top of this base.
