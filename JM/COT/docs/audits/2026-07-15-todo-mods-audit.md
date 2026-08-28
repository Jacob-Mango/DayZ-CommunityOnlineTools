# Audit: third-party COT mods in `TODO/`

Eight community mods that extend or fork COT, audited for capabilities COT lacks
and techniques COT does worse. Claims below marked **[verified]** were checked
directly against COT source; the rest are agent findings worth confirming before
acting.

## Headline

Most of these mods are **older than current COT and would not compile against
it** — they were forked from, or written against, a COT API that has since moved.
Three (`ZenCOT`, `Mu_XMLEditor`, `COT_PvPInspector`) have already been partly
absorbed into COT. So the value here is **ideas, not code**: nothing should be
merged wholesale.

Five findings are worth acting on. Two are COT bugs these mods expose.

---

## COT bugs found

### 1. `JMEntityTracker` only tracks `ItemBase` **[verified — but narrower than it looks]** — FIXED 2026-07-16

`JMEntityTracker.Register` had exactly one call site — `4_World/.../Entities/ItemBase.c:6`.

ZenCOT registers six types (`EntityRegister.c:18-119`). **On investigation only
two of those were worth adding**, and the audit's original framing was wrong:

- `BaseBuildingBase extends ItemBase` (vanilla `basebuildingbase.c:2`) — player-built
  structures were **already tracked**.
- `BuildingBase` is *static map houses*, tens of thousands of them, which never
  change. Registering them would bloat the tracker for no gain. **Deliberately
  not added.**
- `CarScript`/`BoatScript` already have purpose-built `s_JM_AllCars` /
  `s_JM_AllBoats` linked lists that the Vehicles module uses. Adding them to the
  tracker as well would double-track. **Deliberately not added.**

Fixed by registering `AnimalBase` and `ZombieBase` only — genuinely absent,
genuinely dynamic. Note the tracker's only consumer is LootAnalysis, which
searches loot (i.e. `ItemBase`), so the practical impact of this gap was small.

### 2. `JMEntityTracker.Unregister` corrupts its own count **[verified]** — FIXED 2026-07-16

`Classes/JMEntityTracker.c:26-46` — `m_EntityCount--` sat **outside** the
`m_EntitiesByType.Find(t, bucket)` block, so it decremented even when the entity
was never registered or the bucket was already gone. `GetTotalEntityCount()`
drifted permanently. Pre-existing; unrelated to these mods.

Fixed: decrement only on a confirmed bucket removal. `Register` also now guards
against double-insert, which `EEInit` makes reachable.

### 3. Tracker hooks the constructor/destructor, not `EEInit`/`EEDelete` **[verified]** — FIXED 2026-07-16

`4_World/.../Entities/ItemBase.c:3-13` registered in `void ItemBase()` and
unregistered in `~ItemBase()`. Enforce destructors fire on **GC**, not on
deletion, so unregistration lags and the tracker serves stale entities.
`EEInit`/`EEDelete` (ZenCOT `EntityRegister.c:3-15`) are deterministic.

### 4. LootAnalysis silently misses all modded loot **[verified]** — FIXED 2026-07-16

`JMLootAnalysisModule.c:151` hardcoded `CF_XML.ReadDocument("$mission:db/types.xml", ...)`.
There were **no references to `cfgeconomycore.xml` anywhere in COT**, so every
modded/custom-map types file was invisible to the analysis.

Fixed via `Server_FindRegisteredTypesFiles`, which walks `cfgeconomycore.xml`
for `<ce folder="X"><file name="Y" type="types"/>` and parses each discovered
file. Used COT's existing CF_XML DOM rather than Mu_XMLEditor's line-based
reader (`JMXMLEditorModule.c:177-210`), which breaks when `<ce folder=` and
`<file>` share a line. Entries use `Set` not `Insert`, so a modded file
correctly overrides a vanilla entry.

### 5. AntiCheat drops every explosive, mine and roadkill kill **[verified]** — FIXED 2026-07-16

`JMAntiCheatKillHook.c:35-37` does `PlayerBase.Cast(killer)` and returns on
null, commented *"environmental death (fall, zombie, fire) — skip"*. But
grenades, claymores, mines and vehicles are **not** players, so those kills are
silently dropped from AntiCheat stats too.

`COT_PvPInspector` solves this by tagging an owner onto `ItemBase` (the common
ancestor of `ExplosivesBase` and `TrapBase`) at three hook points:
`OnInventoryExit` (thrown, `KF_ItemBase.c:65`), `OnPlacementComplete` (placed,
`:74`), and `TrapBase.StartActivate` (`KF_TrapBase.c:5`), gated by
`KF_IsTrackable()` (`:17`) so normal items cost nothing. Shooter resolution
walks `GetHierarchyParent()` → direct cast → `GetHierarchyRootPlayer()`
(`KF_PlayerBase.c:62-88`); vehicle kills attribute to `VEHICLESEAT_DRIVER`
(`:214-243`).

Related: `JMAntiCheatKillHook.c:44` computed kill distance **at death**, so a
victim who bled out 90s later logged a garbage distance. PvPInspector snapshots
state at `EEHitBy` and uses it if within a 120s window
(`KF_PlayerBase.c:248-305`).

Fixed by both: a `ResolveKiller(Object)` helper (tagged explosive/trap owner →
`GetHierarchyRootPlayer` → `GetHierarchyParent` → `Transport.CrewMember(DRIVER)`),
plus a hit-time snapshot on `PlayerBase` consumed by `OnPlayerKilled` within
`SNAPSHOT_WINDOW_MS` = 120s. Owner tagging lives on `ItemBase`
(`OnInventoryExit` for throws, `OnPlacementComplete` for placement) and
`TrapBase.StartActivate` for traps armed in place, all gated behind
`COT_IsKillTrackable()` so normal items cost nothing.

**`OnPlayerHit` had the same bug** — it also did a bare `PlayerBase.Cast(source)`,
so *every ranged hit* was dropped too, not just kills (the source of a shot is
the weapon). Fixed by the same helper.

---

## Capabilities COT lacks

| Capability | Mod | Verdict |
|---|---|---|
| **Spatial clustering of large entity sets** | `COT_BaseBuildingManager` `JMBuildGroup.c:85-132` | **Take the idea.** Greedy clustering collapses thousands of base parts into a few dozen clickable zones, with avg health / remaining-lifetime rollups (`:24-60`). COT has no aggregation anywhere — Vehicles and ESP render one row per entity. Move server-side and grid-bucket it; theirs is O(n²) client-side on a 10s refresh. |
| **Base-building inventory** | `COT_BaseBuildingManager` | Real gap. Note the `TentBase.PITCHED` filter (`JMBuildingsModule.c:195`) and `GetLifetime()` = *remaining* days, not `GetLifetimeMax()` (`:60`) — exactly what admins want for cleanup, and COT surfaces it nowhere. |
| **types.xml live editor** | `Mu_XMLEditor` | Real gap, see below. |
| **Settings hot-reload** | `Expansion_ConfigReload` | Real gap, see below. |
| **Persisted, browsable kill log** | `COT_PvPInspector` | COT only fires Discord webhooks (`PluginAdminLog.c:55-67`) — nothing queryable. |
| **LoS raycast + aim-angle forensics** | `COT_PvPInspector` `KF_Visibility.c:46-108` | Wallbang / no-aim detection COT completely lacks. Note the earned insight at `:88-90`: **zero the Y component** — server-side pitch is unreliable, yaw is not. |
| **Cargo-preserving vehicle restore** | `COT_VehicleManager` `JMVehiclesModule.c:121-169` | Snapshot cargo refs *before* `COT_OnDebugSpawn`, then delete only what wasn't there — restores default parts without nuking player loot. |
| **`cfgeventspawns.xml` fleet cleanup** | `COT_VehicleManager` `:316-396` | COT has zero `cfgeventspawns` references. Purges "fresh spawn parked on its own spawn point". Preview-before-delete UX (`:207-216`) beats COT's fire-and-forget. |

### XML writing: the line-preserving insight

COT uses `CF_XML` (`JMLootAnalysisModule.c:150`), a proper DOM — but it is
**read-only, and writing through it would reformat the file and destroy
comments**. `Mu_XMLEditor`'s `JMXMLParser.c` is instead a *line-preserving
buffer* (`:53`) that mutates only the substring between `>` and `</tag>`
(`:190-193`), leaving whitespace, comments and ordering byte-identical.

**For editing server config in place, that is the correct architecture.** If COT
ever writes XML, use this pattern, not CF_XML.

Pair it with timestamped backup-before-write (`JMXMLEditorModule.c:433-450`):
`MakeDirectory` + `CopyFile` to `<name>_YYYY-MM-DD_HH-MM-SS.xml`. Recommended
for **any** COT feature that writes server files.

### Settings hot-reload: the five-step pattern

COT has **no** settings hot-reload. `JMConstants.DIR_*` JSON is load-once-at-boot.
The closest is `JMTeleportModule.c:250` (`m_Settings = null; Load();`), which
works only because teleport settings are a leaf nothing else holds a ref to.

The blocker is architectural: `JMWeatherSerialize.Load()` (`:10`) and friends are
**static factories returning a new instance**, so callers cache the pointer and
re-calling orphans stale references. Safe reload requires instance `Reload()`
methods first.

`Expansion_ConfigReload`'s pattern (`expansion_configreload_settings.c:18-43`):

1. Clear dependent caches *before* teardown
2. `Unload()`
3. **Evict from every registry** — both the map and the ordered array
   (`:31-32`); forgetting the second leaves a dangling entry
4. Re-`Init(T, registerClientRpc)`
5. `Load()` from disk, then **push to every connected client** — server-side
   reload alone leaves clients stale

Plus `expansion_configreload_setting_helper.c:18`: capture the RPC-registration
flag *before* teardown and restore it, so reload never silently changes whether
clients get RPC'd.

No mod watches the disk — Enforce exposes no mtime API. Admin-pull is the right
call; COT should make the same one.

---

## Also worth adopting

- **Parent-permission inheritance.** `expansion_configreload_permissions.c:49-54`
  checks `A.B.C`, then falls back to `A.B`. COT's `HasPermission` is exact-string
  only.
- **JSON load-failure handling.** COT calls `JsonLoadFile` bare with no failure
  branch (`JMWeatherSerialize.c:16`) — malformed JSON silently yields a
  default-constructed object. Compare `expansion_configreload_admins.c:39-51`
  (dual parser + report expected schema on failure).
- **Batched multi-ID delete RPC** (`JMBuildingsModule.c:365-427`) — count + N
  network-ID pairs in one RPC, resolve server-side, collect-then-delete. Good
  model for COT bulk actions.
- **Collect-then-delete** (`COT_AnimalManager JMAnimalsModule.c:403-431`) to
  avoid invalidating the linked-list iterator during bulk delete, with
  `d[1] = 0` for horizontal-only distance. Confirm COT's own `Exec_Delete*`
  loops advance the node *before* deleting (`COT_VehicleManager` does, `:226`).
- **`modded class JMModuleConstructor` is the extension contract.**
  `expansion_configreload_cot_register.c:1-11` registers a module from an
  external mod with **zero COT-side edits**. Worth documenting officially.

---

## Do NOT port

- **Per-marker `Timer` at 0.01s (100 Hz)** — `JMAnimalsMapMarker.c:129-137`,
  `JMBuildingsMapMarker.c:125-135`. COT **already removed this**:
  `JMVehiclesForm.c:684-696` uses one form-level ticker, and
  `JMVehiclesMapMarker.c:86-102` replaced stop/start-on-hover with a cheap
  `m_Frozen` bool. Porting these would reintroduce a regression COT fixed.
- **Unbounded single-RPC full-list sends** — `JMAnimalsModule.c:230-236`,
  `JMBuildingsModule.c:271-277`. Will blow the RPC size limit at production
  entity counts. No mod here has pagination.
- **PvPInspector's entire ban subsystem** — `KF_JMPlayerBan.c:9-12` mods
  `JMPlayerBan` to add `SteamID`, which current COT **already has**
  (`3_Game/.../Bans/JMPlayerBan.c:17`) along with `PlayerName`, `BannedAt`,
  `IssuedBy`, `IssuedByName`. COT also migrated to a single `Bans.json` store and
  **`JMPlayerBan.Save()` no longer exists**. COT's model is strictly better.
- **`KF_Time.c`** — superseded by `JMPlayerBan.GetExpiryString()` (`:59-86`),
  which is better (COT emits "3d 2h 15m" with a `< 1m` floor; `KF_Time`
  hardcodes French and drops minutes when days > 0).
- **`KF_Store.Save()`** (`:48-62`) — rewrites the entire 500-record JSON
  synchronously **on every kill**. Per-death disk stall. Buffer and flush on a
  timer if porting the kill log.
- **`ZenCOT_AdminBuildState`** (`JMObjectSpawnerModule.c:143`) — keys a
  `map<Object,int>` by pointer and never removes entries. Leaks, and collides on
  pointer reuse after deletion.
- **AnimalManager's module** — ESP (`JMESPViewTypeAnimal`) largely covers it, and
  it's a fork of an older Vehicles module still carrying un-renamed vehicle
  widget names (`JMAnimalsMenu.c:62-92` — `FindAnyWidget("vehicles_list_panel")`).

---

## Note on `COT_Override`

No scripts — a binarised 455-byte `config.bin` only. It's a keybind-override
shell: `requiredAddons[] = {"JM_COT_Scripts"}` forces it to load after COT so its
own `Inputs.xml` supersedes COT's 33 `UACOT*` actions. That's the sanctioned way
to re-map COT defaults without forking.

Two defects: **the `Inputs.xml` it references does not exist** (`COT_Override/Scripts/`
is absent entirely — the mod currently overrides nothing), and `hideName=0`
while COT sets both `hideName`/`hidePicture` to `1` (`config.cpp:24-25`), so it
would show in the client mod list while COT stays hidden.

---

## Suggested order

1. Fix `JMEntityTracker.Unregister` count bug — one line, real corruption.
2. Extend tracker registration beyond `ItemBase`; move hooks to `EEInit`/`EEDelete`.
3. Fix LootAnalysis to discover all types files via `cfgeconomycore.xml`.
4. Fix AntiCheat kill attribution (explosives/traps/vehicles) + hit-time snapshot.
5. Spatial clustering for large lists — server-side, grid-bucketed.
6. Then the net-new features: types.xml editor, settings reload, kill log,
   base-building module.

Audited: 2026-07-15
