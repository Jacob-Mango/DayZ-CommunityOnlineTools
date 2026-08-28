# The Styles System

DayZ widgets can reference **named styles** that bundle font, colour,
images, and per-state appearance. Styles live in `.styles` XML files and
are loaded by the engine at startup.

## File location

```
gui/looknfeel/dayzwidgets.styles
```

Mods can ship their own `.styles` files and register them with
`LoadWidgetStyles(filename)` at startup.

## XML structure

```xml
<WidgetStyles>
  <Widget Name="TextWidget">
    <Style Name="Normal" Font="gui/fonts/MetronBook" ImageSet="" Color="4294967295" />
    <Style Name="Bold"   Font="gui/fonts/MetronBook-Bold" ImageSet="" Color="4294967295" />
  </Widget>
  <Widget Name="ButtonWidget">
    <Style Name="DayZButton" Font="..." ImageSet="dayz_gui" Color="4294967295">
      <State Name="Normal">
        <Item Name="LeftTop" Image="" />
        <Item Name="Center" Image="ButtonDefault" />
        ...
      </State>
      <State Name="Focus">
        <Item Name="Center" Image="ButtonFocus" />
      </State>
      <State Name="Pressed">
        ...
      </State>
    </Style>
  </Widget>
</WidgetStyles>
```

Top level: `<WidgetStyles>` element.
- One `<Widget>` per widget type (e.g. `TextWidget`, `ButtonWidget`).
- Each widget has multiple `<Style>` entries, each with a name.
- Complex widgets (buttons, progress bars) have nested `<State>` elements
  for different visual states (Normal, Focus, Pressed, Disabled).
- States contain `<Item>` elements that map slice names to image-atlas
  references.

## Applying a style in a layout

```
ButtonWidgetClass my_btn {
 style DayZButton
 ...
}
```

The `style` property references the `Name` attribute of the `<Style>`
element. Style properties (font, colour, images) apply before layout
properties — so anything in the layout overrides the style.

## Common style names

From vanilla DayZ:

| Style | Widget | Use |
|---|---|---|
| `Normal` | TextWidget | Default body text font |
| `Bold` | TextWidget | Bold body |
| `Light` | TextWidget | Light-weight font |
| `DayZNormal` | MultilineTextWidget, MultilineEditBoxWidget | Default multi-line |
| `DayZBold` | MultilineTextWidget | Bold multi-line |
| `DayZDefaultPanel` | PanelWidget | Standard dark panel background |
| `DayZButton` | ButtonWidget | Standard rectangular button |
| `Editor` | CheckBoxWidget, TextListboxWidget | Editor-grade box visuals |
| `Default` | various | Whatever the widget type defaults to |
| `Empty` | various | No visuals, transparent |
| `rover_sim_colorable` | PanelWidget, etc. | Accepts SetColor tinting properly |

## State-based styling

For widgets with hover / press / disabled states (Button, CheckBox,
ProgressBar), the engine swaps the visible images based on the widget's
current state:

| State | When |
|---|---|
| `Normal` | Default |
| `Focus` | Widget has focus |
| `Pressed` | Mouse button down on widget |
| `Disabled` | `Enable(false)` called |

A complete button style might have 4 states each with 15+ image slices
(corners, edges, centre, with-icon variants). This is why writing custom
button styles from scratch is tedious — most mods reuse existing styles
and just override the `color` property.

## The 9-slice anatomy

For panel-like widgets, image slices are organised as a 9-slice grid:

```
LeftTop    Top    RightTop
Left       Center Right
LeftBottom Bottom RightBottom
```

The engine stretches `Top` / `Bottom` horizontally, `Left` / `Right`
vertically, and `Center` in both directions; the four corners are fixed.
This is how a single style scales gracefully to any widget size without
distorted corners.

For progress bars, the style adds a parallel `BarLeftTop` / `BarTop` / …
set used as the "filled" portion overlay.

## Limitations

- Styles are **static after engine load** — you can't modify a style at
  runtime. Adjust per-widget appearance via `SetColor`, `SetImage`, etc.
- Style inheritance is **per widget type** — you can't have a `Bold`
  text style derive from `Normal`. They're independent entries.
- No script API to enumerate styles. You discover them by reading the
  XML.

## Crash trap: per-widget-type registration is mandatory

DayZ resolves styles by **(widget class, style name)**. A `<Style>` is
registered under a specific `<Widget Name="...">` block:

```xml
<Widget Name="PanelWidget">
    <Style Name="MyStyle" ... > ... </Style>
</Widget>
```

If a layout writes `style MyStyle` on a `GridSpacerWidgetClass` but
`MyStyle` is only defined under `PanelWidget`, **the engine cannot find
the style for that widget type**. Behaviour:

- The widget loads without error (no script-side complaint).
- The widget is hidden / not yet rendered → still no crash.
- The first frame the widget becomes visible, the engine renderer
  dereferences a null state pointer → **CTD (engine-level access
  violation)** with no script trace.

**You must redefine the style under every widget type that uses it.**
COT learned this the hard way — see `JM/COT/GUI/styles/cot.styles` for
how `COTSurface` is registered under `PanelWidget`, `GridSpacerWidget`,
`WrapSpacerWidget`, and `ScrollWidget` so any of them can use it.

> **FrameWidget caveat:** `FrameWidget` does NOT support styles in
> vanilla DayZ. Layouts that put `style ...` on a `FrameWidgetClass`
> are silently broken — the style is ignored, and if there's no
> backing color/visual, the frame is invisible. Use `PanelWidgetClass`
> when you need styled visuals.

## Minimum viable solid-color style

Every `<Style>` for a panel-like widget needs at least a `<State Name="Normal">`
block containing the 9-slice image items, even if you just want a flat
colored rectangle. The vanilla `rover_sim_colorable` style points all 9
items at the `WhitePixel` slice from `rover_imageset`; the widget's
`Color` then tints that white pixel to produce a solid fill.

```xml
<Style Name="MySolidFill" Font="" ImageSet="rover_imageset" Color="4294967295">
    <State Name="Normal">
        <Item Name="LeftTop"     Image="WhitePixel" />
        <Item Name="Top"         Image="WhitePixel" />
        <Item Name="RightTop"    Image="WhitePixel" />
        <Item Name="Right"       Image="WhitePixel" />
        <Item Name="RightBottom" Image="WhitePixel" />
        <Item Name="Bottom"      Image="WhitePixel" />
        <Item Name="LeftBottom"  Image="WhitePixel" />
        <Item Name="Center"      Image="WhitePixel" />
        <Item Name="Left"        Image="WhitePixel" />
    </State>
    <State Name="Disabled">
        <!-- Same 9 items, or empty Image="" to make disabled invisible -->
    </State>
</Style>
```

The `Color="..."` is an ARGB integer (alpha in high byte):
`4294967295 = 0xFFFFFFFF` = opaque white (no tint). Replace with your
target color expressed as a decimal integer (e.g. `4027849510 =
0xF0070A26`).

The widget can override the style color via the `color R G B A` layout
property (floats 0..1) — useful when you want one shared style with
per-instance tinting.

## Loading custom styles

```c
LoadWidgetStyles("MyMod/styles/my_widgets.styles");
```

Call this once at mod startup. **Must be called before any layout that
uses the custom styles is loaded** — otherwise widgets construct with no
style and render incorrectly. COT loads `cot.styles` from
`MissionGameplay.OnInit()` (line 72), which runs before any in-game
COT layout is shown.

## COT's custom styles

COT ships `JM/COT/GUI/styles/cot.styles` defining:

- **`COTSurface`** — solid dark navy-blue panel surface used as the
  standard background for COT windows, panels, list rows, etc.
  Registered under `PanelWidget`, `GridSpacerWidget`, `WrapSpacerWidget`,
  and `ScrollWidget`. Default color `0xF0070A26` (very dark navy ~94%
  opaque); widgets may override per-instance with `color R G B A`.

When adding a new COT custom style, follow the same pattern: define it
under every widget type that will use it, include both `<State Name="Normal">`
and `<State Name="Disabled">` blocks, and add an audit comment in
`cot.styles` listing the consumer widget types.

## Custom styles: what actually breaks (COT findings, 2026-08-21)

Three things were established the hard way while adding COT's own nine-slice
styles. All three are engine behaviour, not convention.

### 1. An unresolvable ImageSet is a CTD, not a blank widget

A style whose `ImageSet` cannot be resolved crashes the engine the moment a
widget wearing it is **rendered**:

```
Reason: Access violation. Illegal read by 0x... at 0x20
```

`0x20` is a field offset off a null pointer -- the renderer dereferenced the
missing imageset. There is **no script trace**, and the script log simply stops
mid-frame.

This is the same class of failure as the (widget class, style name) trap below,
and it fires under the same conditions: not at `LoadWidgetStyles` time, not when
the layout is created, but on the first paint.

Two ways to hit it:

- `ImageSet=""` with `<Item Image="some/path.paa">`. **Raw texture paths in a
  style do not work.** Zero of the 4462 lines of `dayzwidgets.styles` use one;
  every Item resolves against an atlas slice. Do not try it.
- `ImageSet="my_set"` where `my_set` failed to register. `LoadWidgetImageSet`
  returns a bool -- **check it**, and skip creating the styled widget when it is
  false, or the fallback path is a crash.

Note the blast radius: the crash happens on render of *any* widget carrying the
bad style, and a layout's content is usually built when its form is constructed,
not when its tab is first shown. In COT's Example form every tab's `Init*()`
runs up front, so a bad style on a hidden tab still crashes on form open.

### 2. `.edds` is a DDS header wrapping per-mip LZ4

The first 128 bytes are a real DDS header and it tells the truth: uncompressed
A8R8G8B8 (`pfFlags 0x41`, no fourcc), with the ASCII tag `ENF1` in `reserved[1]`
at offset `0x24`. What follows is not a DDS payload -- it is an Enfusion chunk
container:

```
[0x80 .. )   chunk table, one 8-byte entry per mip, SMALLEST MIP FIRST:
               char[4] codec ("COPY" or "LZ4 "), u32 chunkSize
[ .. EOF)    chunk payloads, same order:
               "COPY" -> raw BGRA bytes
               "LZ4 " -> u32 rawSize, u32 (compSize | 0x80000000),
                         then a raw LZ4 block (no frame, no size prefix)
```

So a genuine DDS renamed to `.edds` is still a *different format wearing the
same extension* -- but the container is writable from a script. **This repo has
a CLI converter**: `Workbench/Batchfiles/icons/` (`edds.py` for the container,
`build_icons.py` to drive SVG -> `.edds`). Re-encoding
`GUI/textures/new_cursor.edds` reproduces the Workbench header byte-for-byte and
mip 0 pixel-for-pixel; only the smaller mips differ, because Workbench uses a
different downsample filter.

DayZ Tools ships no image-to-edds tool (only `ImageToPAA`), so before that
converter existed the only route was importing a PNG in Workbench. Those imports
leave a `.meta` recording the source:

```
MetaFileClass {
 Name "{ED2D254B10292F9D}egoland/bg_card.edds"
 Configurations { PNGResourceClass PC { SourceFile "bg_card.png" } }
}
```

The engine does not need that `.meta` to load a texture -- the icon sets under
`GUI/textures/icons/` ship without one.

### 3. The `{GUID}` on a texture path is not authoritative

Four of the ten vanilla imagesets carry a GUID that disagrees with their own
`.edds.meta` -- `rover_imageset` among them, and that one backs
`rover_sim_colorable`, the most-used style in the game. Stale GUIDs still
resolve, so the engine is matching on the path. A hand-written imageset does not
need a "real" GUID.

## Engine truth

> The styles XML schema is parsed by the engine; format is documented by
> inspecting `gui/looknfeel/dayzwidgets.styles` (4462 lines in vanilla
> 1.x) and the BI Wiki entry on Widget Styles.
> ```
> proto native void LoadWidgetStyles(string filename);
> proto native bool LoadWidgetImageSet(string filename);
> ```
> declared at lines 692–693 of `enwidgets.c`.
