# DayZ UI Layout System: Limitations & Quirks

This document captures known limitations, quirks, and workarounds for DayZ's Enforce Script UI layout system discovered during Community Online Tools development.

## Table of Contents
- [Size-To-Content Propagation Failures](#size-to-content-propagation-failures)
- [GridSpacer Always Shrinks to Content](#gridspacer-always-shrinks-to-content)
- [No "Fill Remaining Space" Pattern](#no-fill-remaining-space-pattern)
- [Mixed Fractional/Pixel Sizing Conflicts](#mixed-fractionalpixel-sizing-conflicts)
- [WrapSpacer Vertical Alignment Issues](#wrapspacer-vertical-alignment-issues)
- [Horizontal Scrolling Challenges](#horizontal-scrolling-challenges)
- [Widget Positioning Edge Cases](#widget-positioning-edge-cases)

---

## Size-To-Content Propagation Failures

### Problem
When nesting widgets with `Size To Content H 1` or `Size To Content V 1`, the parent often fails to measure the child's actual content size, collapsing the chain to 0×0.

### Observed Pattern
```
GridSpacer (Size To Content H 1)
  └─ WrapSpacer (Size To Content H 1)
       └─ IconGrid Panel (Size To Content H 1)
            └─ WrapSpacer (Size To Content H 1)
                 └─ Chips (110×30 exact)
```
**Result**: Chips render at 0×0 despite being declared 110×30.

### Workaround
**Explicitly force pixel sizes in script after layout creation:**
```c
// After adding all children to the innermost spacer:
m_Grid.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE );
m_Grid.SetSize( totalPixelWidth, exactPixelHeight );
```

Avoid relying on `Size To Content` chains deeper than 2 levels. Measure content manually and call `SetSize` with exact values.

### Why It Fails
The engine appears to resolve Size-To-Content in a single layout pass. When a parent queries a child's size before the child has computed its own content size, the parent caches 0. No second pass corrects this.

---

## GridSpacer Always Shrinks to Content

### Problem
All `CreateGridSpacer` templates use `Size To Content V 1`, causing grids to shrink-wrap their content height rather than filling their parent panel.

### Observed Behavior
```c
Widget browsePanel = layoutRoot.FindAnyWidget( "object_browse_wrapper" );
// browsePanel is 50% of form height = ~250 px
Widget browseGrid = UIActionManager.CreateGridSpacer( browsePanel, 1, 2 );
// browseGrid shrinks to child height (~150 px)
// Leaves ~100 px empty space below the grid inside browsePanel
```

### Impact
- List + preview panes don't expand to fill available vertical space when form is resized.
- Empty gaps appear below content even when the parent panel is larger.

### Workaround
**Skip the GridSpacer wrapper entirely** for children that should fill the parent:
```c
// Instead of CreateGridSpacer(browsePanel, 1, 2) + adding children to grid...
// Add children directly to the panel with fractional positioning:

// List: left half, full height
TextListboxWidget list = TextListboxWidget.Cast(
    g_Game.GetWorkspace().CreateWidgets( "list.layout", browsePanel ) );
// list.layout declares: size 0.5 1, position 0 0

// Preview: right half, full height  
ItemPreviewWidget preview = ItemPreviewWidget.Cast(
    g_Game.GetWorkspace().CreateWidgets( "preview.layout", browsePanel ) );
// preview.layout declares: size 0.5 1, position 0.5 0
```

### Why It Happens
GridSpacer templates are designed for toolbar-style layouts where content height determines container height. No template exists for "fill parent height" grids.

---

## No "Fill Remaining Space" Pattern

### Problem
DayZ layouts have no equivalent to CSS `flex-grow` or `height: calc(100% - 200px)`. A widget cannot declaratively "fill remaining space after fixed-height siblings."

### Attempted Patterns That Don't Work

#### 1. Negative size
```
size 1 -200  // Syntax error or interpreted as 0
```

#### 2. Mixed anchors
```
position 0 100   (top anchor, 100px from top)
size 1 1         (full parent height)
valign top_ref   
```
**Result**: Widget extends to `y=100 + parent_height`, overflowing bottom.

#### 3. Bottom-anchored with top offset
```
position 0 100
size 1 200
valign bottom_ref
```
**Result**: Widget positions at `parent_bottom - 100` (ignores the "200 from top" intent).

### Workaround: All-Fractional Layout

**Divide the form into fractional regions summing to 1.0:**
```
search:     position 0 0.00,  size 1 0.08   (8% of form)
categories: position 0 0.08,  size 1 0.14   (14%)
browse:     position 0 0.22,  size 1 0.40   (40%)  ← this is your "fill"
actions:    position 0 0.62,  size 1 0.38   (38%)
```

**Trade-offs:**
- ✅ Scales proportionally on resize
- ❌ Fixed-pixel sections (like a 40-px toolbar) waste space at large form sizes
- ❌ Can't make "browse fills, others are exact pixel heights"

### When To Use What

| Requirement | Approach |
|-------------|----------|
| Form rarely resizes, fixed layouts OK | Use exact pixel `vexactsize 1` for all panels |
| Form resizes, all sections should scale | Use fractional `vexactsize 0` summing to 1.0 |
| Need scrolling when content exceeds form | Wrap everything in `UIActionScroller` (like Ban Manager) |
| Need one section to fill dynamically | **Not cleanly possible** — pick fractional or accept wasted space |

---

## Mixed Fractional/Pixel Sizing Conflicts

### Problem
Setting `vexactsize 1` (exact pixels) on a widget, then having script call `SetSize(x, fractionOfParent)` creates undefined behavior depending on call order.

### Example
```c
// Layout declares:
size 1 200
hexactsize 0
vexactsize 1   // 200 px exact

// Script does:
widget.SetSize( w, h );   // Tries to set fractional
widget.SetFlags( WidgetFlags.HEXACTSIZE );
```

**Result varies** by whether `SetFlags` before or after `SetSize`, and whether layout's flag was parsed before script runs.

### Workaround
**Always SetFlags BEFORE SetSize** when forcing exact sizing:
```c
widget.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE );
widget.SetSize( pixelWidth, pixelHeight );
```

**Choose one system per widget** — either layout declares exact + script never touches it, OR layout uses fractional + script forces exact.

---

## WrapSpacer Vertical Alignment Issues

### Problem
Children added to a WrapSpacer with default anchoring sometimes render outside the WrapSpacer's visible bounds, even when the child size fits.

### Observed Case
```
WrapSpacer (30 px tall, position 0 0, valign top_ref)
  └─ Chip cell (30 px tall, no explicit position/anchor in layout)
```

**Result**: Chip backgrounds partially render ABOVE the WrapSpacer's parent panel, overlapping the panel above it in the form.

### Root Cause (Theory)
WrapSpacer uses an internal layout algorithm that positions children at fractional row offsets by default. When the child's layout lacks explicit `hexactpos 1 vexactpos 1`, the engine interprets child `position 0 0` as fractional anchor center or uses flow-based positioning that doesn't respect pixel boundaries.

### Workaround
**Explicitly anchor child widgets top-left with exact positioning:**
```
PanelWidgetClass icon_cell {
 position 0 0
 size 110 30
 halign left_ref
 valign top_ref
 hexactpos 1        ← Exact X position
 vexactpos 1        ← Exact Y position
 hexactsize 1
 vexactsize 1
 ...
}
```

This forces the child's top-left corner to align exactly at the parent's top-left, preventing drift.

---

## Horizontal Scrolling Challenges

### Problem
Creating a horizontally-scrolling row of labeled chips required 3 failed attempts before finding a stable pattern.

### Failed Approaches

#### Attempt 1: Size-To-Content-H chain
```
UIActionScrollerH
  └─ Content GridSpacer (Size To Content H 1)
       └─ WrapSpacer (Size To Content H 1)
            └─ Chips
```
**Result**: Chips rendered 0×0 (Size-To-Content propagation failure).

#### Attempt 2: Script SetSize after AddIcon
```c
foreach chip: m_IconGrid.AddIcon(...);
float contentW, contentH;
m_Grid.GetScreenSize( contentW, contentH );  // contentW = 0
m_Grid.SetSize( contentW, 40 );
```
**Result**: `GetScreenSize` returned 0 even after chips added.

#### Attempt 3: Force pixel width in deferred call
```c
GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( this.FixScroller, 34 );
void FixScroller() {
    m_Grid.GetScreenSize( contentW, contentH );  // Still 0
}
```
**Result**: Even 34ms later, engine hadn't resolved content size.

### Working Solution
**Compute expected width mathematically and force it:**
```c
m_TypeFilter.AddIcon( "all", icon_path, "All" );
m_TypeFilter.AddIcon( "food", icon_path, "Food" );
// ... 27 total chips at 110 px wide + ~5 px gap = 115 px each

m_TypeFilter.ForceContentWidth( 27 * 115, 30 );

// Inside ForceContentWidth:
m_Grid.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE );
m_Grid.SetSize( 27 * 115, 30 );

// Force root width (height stays fractional to avoid parent conflicts):
layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE );
layoutRoot.SetSize( 27 * 115, parentHeight );
```

**Key insights:**
- Can't rely on engine to compute content size for Size-To-Content chains
- Must hardcode width = `chipCount × chipWidth` if chip count is static
- Force ONLY the WrapSpacer height to exact (prevents wrapping to row 2)
- Leave outer widget height fractional to avoid conflicts with parent GridSpacer cell

---

## Widget Positioning Edge Cases

### Fractional Position + Exact Size = Trouble
```
position 0 0.08   ← 8% from top
size 1 50         ← 50 px tall
vexactsize 1      ← exact
```
When form is 510 px: position = 41 px, size = 50 px → ends at 91.  
When form is 800 px: position = 64 px, size = 50 px → ends at 114.

**Issue**: Following sibling at `position 0 0.18` (18% from top) = 92 px at 510, 144 px at 800.  
- At 510: sibling starts at 92, previous ends at 91 → 1 px gap ✓  
- At 800: sibling starts at 144, previous ends at 114 → **30 px gap**

### Workaround
**Use consistent sizing modes across siblings:**
- All fractional: `position 0 0.08 size 1 0.10` (ends at 0.18)
- All exact: `position 0 41 size 1 50 vexactpos 1 vexactsize 1` (breaks on resize)

### Widget Overlap with valign bottom_ref
```
PanelA: position 0 0, size 1 200, valign top_ref
PanelB: position 0 0, size 1 220, valign bottom_ref
```
**Result**: Both render, PanelB draws on top. At 510 form:
- PanelA occupies y=0 to y=200
- PanelB occupies y=290 to y=510 (510 - 220)
- Middle region y=200-290 is neither panel (gap)

To have PanelB start where PanelA ends, you'd need to compute PanelA's height and set PanelB's bottom offset accordingly — **but there's no declarative way to do this**.

---

## Checkbox State Reversal Bug

### Problem
`UIActionToggle.OnClick` was firing `CallEvent(UIEvent.CLICK)` **before** toggling `m_Checked` state via `SetChecked(!m_Checked)`.

### Impact
Handlers filtering `if (eid != UIEvent.CLICK) return;` then calling `action.IsChecked()` read the **OLD** state, causing stored module state to be reversed from the visual checkbox state.

### Fix
```c
// WRONG ORDER:
CallEvent( UIEvent.CLICK );
SetChecked( !m_Checked );

// CORRECT ORDER:
SetChecked( !m_Checked );  // Toggle state first
CallEvent( UIEvent.CLICK ); // Then fire event so IsChecked() returns new state
```

### Lesson
Event handlers assume `IsChecked()` reflects the NEW state at CLICK time. Always mutate state before firing the event.

---

## Checkbox Background Color Contrast

### Problem
Unchecked checkbox box was `ARGB(255, 120, 120, 130)` (medium grey) against a dark blue form background (`COTSurface` style). Low contrast made unchecked state nearly invisible.

### Fix
Increased luminosity:
```c
// Before:
static const int COLOR_BOX_OFF = ARGB( 255, 120, 120, 130 );

// After:
static const int COLOR_BOX_OFF = ARGB( 255, 170, 180, 200 );
```

Also increased `COLOR_BOX_ON` from `(30, 80, 160)` to `(79, 156, 255)` to match the form's accent blue.

### Lesson
When designing toggle states, test contrast against the actual background surface color, not white/black assumptions.

---

## Summary Best Practices

1. **Avoid Size-To-Content chains deeper than 2 levels** — manually compute + `SetSize` instead.

2. **Choose fractional OR exact sizing per form section**, don't mix them haphazardly.

3. **Explicitly anchor children in WrapSpacer** with `position 0 0 halign left_ref valign top_ref hexactpos 1 vexactpos 1`.

4. **When forcing widget size in script**, always `SetFlags` before `SetSize`.

5. **For "fill remaining space"**, use all-fractional layout with proportions summing to 1.0.

6. **For horizontal scrolling**, compute content width mathematically — don't rely on `GetScreenSize` after adding children.

7. **GridSpacer for stacked toolbars**, direct fractional positioning for fill-height panes.

8. **Toggle state changes before firing events**, not after.

9. **Test UI color contrast** against real form backgrounds, especially for OFF/disabled states.

---

## Open Questions / Unresolved

- Why does `Size To Content H` propagate through some nesting patterns but not others?
- Is there a widget type that DOES support "fill remaining" natively?
- Can `clipchildren` on a parent force child bounds, or is explicit anchoring always needed?
- Does `SetSort()` widget order affect layout resolution?

---

*Last updated: 2025-01-XX*  
*Session: Community Online Tools Object Spawner form redesign*
