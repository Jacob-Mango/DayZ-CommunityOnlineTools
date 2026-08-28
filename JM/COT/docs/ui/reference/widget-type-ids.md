# Widget Type IDs

Complete list of `WidgetType` constants used with
`WorkspaceWidget.CreateWidget(type, …)`.

From `scripts/1_core/proto/enwidgets.c` lines 7–53:

| Constant | Widget class | Description |
|---|---|---|
| `TextWidgetTypeID` | `TextWidget` | Single-line text |
| `MultilineTextWidgetTypeID` | `MultilineTextWidget` | Multi-line text |
| `MultilineEditBoxWidgetTypeID` | `MultilineEditBoxWidget` | Multi-line edit box |
| `RichTextWidgetTypeID` | `RichTextWidget` | Multi-line text with images in text |
| `RenderTargetWidgetTypeID` | `RenderTargetWidget` | Render target for a `BaseWorld` |
| `ImageWidgetTypeID` | `ImageWidget` | Single picture or multi-picture set |
| `ConsoleWidgetTypeID` | (no script binding) | Console output widget |
| `VideoWidgetTypeID` | `VideoWidget` | Video player |
| `RTTextureWidgetTypeID` | `RTTextureWidget` | Texture for child widgets to sample |
| `FrameWidgetTypeID` | `FrameWidget` | Dummy hierarchy node / clipper |
| `EmbededWidgetTypeID` | `EmbededWidget` | Embeds another layout file |
| `ButtonWidgetTypeID` | `ButtonWidget` | Clickable button |
| `CheckBoxWidgetTypeID` | `CheckBoxWidget` | Boolean toggle |
| `WindowWidgetTypeID` | (no script binding) | Window widget |
| `ComboBoxWidgetTypeID` | `XComboBoxWidget` | Drop-down combo |
| `SimpleProgressBarWidgetTypeID` | `SimpleProgressBarWidget` | Simple progress bar |
| `ProgressBarWidgetTypeID` | `ProgressBarWidget` | Styled progress bar |
| `SliderWidgetTypeID` | `SliderWidget` | Min/max value picker |
| `BaseListboxWidgetTypeID` | `BaseListboxWidget` | List of items (base class) |
| `TextListboxWidgetTypeID` | `TextListboxWidget` | List of text rows |
| `GenericListboxWidgetTypeID` | (no direct binding) | Generic list — engine-internal |
| `EditBoxWidgetTypeID` | `EditBoxWidget` | Single-line text input |
| `PasswordEditBoxWidgetTypeID` | `PasswordEditBoxWidget` | Masked input |
| `WorkspaceWidgetTypeID` | `WorkspaceWidget` | Root container — created by engine, not script |
| `GridSpacerWidgetTypeID` | `GridSpacerWidget` | Grid auto-layout |
| `WrapSpacerWidgetTypeID` | `WrapSpacerWidget` | Wrap auto-layout |
| `ScrollWidgetTypeID` | `ScrollWidget` | Scrollable viewport |

## Layout class names

In `.layout` files, the **WidgetClass** name is the script class name
plus the `Class` suffix:

| Type ID | Layout class name |
|---|---|
| `TextWidgetTypeID` | `TextWidgetClass` |
| `PanelWidgetTypeID` (alias for plain `Widget`) | `PanelWidgetClass` |
| `FrameWidgetTypeID` | `FrameWidgetClass` |
| `ButtonWidgetTypeID` | `ButtonWidgetClass` |
| `ImageWidgetTypeID` | `ImageWidgetClass` |
| `GridSpacerWidgetTypeID` | `GridSpacerWidgetClass` |
| `WrapSpacerWidgetTypeID` | `WrapSpacerWidgetClass` |
| `ScrollWidgetTypeID` | `ScrollWidgetClass` |
| `TextListboxWidgetTypeID` | `TextListboxWidgetClass` |
| `EditBoxWidgetTypeID` | `EditBoxWidgetClass` |
| `PasswordEditBoxWidgetTypeID` | `PasswordEditBoxWidgetClass` |
| `SliderWidgetTypeID` | `SliderWidgetClass` |
| `CheckBoxWidgetTypeID` | `CheckBoxWidgetClass` |
| `VideoWidgetTypeID` | `VideoWidgetClass` |
| `RenderTargetWidgetTypeID` | `RenderTargetWidgetClass` |
| `RTTextureWidgetTypeID` | `RTTextureWidgetClass` |
| `EmbededWidgetTypeID` | `EmbededWidgetClass` |

## Note on `PanelWidget`

`PanelWidget` doesn't appear in the `WidgetType` enum because it's
internally aliased to `WidgetTypeID` (the generic widget type). In
layouts, `PanelWidgetClass` works the same as a plain `Widget` with a
visible colour.

## Engine truth

> The enum and the `WidgetType` typedef are at lines 7–53 of
> `scripts/1_core/proto/enwidgets.c`. The enum entries are only visible
> with `#ifdef DOXYGEN` — the actual `WidgetType` is an opaque `TypeID`
> at runtime.
