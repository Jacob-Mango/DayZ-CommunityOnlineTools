# Codebase Audit — 2026-07-06

Static analysis findings from the pre-flight tools plus a sweep of the
module code. Items grouped by priority and cluster.

## Quick wins (low risk, high payoff)

### A1. RPC cross-check flags (rpc_check.py)

**Status**: pre-existing, not introduced this session.

- `JMESPModule.c` — `HasPermission("Loadouts.Backup")` and
  `HasPermission("Items.CreateSet")` called but neither string is
  registered via `RegisterPermission`. Either add the registrations
  or remove the dead `HasPermission` calls (the strings appear to
  be from an old feature that was deleted).
- `JMEventsModule.c` × 6 — missing overrides for `GetLayoutRoot`,
  `GetCategory`, `GetTitle`, `GetIconName`, `ImageIsIcon`,
  `HasAccess`. Form registration likely breaks; admin UI shows
  no icon/label.
- `JMExampleModule.c` — same `HasAccess` missing override (this is
  the example/template module, less critical but still inconsistent).
- `JMtemStatsModule.c` × 4 — class not registered in
  `JMModuleConstructor.c` + same missing overrides. Module is
  effectively dead on disk.
- `JMMapEditorModule.c` — `enum JMMapEditorModuleRPC` has `UndoState`
  case not handled in `OnRPC` switch (real bug: server can't deliver
  UndoState RPC).

### A2. Wrap-spacer update mismatch (form-wide)

Several forms still use `SetWidth(1.0)` paired with a `SetFixedSize(32, 32)`
sibling on a WrapSpacer — this is the canonical "wraps on resize" pattern.

- `JMPlayerForm.c` (search + refresh rows)
- `JMMissionSelectorForm.c` toolbar
- `JMESPForm.c` (refresh + dropdown toolbar)

Fix: replace `SetWidth(1.0)` with `SetWidth(0.85)` wherever there's a
fixed-pixel icon sibling, per the pattern in
`docs/ui/patterns/flow-composition.md`.

### A3. Map editor `UndoState` RPC handle

`JMMapEditorModule.c` doesn't handle `UndoState` in `OnRPC`. Server
sends it, client no-ops. Real bug — the Undo / Redo buttons can't
work over RPC. Add a `case JMMapEditorModuleRPC.UndoState:` that
processes the response (could just `break` for now).

### A4. ItemStats module dead on disk

`JMtemStatsModule.c` is in the modules folder but not registered in
`JMModuleConstructor.c`. Either register it (with the missing overrides
above) or remove it from `modules/ItemStats/`.

### A5. ESP permissions use unregistered strings

`JMESPModule.c:Items.CreateSet` and `Loadouts.Backup` are
`HasPermission` checks for strings never registered. Trace the call
chain — likely leftover from removed features. Either register the
permissions in `JMESPModule`'s constructor or remove the dead code
paths.

## Patterns to standardise (form-level)

### P1. Refresh+dropdown toolbar consistency

The "refresh icon (32×32) + dropdown/spacer (0.85)" pattern is now
documented but inconsistently applied. Forms still using the old
`GridSpacer(1, 2)` pattern:

- `JMESPForm.c`
- `JMMissionSelectorForm.c` (different pattern, verified mid-session)
- `JMRoleManagerForm.c` partial (recent fix landed)

Apply the WrapSpacer recipe in `flow-composition.md` to all of these.

### P2. Bulk-action row layout

Stacking destructive actions as one-per-row (`GridSpacer(N, 1)`) is
the entity-manager style. Forms still grouping them horizontally:

- `JMWebhookSection.c` Save + Remove (resolved mid-session — now Save
  full-width below header)
- Any module that hasn't been refactored after the entity-manager
  restructure

Audit each form's per-entry / per-section "destructive action" rows
and confirm N×1 stacking.

### P3. Chained-confirmation deferral

Anywhere a chain like "first popup → second popup → act" exists, the
second popup must be deferred by ~50ms via `CallLater`. Audit chains:

- `JMPlayerModule.BanPlayerMulti` (suspect; verify)
- Any new "ask X then ask Y" flow added in the future

The deferral idiom lives in `JMPlayerBanForm.OpenBanReasonPopup` —
use that as the template.

### P4. GetSelection() defensive coercion

`JMCompensationsForm.OnClick_Spawn` and `JMLoadoutForm.OnClick_Spawn`
already have `-1 → default` coercion + `default:` popup guard. Audit
the remaining spawn handlers for the same pattern — particularly
any switch that uses the dropdown selection directly as a case.

## Performance opportunities

### Perf-1. Vehicle list scroller bounds recompute every tick

`JMVehiclesForm.TickMarkers()` calls `UpdateListScrollerBounds()`
on every marker tick (~20Hz). The function early-exits if the panel
height hasn't changed (`m_LastListPanelH` guard), but the
`m_VehicleListPanel.GetScreenSize()` call still runs every tick.

- Window resize is the only condition that changes panel height.
- Could be driven from `OnResize()` alone, with the tick callback
  purely for marker positioning.

Low priority — the GetScreenSize call is cheap — but it's dead work
for the common case.

### Perf-2. `m_VehicleEntries` reallocation churn

`JMVehiclesForm.LoadVehicles()` clears and rebuilds `m_MapMarkers` +
`m_VehicleEntries` on every refresh (also called via
`OnDeltaUpsert`/`OnDeltaRemove`).

For 200+ vehicles on a refresh:
- Two ~200-entry arrays re-allocated.
- Map markers destroyed + recreated (likely the bigger cost).

`OnDeltaUpsert`/`OnDeltaRemove` already do granular updates (good).
The manual `Refresh` button triggers full reload (acceptable). Could
consider pooling `m_VehicleEntries` to avoid GC churn on full reloads.

Lower priority.

### Perf-3. RPC spam from map editor

Per the `LootAnalysis_Audit.md` recommendation: cache lowercase
strings, format on client, debounce. Map editor's `RebuildList`
might RPC every keystroke from the asset browser search. Audit
`JMMapEditorForm.OnChange_AssetFilter` — likely calls `BuildAssetBrowser`
on every CHANGE event which calls `JMMapEditorCatalog.Build()` — that
rebuilds the entire category tree from disk on every keystroke.

Should:
- Debounce search by 250–300ms (matches existing pattern).
- Cache the lowercase catalog key on first build.
- Rebuild only on category-filter click (which it already does via
  `OnClick_Asset`).

### Perf-4. `m_RefreshListButton` spin animation trigger

`JMObjectSpawnerForm.OnClick_RefreshList` triggers a spin but is
called from many search-driven update paths. If the search triggers
a rebuild on every keystroke AND the search calls back into
`OnClick_RefreshList`, the icon spins constantly — visual noise.

Audit: ensure `OnClick_RefreshList` is only fired from manual user
action, not from programmatic rebuild.

## Layout / structural improvements

### S1. Root-level `Columns N Rows N` audit (already done)

`audit_root_grids.py` was added to `JM/COT/ai/tools/`. Result: every
form flagged has explicit fractional child positioning, so adding
`Columns N Rows N` is a no-op for them. No further action.

### S2. Permission registration in dedicated helper

`RegisterPermission` is called inline in module constructors (e.g.
`JMPlayerModule.c:140` references). A small helper like
`JMPermissions.Register(string name)` could centralize the list and
make a manifest-style overview easy. Low impact — current per-module
registration is fine but the audit found unregistered strings being
checked, which a manifest would catch at lint time.

### S3. `m_PendingSteamID` cleanup pattern in ban form

`OnManualBan_GotReason` clears `m_PendingSteamID` after a successful
ban but doesn't always clear `m_TempData` for the same reason in
compensations/loadouts. Verify cancellation paths in every form
cleared their pending state.

## Dead code / cleanup

### C1. `JMMaterialBase.c` (real fix needed — `BaseBuilding` was removed)

`Classes/BaseBuilding/JMConstructionPartData.c:8` declares
`NOT_ENOUGH_MATERIALS //TODO: link this to a permission` — TODO from
a feature that was never finished. Decide: finish the permission
mapping or remove the TODO + flag value.

### C2. Multiple `Print()` debug calls

Several modules still ship `Print()` debug calls without being
guarded by `#ifdef COT_DEBUGLOGS`. Audit `JMObjectSpawnerForm`,
`JMLoadoutForm`, `JMWebhookForm` — some calls are already inside
the guard but not all.

### C3. `TODO: URGENT: MOVE TO COMMUNITY FRAMEWORK`

`Classes/COTModule.c:318` — explicit URGENT note for refactor. Not
blocking but tracked.

### C4. JMESPModule has duplicate `JMESPForm`-style menu generation
spread across multiple methods. Consolidate into a single
`BuildCategoryMenu` helper for consistency.

## Pattern consistency wins

### I1. Module constructor declaration

Each module's `JMModuleConstructor.c` registration uses different
patterns (some with categories, some without, some with custom
load-order). Standardise via a helper: `RegisterModule(name, getInput, category)`
that wraps the boilerplate.

### I2. Action callback naming

Per `JMCompoundActionEngine`/`UI_STYLE_GUIDE`: `OnClick_*` for
buttons, `OnChange_*` for inputs, `OnConfirmation_*` for popup
results. Audit found mixed conventions in a few forms (e.g.
`Click_*` instead of `OnClick_*`). Grep for `Click_` action handler
names and rename to match.

### I3. Static-scope `Tick` dispatch

`JMMapEditorForm` uses a static `s_Instance` + `TickStatic` indirection
for its per-frame callback (because `CallLater` only resolves static
method names by string). Same pattern would be needed in any form
that uses `CallLater(..., this, "MethodName")`. Already documented
in the JMCameraForm but worth a cross-form audit.

## Recommended order of operations

1. **A1** RPC cross-check findings — fix or remove dead code paths.
2. **A3** Map editor UndoState RPC handle (real bug, easy fix).
3. **A5** ESP permissions cleanup.
4. **A4** ItemStats register-or-delete decision.
5. **Perf-3** Map editor debounce (if confirmed).
6. **A2 / P1** Toolbar WrapSpacer consistency pass.
7. **C1–C3** TODO cleanup pass.
8. **I1–I3** Pattern consistency pass.

## Findings written: 2026-07-06

Based on: `rpc_check.py` (14 findings), `enforce_lint.py --changed`
(clean), `audit_root_grids.py` (audit only), module file count audit,
TODO/FIXME sweep, manual code review of:
- `JMCompensationsForm.c` (1047 lines)
- `JMVehiclesForm.c` (713 lines)
- `JMPlayerBanForm.c` (706 lines)
- `JMLoadoutForm.c` (177 lines)
- `JMMapEditorForm.c` (1067 lines)
- `JMObjectSpawnerForm.c` (969 lines)
- `JMWebhookSection.c` (228 lines)
- `JMTeleportForm.c` (231 lines)
- `JMRoleManagerForm.c` (934 lines)