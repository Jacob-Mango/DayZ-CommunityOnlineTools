# `RenderTargetWidget` and `RTTextureWidget`

3D rendering inside a 2D widget. `RenderTargetWidget` shows the view from
a virtual camera; `RTTextureWidget` exposes the result as a texture
usable in shaders.

## Classes

```c
class RenderTargetWidget extends Widget
{
    proto native void SetRefresh(int period, int offset);
    proto native void SetResolutionScale(float xscale, float yscale);
};

class RTTextureWidget extends Widget {};
```

## Linking a world camera

```c
SetWidgetWorld(entity, /* camera index */ 0, rttWidget);
```

`SetWidgetWorld` (global proto function) binds an `IEntity` and one of
its cameras to an `RTTextureWidget`. The widget then renders whatever
that camera sees.

For player previews, item rotation displays, mirrors, etc.

## Refresh control

```c
m_RT.SetRefresh(/* every N frames */ 1, /* initial offset */ 0);
```

- `period = 1` — re-render every frame (most expensive)
- `period = 2` — every other frame
- Higher values reduce GPU cost but make the view less responsive

Useful for off-screen / always-visible mini-maps that don't need 60fps.

## Resolution scale

```c
m_RT.SetResolutionScale(0.5, 0.5);   // render at 50% resolution
```

Reduces render-target resolution for performance — text and edges get
blurrier but rendering is faster. Useful for backgrounds and previews.

## Workflow: player preview

```c
// 1. Set up the camera in 3D space pointing at the player model
Camera cam = …;

// 2. Create an RTTextureWidget in your layout, name it "preview_rt"
//    + an ImageWidget that samples it

// 3. Bind:
RTTextureWidget rt = RTTextureWidget.Cast(layoutRoot.FindAnyWidget("preview_rt"));
SetWidgetWorld(playerEntity, 0, rt);
```

The vanilla character creator does this for the lobby preview.

## Use cases in DayZ

- Inventory item 3D preview (`ItemPreviewWidget`, similar class)
- Character creator (`PlayerPreviewWidget`)
- Map screen (rendering ground-level previews)
- Loading-screen backgrounds with subtle camera motion

> COT uses `ItemPreviewWidget` (not `RenderTargetWidget` directly) for
> the object spawner preview pane — see `JMObjectSpawnerForm`.

## Performance

Each render target is a separate GPU pass. Many small render targets are
much more expensive than a single large one. Use sparingly; reduce
`SetRefresh` period when the view doesn't need to update.

## Engine truth

> Defined at lines 236–245 of `scripts/1_core/proto/enwidgets.c`. The
> `SetGUIWidget(IEntity, int, RTTextureWidget)` global function at line
> 637 is the binding point for entity ↔ widget.
