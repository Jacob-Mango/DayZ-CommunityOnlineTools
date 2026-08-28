# DayZ Default Styles

Reference list of the most commonly used named styles, where they're
defined, and what they look like.

## Browse the source

The vanilla styles live in:

```
gui/looknfeel/dayzwidgets.styles
```

4462 lines, fully expanded. Open it in a text editor to see every
defined style for every widget class.

## TextWidget styles

| Style | Font | Note |
|---|---|---|
| `Normal` | `MetronBook` | Default body text (bitmap font) |
| `Bold` | `MetronBook-Bold` | Bold body |
| `Light` | `MetronLight` | Light-weight |
| `None` | `sdf_MetronBook72` | Largest available SDF |

## MultilineTextWidget / MultilineEditBoxWidget styles

| Style | Use |
|---|---|
| `DayZNormal` | Default chat / dialog text |
| `DayZBold` | Section headers |

These styles include `ListboxHighlight` images for the active line.

## RichTextWidget styles

| Style |
|---|
| `DayZNormal` |
| `DayZBold` |

Same as multiline.

## ProgressBarWidget styles

| Style | Use |
|---|---|
| `Default` | Generic progress |
| `DayZLoading` | The main-menu loading bar |
| `DayZHealth` | HUD health bar |
| `DayZRecoil` | Weapon recoil indicator |
| `DayZStamina` | HUD stamina bar |

Each uses 9-slice imagery with separate "bar" and "track" portions.

## ButtonWidget styles

| Style | Use |
|---|---|
| `DayZButton` | Default rectangular button |
| `DayZButtonRound` | Rounded variant |
| `DayZSettingsButton` | Settings menu buttons (larger, with focus rim) |

## CheckBoxWidget styles

| Style | Use |
|---|---|
| `Default` | Standard checkbox |
| `Editor` | Editor's square box (used by camera tools) |

## TextListboxWidget styles

| Style | Use |
|---|---|
| `Default` | Default listbox |
| `Editor` | Editor variant (smaller, simpler) |

## SliderWidget styles

| Style | Use |
|---|---|
| `Default` | Horizontal slider |

## EditBoxWidget styles

| Style | Use |
|---|---|
| `Default` | Default text input |

## Panel styles

| Style | Use |
|---|---|
| `DayZDefaultPanel` | Standard dark transparent background |
| `rover_sim_colorable` | Accepts `SetColor` tinting properly (without this, `SetColor` may have no effect on some panel styles) |
| `Empty` | No visuals (used for hierarchy containers that shouldn't render anything) |
| `Colorable` | Variant of rover_sim_colorable for buttons |

## How to discover what a style looks like

Open `dayzwidgets.styles`, search for `Name="StyleName"`, then look at
the referenced images in `<Item Name=... Image="..." />` entries. The
`Image` attribute is an imageset slice; you can find that in the
matching `.imageset` (e.g. `gui/imagesets/dayz_gui.imageset`).

The actual visual is the rendered PAA texture at the slice's UV
rectangle.

## When to use custom styles

If you want a consistently different look (e.g. a green primary button
across the whole mod), define a custom style in your own `.styles` file
and load it with `LoadWidgetStyles`.

If you only need per-widget colour changes, `SetColor` on a stock style
is simpler.

## COT's conventions

- COT uses `DayZDefaultPanel` for the form root.
- Buttons use the default `DayZButton` style, then `SetColor(COLOR_GREEN)`
  / `SetColor(COLOR_RED)` for emphasis.
- Custom visuals (the icon-only refresh button, the toggle-style
  checkbox) are implemented at the **layout file** level rather than as
  custom styles. Trade-off: more layouts to maintain, but no risk of
  conflicting with other mods that touch `dayzwidgets.styles`.

## Engine truth

> Style data is XML, loaded by the engine at startup or via
> `LoadWidgetStyles(filename)`. The format is documented by examining
> `gui/looknfeel/dayzwidgets.styles` directly.
