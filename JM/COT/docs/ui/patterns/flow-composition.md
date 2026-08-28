# Flow Composition — Building Rows and Columns

The recurring problem: place several widgets on one line where some have
fixed pixel sizes (icons) and others stretch to fill space (labels,
search boxes). DayZ's layout system has no `flex: 1` — you have to
combine `SetFixedSize` and `SetWidth` carefully so things sum to 1.0.

## The mental model

A `WrapSpacer` lays children out left-to-right.

1. **Fixed-size children** (`SetFixedSize(w, h)`) consume their absolute
   pixel width first.
2. **Fractional children** (`SetWidth(0..1)`) divide the **remaining**
   space proportionally.
3. If the sum of fractions exceeds 1.0, the row wraps.

This gives the "icon + stretch + button" pattern you'd write in CSS as
flexbox.

## Standard inline-row recipes

### Toolbar: refresh icon + search box

```c
Widget toolbar = UIActionManager.CreateWrapSpacer(parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

UIActionImageButton refresh = UIActionManager.CreateIconButton(toolbar, ICON_CLOCKWISE, this, "OnRefresh");
refresh.SetFixedSize(32, 32);

UIActionSearchBox search = UIActionManager.CreateSearchBox(toolbar, this, "OnSearch", "Search…");
search.SetWidth(1.0);   // fills 100% of remaining width
```

Result: `[🔄] [search ──────────────────────────────────]`

### Per-entry row: icon-only delete + spawn + label

```c
Widget row = UIActionManager.CreateWrapSpacer(parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

// 1) Icon-only delete (32x32). Compact "O"/"X" confirm labels so the
//    confirm gesture fits inside the square — full "Confirm"/"Cancel"
//    clips at narrow widths.
UIActionConfirmInline delBtn = UIActionManager.CreateConfirmInline(row, "", this, "OnDelete");
UIActionIconGrid.ApplyDeletePreset(delBtn);
delBtn.SetButton("");
delBtn.SetFixedSize(32, 32);
delBtn.CenterIcon(32, 16);
delBtn.SetConfirmLabel("O");
delBtn.SetCancelLabel("X");

// 2) Spawn directly after the delete icon. Fractional (NOT fixed-pixel)
//    so the WrapSpacer can pack all siblings on the same row.
UIActionButton spawn = UIActionManager.CreateButton(row, "Spawn", this, "OnSpawn");
spawn.SetWidth(0.20);

// 3) Label fills the rest of the row.
UIActionText name = UIActionManager.CreateText(row, "", entry.Name);
name.SetWidth(0.78);
```

Sum check: `32 px (fixed) + 20% + 78% = 100% of remaining width`. Fits
exactly, no wrap. All four siblings share the same row.

The `SetConfirmLabel("O")` / `SetCancelLabel("X")` pair is
`UIActionConfirmInline`'s `UseIconConfirmLabels()` minus the "OK":
the buttons themselves fit inside a 32 px square only when the
confirm/cancel text is one ASCII character. Multi-byte Unicode in the
script source caused a CTD on COT open (per comment in
`UIActionConfirmInline.c`), so stick to single-byte ASCII glyphs.

### Two equal columns

```c
chkA.SetWidth(0.5);
chkB.SetWidth(0.5);
```

Sum = 1.0. Both share the row equally.

### Label : value (right-aligned)

```c
Widget row = UIActionManager.CreateWrapSpacer(parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

UIActionText label = UIActionManager.CreateText(row, "Server", "");
label.SetWidth(0.3);

UIActionText value = UIActionManager.CreateText(row, "", "localhost:2302");
value.SetWidth(0.7);
// UIActionText's value-slot is right-aligned by default
```

## The cardinal rule

> **Fractional `SetWidth` values across siblings should sum to ≤ 1.0.**
> Anything more wraps. Anything less leaves whitespace.

Common mistake: `0.85 + 0.4 = 1.25` → second widget wraps to a new line
and looks lost.

## Anatomy of "right edge sticky" buttons

To put a button at the **far right** of a row:

```c
// Method 1: stretchy filler
spacer.SetWidth(1.0);     // takes everything
button.SetFixedSize(80, 28);
// Result: filler eats the whole row, button gets pushed out → WRONG

// Method 2: math out the button width
filler.SetWidth(0.7);
button.SetWidth(0.3);     // sits flush right because 0.7 + 0.3 = 1.0

// Method 3: content alignment
row.SetContentAlignmentH(WidgetAlignment.WA_RIGHT);
button.SetFixedSize(80, 28);
// Now the button is right-aligned within the row, no filler needed
```

Method 3 is the cleanest if you don't need anything to the left.

## Patterns to avoid

### Absolute positioning (`SetPos` + `SetWidth`)

```c
btn1.SetWidth(0.45);
btn1.SetPosition(0.05);   // 5% from left
btn2.SetWidth(0.45);
btn2.SetPosition(0.55);   // 55% from left
```

Fragile: changes to one widget require manual math on the next.
Use `WrapSpacer` flow instead.

### Nested grids for inline rows

```c
GridSpacer top(1, 3);
  ...
  GridSpacer inner(1, 3);    // a row inside a column? confusing
```

If the row is just "few widgets side by side", use a `WrapSpacer`
directly. Reserve grids for tabular data where ALL rows share the same
column structure.

### Mixing fixed-pixel and fractional siblings in a WrapSpacer

Each WrapSpacer sibling is either **fixed-pixel** (`SetFixedSize(w, h)`)
or **fractional** (`SetWidth(0..1)`). Mixing both on one row works, but
the fixed-pixel siblings consume their absolute width **first** and the
fractional siblings divide the **remaining** space. Headroom:

```c
// WRONG: 32 px icon + 0.20 + 0.78 + 0.55 = fraction sum > 1.0
//        → the label wraps to a new line.
delBtn.SetFixedSize(32, 32);
spawn.SetWidth(0.20);
name.SetWidth(0.78);
label.SetWidth(0.55);   // ← pushes total > 1.0

// RIGHT: leave ~0.05 of headroom when there is a fixed-px icon sibling.
//        WrapSpacer reserves the icon's 32 px from the row budget before
//        splitting the remainder among fractional siblings, but the
//        fractional sum still has to be ≤ 1.0 or the engine wraps the
//        last sibling. Treat the icon's pixel slice as ~0.05 of a 700 px
//        sidebar and budget accordingly.
delBtn.SetFixedSize(32, 32);
spawn.SetWidth(0.18);
label.SetWidth(0.35);
name.SetWidth(0.55);    // ← total = 0.18 + 0.35 + 0.55 = 0.93
```

### Stacking fixed-size items horizontally

```c
btn1.SetFixedSize(100, 30);
btn2.SetFixedSize(100, 30);
btn3.SetFixedSize(100, 30);
```

Works, but loses responsiveness. If the parent shrinks, content
overflows. Mix at least one fractional child to absorb the resize:

```c
btn1.SetFixedSize(100, 30);
btn2.SetFixedSize(100, 30);
filler.SetWidth(1.0);       // absorbs leftover space
btn3.SetFixedSize(100, 30);
```

(But now btn3 is mid-line because filler goes between 2 and 3 — adjust
order based on intent.)

## Vertical flow

`WrapSpacer` is intrinsically horizontal-flow with wrap. For vertical
stacks, use `GridSpacer(1, 1)`:

```c
Widget stack = UIActionManager.CreateGridSpacer(parent, 1, 1);
// add children — each becomes a new row
```

Or stack `WrapSpacer` rows inside a parent `GridSpacer(N, 1)`.

## See also

- [[../widgets/wrapspacer-widget]] for WrapSpacer details
- [[sizing-rules]] for fractional-vs-exact math
- [[../patterns/containers]] for choosing the container type

## Modal Flow: Chained Confirmations

A two-step flow like "ask for X → ask for Y → act" (e.g. Steam ID
prompt → reason prompt → ban) is a common pattern. Implementation
seems obvious:

```c
void OnClick_BanByID(...)
{
    CreateConfirmation_Two(JMConfirmationType.EDIT, "Ban Offline Player",
        "Enter the Steam 64 ID:", cancelLabel, "", confirmLabel,
        "OnGotSteamID");
}

void OnGotSteamID(JMConfirmation confirmation)
{
    string steamID = confirmation.GetEditBoxValue();
    steamID.Trim();
    if (steamID == "") return;

    m_PendingSteamID = steamID;
    // Looks fine, but the second popup silently fails to open.
    CreateConfirmation_Two(JMConfirmationType.EDIT, "Ban Reason",
        "Reason for banning " + steamID + ":",
        cancelLabel, "", confirmLabel, "OnGotReason");
}
```

**Engine quirk**: calling `CreateConfirmation_Two` from inside the
callback of a previous `CreateConfirmation_Two` (same `JMConfirmationType`)
does NOT open the second modal. The first modal's focus lock isn't
released before the second tries to acquire it. Symptom: first popup
opens and dismisses normally, second popup never appears.

**Workaround**: defer the second `CreateConfirmation_Two` by one
`CallLater` tick so the first modal fully tears down:

```c
void OnGotSteamID(JMConfirmation confirmation)
{
    string steamID = confirmation.GetEditBoxValue();
    steamID.Trim();
    if (steamID == "") return;

    m_PendingSteamID = steamID;
    GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OpenReasonPopup, 50, false);
}

void OpenReasonPopup()
{
    if (m_PendingSteamID == "") return;
    CreateConfirmation_Two(JMConfirmationType.EDIT, "Ban Reason",
        "Reason for banning " + m_PendingSteamID + ":",
        cancelLabel, "", confirmLabel, "OnGotReason");
}
```

50 ms is enough for the engine to release the focus lock. Single-step
confirmations (one `CreateConfirmation_Two` from a button click) are
not affected. The deferral should also re-guard the pending state in
case the first popup was cancelled via timeout — the deferred
function checks the state and bails if it's empty.
