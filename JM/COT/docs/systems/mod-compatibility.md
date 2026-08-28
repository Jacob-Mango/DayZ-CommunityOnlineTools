# Mod Compatibility — DayZ-Expansion

COT is extended by other mods through Enforce Script `modded class`. Those
mods compile **against COT's source**, so COT's member visibility and method
signatures are a public API contract. Narrowing either breaks the dependent
mod's compile — and the error surfaces in *their* file, not ours.

DayZ-Expansion is the largest consumer: **60 files** touch COT APIs, of which
**16** declare `modded class` over a COT type.

## The rule that matters

> **A `modded class` cannot access a `private` member of the class it mods.**

`private` in Enforce Script is per-class, not per-hierarchy. A `modded class`
is compiled as a *separate* class merged at link time, so `private` shuts it
out entirely. `protected` is the narrowest visibility that still admits
modded subclasses.

This makes `private` → a breaking change for any member a mod reads.

## Members held `protected` for Expansion

Each of these was `private` and is deliberately widened. **Do not re-narrow.**

| Class | Member | Consumed by |
|---|---|---|
| `JMPermissionManager` | `m_MissionLoaded` | `Expansion_RegisterPermission` guard |
| `JMPlayerInstance` (PermissionsOld) | `m_DataLastUpdated` | AI + Hardline `Update()` netsync rate-limit |
| `JMESPModule` | `m_MappedESPObjects` | `ExpansionESPModificationModule.Get(target)` |
| `JMPlayerForm` | `m_RefreshStats`, `m_ApplyStats` | `SetSort()` re-order when injecting rows |
| `JMPlayerForm` | `m_Stamina` | `GetLayoutRoot().GetParent()` used as injection anchor |
| `JMPlayerForm` | `m_SelectedInstance` | reads faction / reputation per player |
| `JMPlayerForm` | `m_Module` | calls `SetExpansionFaction()` / `SetExpansionReputation()` |

Note `JMPlayerInstance.m_Stamina` (a `private float` stat) is **intentionally
still private** — it is a different member from `JMPlayerForm.m_Stamina`
(`UIActionSlider`) and no mod touches it. Don't widen by name-matching.

### Keep the two JMPlayerInstance variants in sync

There are two: `4_World/.../PermissionsOld/JMPlayerInstance.c` (active) and
`3_Game/.../Permissions/JMPlayerInstance.c` (`#ifdef CF_MODULE_PERMISSIONS`).
The CF variant already declared `m_DataLastUpdated` as `protected`; the old one
did not, which is what broke. When editing visibility in one, mirror it.

## Deleting a module can break a dependent mod

Visibility is not the only way to break `modded class`. **Removing a type breaks
it too** — and the module does not have to be one anyone still uses.

COT's ItemSetSpawner module was deleted in `893b62e5`. Expansion still ships
`modded class JMItemSetSettings` behind `#ifdef JM_COT`, so the whole Mission
module stopped compiling:

```
dayzexpansion\classes\modules\itemsets\jmitemsetsettings.c(14):
Unknown type 'JMItemSetSettings'
```

Note the failure is in *Expansion's* file, for a type *COT* removed, and it
takes the entire server down — not just the item-set feature.

`JM/COT/Scripts/5_Mission/.../Compat/JMItemSetCompat.c` restores the three types
Expansion touches (`JMItemSetSettings`, `JMItemSetSerialize`,
`JMItemSetItemInfo`) as **data carriers only**. No module, no form, no RPC, no
persistence — `Load`/`Save` are deliberately absent because they depended on
`JMConstants.DIR_ITEMS` / `EXT_ITEM`, which were removed alongside the module.
Expansion's `Defaults()` override populates `ItemSets`; nothing drains it.

Its constructor is `protected`, not `private` as the original was — a modded
class cannot reach a private **constructor** either.

> **Before deleting a module**, grep the dependent mod for every type it
> declares. If a mod mods it, leave stub types behind.

## The reverse direction: COT calling into Expansion

The cases above are Expansion reaching into COT. The dependency also runs the
other way, and it breaks the same build.

`JMTerritoryModule` called two `ExpansionTerritoryModule` methods that do not
exist in Expansion at all:

```
jmterritorymodule.c(169): Undefined function
'ExpansionTerritoryModule.Exec_AdminSetTerritoryLevel'
```

`JM/COT/Scripts/5_Mission/.../Compat/JMTerritoryCompat.c` adds them via
`modded class ExpansionTerritoryModule`, implemented against Expansion's real
API. Guarded on `EXPANSIONMODBASEBUILDING` to match the caller, since Expansion
is **not** in COT's `requiredAddons` and COT must still build standalone.

**A `modded class` must be declared in the same layer as its base.** Both
`ExpansionTerritory` and `ExpansionTerritoryModule` are 4_World, so the shim
lives in `4_World/CommunityOnlineTools/Compat/`, not beside the 5_Mission module
that calls it. Getting this wrong fails with `Unknown type 'ExpansionTerritory'`
— the type is simply not visible from the later layer. The call direction is
unaffected: `JMTerritoryModule` (5_Mission) still reaches the shim, because
later layers see earlier ones, not the reverse. COT's other Expansion-modding
files (`ExpansionAirdrop_COT.c`, `ExpansionVehicleCover_COT.c`) all sit in
4_World for the same reason.

Three traps this hit, worth knowing before writing similar code:

- **`m_Territories` is client-only.** Its own comment says *"Contains only
  territories which a client is member of"*, and `Exec_UpdateClient` is its
  only writer — it is empty on the server, where admin paths run. The
  server-side source is `GetAllTerritoryFlags()`, marked *"Called server side"*.
- **`Send_UpdateClient` is `private`** — unreachable from a modded class, the
  same rule as above but pointing the other way. Use the public
  `UpdateClient(territoryID)` wrapper, which walks the member list and guards
  on `IsMissionHost()` itself.
- **`ExpansionTerritory.TerritoryLevel` is `protected` with a getter and no
  setter**, so the shim adds `JM_COT_SetTerritoryLevel`. No explicit save is
  needed — the level rides the flag entity through
  `TerritoryFlag.CF_OnStoreSave` → `ExpansionTerritory.OnStoreSave`.

> **Before calling an Expansion method, grep Expansion for it.** COT's own
> build packs the PBO without compiling against Expansion, so a call to a
> method that does not exist builds green here and fails only at server start.

Note Expansion already ships its own `ExpansionCOTTerritoriesModule:
JMRenderableModuleBase` under `#ifdef JM_COT`. COT's Territory module overlaps
it; worth resolving before either grows further.

## Methods COT provides for mod compat

### `JMPermissionManager.IsAdminToolsToggledOn()`

Expansion calls this from ~25 sites (recipes, user actions, territory). It
supplies its own via `modded class` under `#ifdef JM_COT`, but COT defines it
too so the symbol resolves even if Expansion's modded class is absent,
load-ordered differently, or its COT block compiles out. Both return
`GetCommunityOnlineToolsBase().IsActive()` — identical behaviour either way.

### `JMPermissionManager.RegisterPermission(string, JMPermissionType)`

Expansion carried `Expansion_RegisterPermission` — a verbatim copy of COT's
`RegisterPermission` body, duplicated *only* to pass a `permType`. That copy is
why it needed `m_MissionLoaded`. The overload lets mods call
`RegisterPermission(name, type)` directly instead of reimplementing against COT
internals. The single-arg form now delegates to it, so there is one code path.

## What Expansion extends (verified compatible)

Base classes: `JMRenderableModuleBase` (×4), `JMFormBase` (×4), `JMESPMeta`
(×3), `JMModuleBase` (×1).

`UIActionManager` calls — all match current signatures including arg order:

- `CreateButton(parent, label, instance, funcname)` — 4-arg form, `width` defaults
- `CreatePanel(parent, color, height)`
- `CreateText(parent, label, text)`
- `CreateEditableText(parent, label, instance, funcname[, text, button])`
- `CreateDropdownBox(parent, dropDownParent, label, values, instance, funcname)`

Permissions: `RegisterPermission`, `HasPermission` (all 3 overloads),
`GetPlayer`, `GetClientPlayer`, `GetPlayers`, `JM_GetSelected().GetPlayers`.

## Before you change COT's API

1. **Never narrow visibility** on anything in the table above.
2. **Never reorder or remove `UIActionManager.Create*` params** — add new ones
   with defaults at the end instead.
3. Re-run the audit below after touching any class Expansion mods.

### Automated: `expansion_compat_check.py`

```
py JM/COT/ai/tools/expansion_compat_check.py
```

Checks both directions and exits non-zero on findings, so it can gate a build.
Set `DAYZ_EXPANSION` / `DAYZ_SCRIPTS` if the checkouts are not beside the repo /
at `~/Documents/DayZ Projects/scripts`; it errors rather than reporting a false
clean when a tree is missing.

It checks three things: (A) types Expansion mods exist in COT, (B) Expansion
methods COT calls exist, and (C) every COT `modded class Expansion*` sits in its
base class's layer.

Four things it gets right that a naive grep does not:

- **Vanilla is on the search path.** Expansion classes inherit `EntityAI` /
  `ItemBase`, so `IsSetForDeletion` and `GetAttachmentByType` resolve from
  vanilla DayZ, not Expansion's tree. Without vanilla, every inherited call
  reports as missing.
- **Expansion often defines what it calls.** `EXP_GetMeta` is declared in
  Expansion's own `modded class JMESPModule` — it needs nothing from COT.
- **`#ifdef` guards decide what is reachable.** Only flags in COT's
  `config.cpp` `defines[]` are live, and commented-out entries there
  (`//"JM_COT_LOGGING"`) are not defines. Findings behind a dead flag are
  listed as `inert`, not breakage.

Verified by hiding `modules/Compat/` and confirming it re-finds all three real
bugs (`JMItemSetSettings`, `Exec_AdminSetTerritoryLevel`,
`JM_COT_GetTerritories`). Expected output now: no active breakage, plus the
known `inert JMVehiclesMenu`.

### Manual: every class a mod extends, checked two ways

Derives the class list from the mod itself rather than a hardcoded list, so a
newly-modded class is never silently skipped. Checks **existence** first, then
private-member access — a missing type and a private member break the compile
just the same.

```bash
EXP="/path/to/DayZ-Expansion"
for cls in $(grep -rhoE "modded class JM[A-Za-z]*" --include="*.c" "$EXP" \
             | awk '{print $3}' | sort -u); do
  # Existence: `class X`, `class X: Base`, or `modded class X` anywhere in COT.
  f=$(grep -rlE "^[[:space:]]*(modded )?class ${cls}\b" --include="*.c" JM/COT/Scripts/ | head -1)
  if [ -z "$f" ]; then
    echo "!! $cls does NOT exist in COT — mod cannot compile (deleted module?)"
    continue
  fi
  grep -oE "^[[:space:]]*private[[:space:]].*" "$f" | grep -oE "m_[A-Za-z0-9_]+" | sort -u > /tmp/p.txt
  files=$(grep -rl "modded class $cls\b" --include="*.c" "$EXP")
  [ -z "$files" ] && continue
  hits=""
  while read -r m; do
    [ -z "$m" ] && continue
    echo "$files" | xargs grep -hoE "\b$m\b" 2>/dev/null | head -1 | grep -q . && hits="$hits $m"
  done < /tmp/p.txt
  [ -n "$hits" ] && echo "!! $cls private members used by mod:$hits" || echo "ok $cls"
done
```

A `!!` line means that mod will fail to compile — *if* the block is active.
Widen the member to `protected`, or add a stub type.

Two caveats before acting on a hit:

1. **Check the `#ifdef` guard first.** Only blocks guarded by `JM_COT` (in
   COT's `config.cpp` `defines[]`) actually compile. `JMVehiclesMenu` reports
   `!!` and is *expected* — Expansion guards it with `JM_COT_VEHICLEMANAGER`,
   which COT does not define, so the block compiles out and the missing type is
   never referenced. Defining that flag would break the build: it needs a type
   COT lacks *and* does `override private void GetVehicleTypeInfo`.
2. **Member matching is textual.** Confirm each hit is a real access on the
   modded class — `JMPlayerInstance.m_Stamina` (`float` stat) and
   `JMPlayerForm.m_Stamina` (`UIActionSlider`) are different members sharing a
   name. Don't widen by name-matching.

Current expected output: `ok` for all 15 active classes, plus the known
`!! JMVehiclesMenu` above.

## Findings written: 2026-07-15