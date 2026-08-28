# Widget Lifecycle & Update Loop

## Construction

A widget exists from the moment the engine creates it until `Unlink()` is
called on it or on an ancestor. There are three creation paths:

### 1. From a `.layout` file

```c
Widget root = g_Game.GetWorkspace().CreateWidgets(
    "JM/COT/GUI/layouts/my_form.layout",
    parentWidget  // optional
);
```

Every widget node in the layout is instantiated. If any node has a
`scriptclass`, that script instance is constructed too, and the engine
calls its `OnInit()` virtual method **after** the layout root and all
children exist.

### 2. Programmatically with `CreateWidget`

```c
Widget w = g_Game.GetWorkspace().CreateWidget(
    WidgetType.PanelWidgetTypeID,
    /* left */ 0, /* top */ 0,
    /* width */ 100, /* height */ 30,
    /* flags */ WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS
              | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE,
    /* color */ 0xFF000000,
    /* sort */ 0,
    /* parent */ parentWidget
);
```

Rarely used in mods — the layout-file approach is more maintainable.

### 3. As a child of another widget

```c
parent.AddChild(otherWidget);
```

Used when re-parenting existing widgets. Rare.

## `OnInit` / script class construction

When a widget has `scriptclass "MyClass"`:

1. Engine creates the C++ widget.
2. Engine creates an instance of `MyClass` and calls its constructor.
3. Engine binds the script instance to the widget — `widget.GetScript(out s)`
   now returns the instance.
4. Engine calls `OnInit()` on the script. The widget's `layoutRoot` is
   already set at this point.

```c
class MyForm: UIActionBase  // or any script class
{
    override void OnInit()
    {
        super.OnInit();

        // safe to FindAnyWidget on layoutRoot here:
        Widget btn = layoutRoot.FindAnyWidget("my_button");
    }
}
```

> **Gotcha:** Calling `SetChecked(true)` (or any state-changing method
> that fires events) from `OnInit` will fire **before the caller has a
> chance to wire up handlers**. Use silent variants like
> `SetCheckedSilent` for initial state.

## The update tick

Every frame, the engine calls `Update()` on widgets whose script class
overrides it. This is where you tick animations, polling, etc.

```c
class MyAnimatedThing: UIActionBase
{
    override void Update(float timeSlice)
    {
        super.Update(timeSlice);
        // timeSlice is seconds since last frame (e.g. 0.016 at 60 fps)
    }
}
```

**`timeSlice` is in seconds.** Don't multiply by 1000 or anything — use it
directly with `delta = speed * timeSlice`.

### Update is opt-in

The widget needs an `OnUpdate` overload (or its script class's `Update`)
for the engine to tick it. Pure layout widgets (Panel, Frame, Text) don't
tick — that's fine, they don't need to.

## Destruction

```c
widget.Unlink();
```

This:
- Removes the widget from its parent's child list.
- Recursively unlinks every descendant.
- Triggers GC of the script instance (no `OnDestroy` callback in vanilla — use
  the destructor `void ~MyClass()`).

```c
class MyForm: ScriptedWidgetEventHandler
{
    void ~MyForm()
    {
        // run any cleanup here
    }
}
```

> **Don't `Unlink` a widget twice.** The second call crashes the engine.
> When rebuilding dynamic UI sections, use the pattern:
> ```c
> if (m_DynamicContent)
>     delete m_DynamicContent;       // calls ~Widget which Unlinks
> m_DynamicContent = UIActionManager.CreateGridSpacer(parent, 1, 1);
> ```

## Showing and hiding

`widget.Show(bool)` toggles `WidgetFlags.VISIBLE`. Hidden widgets:
- Don't render
- Don't receive input
- Still exist in the tree
- Still consume layout space in their parent (`Show(false)` ≠ remove)

If you want to remove from layout, `Unlink` or move under a hidden parent.

## Enable / disable

`widget.Enable(bool)` toggles `WidgetFlags.DISABLED`. Disabled widgets:
- Render greyed-out (depending on style)
- Don't receive input

COT's `UIActionBase` adds a custom `Disable()` that also shows an
`action_wrapper_disable` overlay panel for a stronger visual effect.

## Lifecycle integration with COT forms

`JMFormBase` (a `ScriptedWidgetEventHandler`) defines lifecycle hooks that
the COT window system calls:

| Hook | Called when |
|---|---|
| `SetModule(module)` | Right after construction — store the typed module ref |
| `OnInit()` | Once, after layout has been built — build static UI |
| `OnShow()` | Each time the form becomes visible — refresh data |
| `OnHide()` | Each time the form is hidden — stop timers |
| `OnResize(w, h)` | Window was resized — update scrollers |
| `OnSettingsUpdated()` | Module data changed — rebuild dynamic content |
| `OnClientPermissionsUpdated()` | Permissions changed — update gating |

These are COT conventions, not engine events. See
`styling/style-guide.md` for the contract.

## Engine truth

> Lifecycle primitives in `scripts/1_core/proto/enwidgets.c`:
> ```
> proto void Widget();          // private — only engine constructs
> proto void ~Widget();         // private — destructors run via Unlink
> proto native void AddChild(Widget child, bool immedUpdate = true);
> proto native void RemoveChild(Widget child);
> proto native void Unlink();   // destroys widget and all children
> proto native volatile void Update();
> proto void GetScript(out Class data);
> ```
