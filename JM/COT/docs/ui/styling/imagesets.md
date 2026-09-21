# Image Sets

An `.imageset` is a packed atlas of named image slices, similar to a CSS
sprite sheet. Multiple icons share one texture file and the imageset
maps slice names → atlas coordinates.

## Imageset file format

```xml
<ImageSet Texture="dayz_gui.paa">
  <Image Name="icon_warning" X="0" Y="0" Width="32" Height="32" />
  <Image Name="icon_check"   X="32" Y="0" Width="32" Height="32" />
  <Image Name="icon_x"       X="64" Y="0" Width="32" Height="32" />
  ...
</ImageSet>
```

- `Texture` — the underlying `.paa` atlas file
- `<Image>` entries map slice names to pixel rectangles within the atlas

## Loading an imageset

```c
LoadWidgetImageSet("gui/imagesets/dayz_gui.imageset");
```

Imagesets are global once loaded — accessible from any widget that
references them.

## Referencing a slice

Use the `set:name image:name` syntax:

```c
m_Icon.LoadImageFile(0, "set:dayz_gui image:icon_warning");
m_Icon.SetImage(0);
```

In a layout:

```
image "set:dayz_gui image:icon_warning"
```

The engine resolves the slice and uses its UV rectangle.

## Common imagesets in DayZ

| Imageset | Contents |
|---|---|
| `dayz_gui` | The main UI imageset (status icons, button frames, etc.) |
| `ccgui_enforce` | Enforce-engine UI widgets (default styles use this) |
| `dayz_inventory` | Inventory-specific icons |
| `hud_inventory` | HUD overlay icons |

Imagesets are loaded by the engine on startup; you don't need to load
them yourself for vanilla ones.

## Custom imagesets

Bundle your own atlas:

```
MyMod/
  gui/
    imagesets/
      mymod_gui.imageset
      mymod_gui.paa
```

Load on startup:

```c
override void OnGameplayDataHandlerLoad()
{
    LoadWidgetImageSet("MyMod/gui/imagesets/mymod_gui.imageset");
}
```

Reference with `set:mymod_gui image:my_icon`.

## When NOT to use an imageset

For one-off images (large screenshots, full-frame backgrounds), use a
direct path:

```c
m_Image.LoadImageFile(0, "MyMod/gui/textures/background.paa");
```

Imagesets are best for collections of small icons used together (toolbar
icons, status indicators).

## COT doesn't use imagesets

COT ships individual `.paa` files in `JM/COT/GUI/textures/icons/`. Each
icon is its own file referenced by path:

```c
m_Icon.LoadImageFile(0, JMConstants.ICON_TRASH_CAN);
```

This avoids needing to maintain an imageset XML alongside the icons.
The tradeoff is slightly less efficient texture loading at startup.

## Performance

The engine batches draw calls per-texture. Using multiple icons from a
single imageset is more efficient than the same icons from separate
files — fewer texture binds per frame.

For a HUD with many icons, prefer imagesets. For UI built from layout
files, the difference is negligible.

## Engine truth

> Imageset loading proto:
> ```
> proto native bool LoadWidgetImageSet(string filename);
> ```
> at line 692 of `scripts/1_core/proto/enwidgets.c`. The imageset XML
> format is engine-defined; mirrors what's in `gui/imagesets/*.imageset`.
