# DayZ Vanilla UI Reference

A practical reference to the DayZ Enforce Script widget system, written for COT contributors.
All facts here are sourced from the engine's script bindings
(`scripts/1_core/proto/enwidgets.c`) and from real layout files in the game.

## How to read these docs

Each file is **standalone** — you can land on `widgets/text-widget.md` from a search
and have everything you need to use that widget without reading the rest.
Cross-references use `[[double brackets]]` to mark related topics.

## Index

### Foundations
- [Widget System Overview](overview.md) — how widgets, layouts, scripts, and the workspace fit together
- [Layout File Format](layout-file-format.md) — the `.layout` text format, properties, nesting
- [Coordinate System](coordinate-system.md) — position, size, exact-vs-relative, alignment refs
- [WidgetFlags](widget-flags.md) — the bitmask that controls rendering, input, and sizing behaviour
- [Lifecycle & Update Loop](lifecycle.md) — creation, `Update()`, destruction, the workspace tick

### Widget reference
- [Widget (base)](widgets/widget-base.md) — shared API for every widget
- [PanelWidget](widgets/panel-widget.md) — solid-coloured background, the most common container
- [FrameWidget](widgets/frame-widget.md) — invisible hierarchy node, clipping rectangle
- [TextWidget](widgets/text-widget.md) — single-line text, styling, outline/shadow
- [MultilineTextWidget](widgets/multiline-text-widget.md) — wrapped multi-line text
- [RichTextWidget](widgets/rich-text-widget.md) — text with embedded images and inline styling
- [ImageWidget](widgets/image-widget.md) — single-image render, masking, UV control
- [ButtonWidget](widgets/button-widget.md) — clickable widget with state
- [CheckBoxWidget](widgets/checkbox-widget.md) — boolean toggle (vanilla)
- [EditBoxWidget](widgets/editbox-widget.md) — single-line text input
- [PasswordEditBoxWidget](widgets/password-editbox-widget.md) — masked text input
- [MultilineEditBoxWidget](widgets/multiline-editbox-widget.md) — multi-line text input
- [SliderWidget](widgets/slider-widget.md) — min/max/step value picker
- [ProgressBarWidget](widgets/progressbar-widget.md) — visual fill bar
- [TextListboxWidget](widgets/textlistbox-widget.md) — row-and-column text list
- [SpacerWidget (base)](widgets/spacer-widget.md) — base for grid/wrap spacers
- [GridSpacerWidget](widgets/gridspacer-widget.md) — fixed rows × columns auto-layout
- [WrapSpacerWidget](widgets/wrapspacer-widget.md) — flow layout that wraps to new line
- [ScrollWidget](widgets/scroll-widget.md) — scrollable content area with H/V scrollbars
- [CanvasWidget](widgets/canvas-widget.md) — script-drawn lines for HUD overlays
- [VideoWidget](widgets/video-widget.md) — video playback with subtitles
- [RenderTargetWidget](widgets/rendertarget-widget.md) — embed a 3D camera in the UI
- [WorkspaceWidget](widgets/workspace-widget.md) — top-level container, factory for new widgets

### Event handling
- [ScriptedWidgetEventHandler](events/event-handler.md) — every callback the engine can fire
- [Mouse Events](events/mouse-events.md) — click, double-click, enter, leave, wheel, drag
- [Keyboard & Controller](events/keyboard-controller.md) — key down/up/press, ControlID, focus
- [Drag and Drop](events/drag-drop.md) — the full drop chain
- [Focus Management](events/focus.md) — `SetFocus`, `SetActiveWindow`, focusable widgets

### Styling
- [Styles System](styling/styles-system.md) — `.styles` files, named styles per widget
- [Fonts & Text Styling](styling/fonts-and-text.md) — SDF fonts, sizes, bold/italic, shadow, outline
- [Colors & Alpha](styling/colors-alpha.md) — ARGB format, `SetColor`, `SetAlpha`, the global LV controls
- [Image Sets](styling/imagesets.md) — `.imageset` packs, named slices, atlas usage
- [Textures: `.edds` & `.paa`](styling/textures-edds-and-paa.md) - converting an image to either format, when to use which
- [DayZ Default Styles](styling/dayz-default-styles.md) — `DayZDefaultPanel`, `Editor`, common style names

### Layout patterns
- [Containers Cheat Sheet](patterns/containers.md) — Panel vs Frame vs Grid vs Wrap vs Scroll
- [Flow Composition](patterns/flow-composition.md) — building rows, columns, grids without overflow
- [Sizing Rules](patterns/sizing-rules.md) — when to use fractional vs exact, mixing them in one parent
- [Alignment Anchors](patterns/alignment-anchors.md) — `left_ref` / `right_ref` / `center_ref` / `bottom_ref`
- [Z-Order & Sort](patterns/z-order.md) — `SetSort`, the sibling reordering trick
- [Clipping](patterns/clipping.md) — what clips children and what overflows
- [Reusable Layouts (Embedding)](patterns/embedding.md) — `EmbededWidget` and `CreateWidgets` with parent

### Workspace API
- [Creating Widgets at Runtime](workspace/runtime-creation.md) — `CreateWidget` vs `CreateWidgets`
- [Modal Windows](workspace/modal.md) — `SetModal`, blocking input
- [Cursor Control](workspace/cursor.md) — `SetCursorWidget`, `ShowCursorWidget`
- [Active Window & Focus](workspace/active-window.md) — controller/keyboard navigation roots

### Reference data
- [WidgetType IDs](reference/widget-type-ids.md) — every type the engine exposes
- [WidgetFlags Enum](reference/widget-flags-enum.md) — every flag bit and what it does
- [ControlID Enum](reference/control-id-enum.md) — controller buttons mapped to enum
- [Layout Property Index](reference/layout-property-index.md) — every property name found in `.layout` files
- [Common Pitfalls](reference/pitfalls.md) — the bugs everyone hits at least once

## Conventions used in these docs

- **Code references** use the `file:line` format and link relative to the repo root.
- **Layout snippets** are short — each one shows just enough to illustrate a single concept.
- **"Engine truth"** boxes mark behaviour verified against the engine bindings; **"Convention"**
  boxes mark project-level conventions that aren't enforced by the engine.

## Other COT UI docs

- `JM/COT/docs/ui/styling/style-guide.md` — naming and pattern conventions for COT forms specifically
- `JM/COT/Scripts/5_Mission/CommunityOnlineTools/gui/Actions/` — the COT widget wrappers (`UIAction*`)

---

*Last updated: 2026-06-21*
