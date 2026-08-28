# `WrapSpacerWidget`

Flow layout that arranges children left-to-right and wraps to a new line
when they don't fit. The DayZ equivalent of CSS `flex-wrap: wrap` or
HTML's natural inline flow.

## Class

```c
class WrapSpacerWidget extends SpacerWidget {};
```

Like `GridSpacer`, no script API beyond the inherited content alignment.

## Layout properties

```
WrapSpacerWidgetClass my_wrap {
 position 0 0
 size 1 30
 hexactpos 0
 vexactpos 1
 hexactsize 0
 vexactsize 1
 Padding 0
 Margin 0
 "Size To Content H" 1
 "Size To Content V" 1
 align horizontal_left
 align vertical_center
}
```

## How wrapping works

Children flow left-to-right (within the spacer's width). When the next
child would overflow, it moves to a new line below.

Each child's width is computed from its `SetWidth(fraction)`. The
fractions are interpreted as fractions of the **spacer's width** — so
0.5 + 0.5 = 1.0 fills the row exactly. 0.6 + 0.6 = 1.2 → overflow →
second child wraps to next line.

Fixed-size children (e.g. `SetFixedSize(32, 32)`) consume their absolute
pixel width first; fractional siblings share the **remaining** width.

## Compact vs normal

COT has two variants:
- `CreateWrapSpacer` — uses `UIWrapSpacer.layout`, normal spacing
- `CreateWrapSpacerCompact` — uses `UIWrapSpacerCompact.layout`, zero padding between children
- `CreateWrapSpacerFit` — auto-fits to content

The "compact" variant is what causes the classic "icon next to text with
no gap" look (e.g. trash icon hugging a class name). For laid-out rows
with breathing room, use the normal `CreateWrapSpacer`.

## When children stay on one line

The wrap only happens when the sum of fractional widths > 1.0 OR when
fixed-size children + fractional widths overflow.

This is the **standard pattern** for "stay-on-one-line" rows:

```c
Widget row = UIActionManager.CreateWrapSpacer(parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

UIActionImageButton icon = UIActionManager.CreateIconButton(row, ICON_TRASH, this, "OnDelete");
icon.SetFixedSize(32, 32);    // consumes 32 px

UIActionText label = UIActionManager.CreateText(row, "", "Item name");
label.SetWidth(0.75);          // 75% of remaining width

UIActionButton spawn = UIActionManager.CreateButton(row, "Spawn", this, "OnSpawn");
spawn.SetWidth(0.25);          // 25% of remaining width

// 32 px (fixed) + 75% + 25% = full row, no wrap
```

## Patterns

### Toolbar (icon + search box)

```c
Widget toolbar = UIActionManager.CreateWrapSpacer(parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

UIActionImageButton refresh = UIActionManager.CreateIconButton(toolbar, ICON_REFRESH, this, "OnRefresh");
refresh.SetFixedSize(32, 32);

UIActionSearchBox search = UIActionManager.CreateSearchBox(toolbar, this, "OnSearch", "Search…");
search.SetWidth(1.0);          // fills remaining
```

### Two equal items

```c
chkA.SetWidth(0.5);
chkB.SetWidth(0.5);
```

### Right-aligned button

WrapSpacer doesn't natively support "stick to the right". To approximate:

```c
spacer = CreateWrapSpacer(...);
spacer.SetContentAlignmentH(WidgetAlignment.WA_RIGHT);   // align children to right
btn.SetWidth(0.3);  // takes 30%, sits at right edge thanks to alignment
```

Or insert a stretchy spacer:

```c
filler.SetWidth(0.7);
btn.SetWidth(0.3);  // pushed to the right
```

## Common pitfalls

### Fractions summing > 1 cause wrap

```c
btn1.SetWidth(1.0);
btn2.SetWidth(0.4);   // total 1.4 → wraps to next line
```

If `btn2` was supposed to sit beside `btn1`, you've lost it to wrap. Set
`btn1.SetWidth(0.6)` instead.

### Mixed pixel + fraction siblings

Pixel-sized children consume their absolute size first; fractional
siblings then divvy up the remainder. If you have multiple pixel-sized
children, their total is subtracted from the row width before fractions
apply.

### Compact spacer + long text overlapping

`CreateWrapSpacerCompact` packs children with zero padding. With a wide
fractional text child next to a fixed-size icon, the text may render its
glyphs directly adjacent to the icon. Use the normal `CreateWrapSpacer`
for visual breathing room.

## Engine truth

> Defined at lines 477–479 of `scripts/1_core/proto/enwidgets.c`. The
> layout engine reads `Padding`, `Margin`, `Size To Content H/V` from the
> layout file and arranges children at runtime.
