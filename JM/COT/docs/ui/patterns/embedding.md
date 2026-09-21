# Reusable Layouts — Embedding

To re-use a widget subtree across multiple layouts, save it as its own
`.layout` file and reference it from a host layout. Two mechanisms:

1. **`EmbededWidget`** in a parent layout — declarative
2. **`g_Game.GetWorkspace().CreateWidgets(...)` with a parent** in script — programmatic

## `EmbededWidget`

Declared in the engine as `EmbededWidgetTypeID`. Used in layouts to
inline another layout file as a single child.

```
PanelWidgetClass form_root {
 size 1 1
 {
  EmbededWidgetClass shared_toolbar {
   layout "MyMod/GUI/layouts/shared_toolbar.layout"
  }
  // … other content
 }
}
```

At load time, the engine inlines the contents of
`shared_toolbar.layout` as if you'd typed them directly in place.

## Programmatic embedding

Equivalent to the above, but done from script:

```c
Widget toolbar = g_Game.GetWorkspace().CreateWidgets(
    "MyMod/GUI/layouts/shared_toolbar.layout",
    /* parent */ m_HeaderArea
);
```

The new toolbar's root becomes a child of `m_HeaderArea`.

## When to embed

- A header / footer that appears in multiple forms
- A button row that's reused (`UIActionButton.layout` is this kind of thing)
- A "card" component for list rows

Every COT `UIAction*` widget works this way: each one is a small layout
file, and they get embedded into form layouts at construction time via
`UIActionManager.CreateButton`, `CreateText`, etc.

## How COT uses it

`UIActionManager.CreateButton(parent, "label", ...)` does:

```c
Widget widget = g_Game.GetWorkspace().CreateWidgets(
    "JM/COT/GUI/layouts/uiactions/UIActionButton.layout",
    parent
);
// configure script class, set label, etc.
```

The `UIActionButton.layout` is a small reusable widget subtree —
button background, icon image, text label — bundled with a
`scriptclass "UIActionButton"`. Embedding it gives you a fully wired
button without writing 30 lines of layout markup per call site.

## Layout discovery

You can find every COT reusable layout in:

```
JM/COT/GUI/layouts/uiactions/
```

- `UIActionButton.layout`
- `UIActionCheckbox.layout`
- `UIActionImageButton.layout`
- `UIActionSlider.layout`
- `UIActionText.layout`
- … etc.

Each one is a small standalone tree designed to be embedded.

## Limitations

### No per-instance parameters

The embed mechanism inlines a fixed tree. To customise per instance
(label text, colour, callback), do it in script after construction:

```c
UIActionButton btn = UIActionManager.CreateButton(parent, "OK", this, "OnOK");
btn.SetColor(COLOR_GREEN);
```

`.layout` files can't accept arguments.

### Embedded layout's root size

The embedded layout's root widget keeps its own `size` declaration. If
you embed a layout with `size 400 200`, the child becomes 400×200
regardless of the host's preferences. Override in script with
`SetSize`/`SetFixedSize`.

### Embedding chains

You can embed layouts that themselves embed other layouts. Just don't
recurse — embedding `A` from `A` is an infinite loop and crashes the
engine at load time.

## Pitfalls

### Forgetting `scriptclass` doesn't transfer

The host layout's `scriptclass` is for the host's root widget; the
embedded widget keeps its own `scriptclass` (declared in its own layout
file).

### Performance

Each embed = a separate layout-file load. Lots of dynamic embeds in a
hot path (e.g. one per row in a 1000-row list) can be measurably slow.
For massive lists, generate widgets programmatically with `CreateWidget`
instead.

## Engine truth

> `EmbededWidget` is one of the type IDs in the `WidgetType` enum
> (line 33 of `scripts/1_core/proto/enwidgets.c`). The C++ layout
> loader handles inlining; nothing extra in script.
