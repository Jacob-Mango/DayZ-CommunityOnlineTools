# ControlID Enum

From `scripts/1_core/proto/enwidgets.c` lines 639–654. Used in
`OnController(Widget w, int control, int value)` callbacks.

```c
enum ControlID
{
    CID_NONE       = 0,
    CID_SELECT     = 1,   // A / X (PS)  — confirm focused widget
    CID_BACK,             // B / Circle  — cancel / back
    CID_LEFT,             // D-pad left  — navigation
    CID_RIGHT,            // D-pad right
    CID_UP,               // D-pad up
    CID_DOWN,             // D-pad down
    CID_MENU,             // Start       — open main menu
    CID_DRAG,             // Y / Triangle — likely for drag-to-pickup
    CID_TABLEFT,          // LB / L1     — previous tab
    CID_TABRIGHT,         // RB / R1     — next tab
    CID_RADIALMENU,       // (radial menu trigger)
    CID_COUNT             // not a real ID; the enum's count
};
```

## Value semantics

The `value` argument to `OnController`:
- Digital buttons send `0` (released) or `1` (pressed)
- Analogue triggers can send `0..255`
- Sticks aren't routed through `OnController` — they affect cursor / focus directly

## Typical handler

```c
override bool OnController(Widget w, int control, int value)
{
    if (value == 0)
        return false;   // ignore release events

    switch (control)
    {
    case ControlID.CID_SELECT:
        Confirm();
        return true;

    case ControlID.CID_BACK:
        Cancel();
        return true;

    case ControlID.CID_TABRIGHT:
        NextTab();
        return true;

    case ControlID.CID_TABLEFT:
        PrevTab();
        return true;
    }
    return false;
}
```

## DayZ console controller mapping

Xbox / PlayStation conventions:

| ControlID | Xbox | PlayStation |
|---|---|---|
| `CID_SELECT` | A | × (Cross) |
| `CID_BACK` | B | ○ (Circle) |
| `CID_LEFT` | D-pad left | D-pad left |
| `CID_RIGHT` | D-pad right | D-pad right |
| `CID_UP` | D-pad up | D-pad up |
| `CID_DOWN` | D-pad down | D-pad down |
| `CID_MENU` | Start | Options |
| `CID_DRAG` | Y | △ (Triangle) |
| `CID_TABLEFT` | LB | L1 |
| `CID_TABRIGHT` | RB | R1 |
| `CID_RADIALMENU` | (radial bind) | (radial bind) |

## Cross/Circle swap on Japan/Asia consoles

On some PS3/PS4/PS5 systems sold in Japan and Asia, the Circle button
is "confirm" and Cross is "cancel" — the opposite of Western convention.
DayZ exposes this with the PS3-only proto:

```c
#ifdef PS3
proto native bool IsCircleToCrossSwapped();
#endif
```

If `true`, swap your understanding of `CID_SELECT` and `CID_BACK` on
PS3. (Not exposed on newer console SDKs; engine handles the swap
internally.)

## Engine truth

> Enum at lines 639–654 of `scripts/1_core/proto/enwidgets.c`.
> Callback signature: `bool OnController(Widget w, int control, int value)`
> at line 671.
