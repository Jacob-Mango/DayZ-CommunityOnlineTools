# Cleanup playbook

How `JMPlayerForm` (3,279 lines, 335 members) was cleaned up, and how to give any other form, module or
tab class the same treatment. Everything here was learned on that file; the worked example is at the end.

The tools live in [`Workbench/Batchfiles/cleanup/`](../../Workbench/Batchfiles/cleanup/). Run them from the
repo root (`JM/COT`), e.g. `python Workbench/Batchfiles/cleanup/reorder.py Scripts --apply`.

## 1. The target shape

These are the rules the cleanup enforces. They are also the review checklist for new code.

### Folder layout

The full rule, with examples and the definition of a component, is in [module-folder-layout.md](module-folder-layout.md). Summary:

A module folder holds the module, the form, the classes several tabs share, and sub folders for tabs and
components:

```
modules/Player/
    JMPlayerModule.c  JMPlayerForm.c  JMPlayerRowWidget.c ...     module, form, shared plain classes
    Components/       JMPlayerRoster.c  JMPlayerRosterFilter.c  JMPlayerRowMenu.c
                      JMPlayerStat.c  JMPlayerStats.c  JMPlayerToggle.c  JMPlayerToggles.c
    Tabs/
        General/      JMPlayerFormTabGeneral.c  JMPlayerStatRow.c
        Inventory/    JMPlayerFormTabInventory.c
        Actions/      JMPlayerFormTabActions.c
        ...
modules/Vehicles/Tabs/Map/   ... Components/ appears here when a component serves only that tab
```

- `Tabs/<Name>/` holds the tab class (`<Prefix>FormTab<Name>.c`) and every class only that tab uses: row
  widgets, popups, hover panels, map markers.
- A **component** is a part the form, the module or a tab is built from: a plain class that keeps a
  back-reference (`m_Form`, `m_Tab`, `m_Owner`, `m_Parent`) to the form or tab it serves (see 3.2), or a class
  family kept in the module folder - a base class plus its subclasses, like `JMPlayerStat` with the built-in
  stats and `JMPlayerToggle` with the built-in toggles. It goes in `Components/` next to the form when the form, the module or
  several tabs use it, and in `Tabs/<Name>/Components/` when one tab does.
- A class that the form, the module, another tab, another module, `ExampleScriptOverride` or `AutoTest` also
  names is shared and stays in the module folder. When a second tab starts using a helper, move it up.
- Layouts and stringtable keys stay where they are - they belong to other addons and are found by path/key.

`organize_tabs.py` (4.8) sorts a tree into this shape and can be re-run at any time.

### Class layout

Top to bottom, in this order:

1. Fields, all of them, in their original relative order, except that `static` / `static const` fields come first,
   then one blank line, then the member fields (see [conventions.md](conventions.md)).
2. Constructors (`void ClassName(...)` - Enforce writes constructors with a `void`).
3. Destructor.
4. Getters (`Get*`, `Is*`, `Has*`), sorted A-Z, case-insensitive.
5. Setters (`Set*`), sorted A-Z.
6. Everything else, in its original order (overrides, event handlers, helpers).

- An `override` is never a getter or setter, whatever its name. `Is*`/`Has*` methods that are really logic
  still land in the getter group - the classification is by name only.
- A top-level `#if ... #endif` block inside a class is one unit. It moves to the fields when everything in
  it is a field; otherwise it stays in the "everything else" group.
- Fields with no doc comment sit directly under each other, no blank line. A blank line goes only before a
  field that has its own `//!` comment, and between the static group and the member group.
  (Neither `reorder.py` nor `compact.py` implements the static-first grouping or the static/member blank line yet - they keep fields in
  their original order; do it by hand. See [conventions.md](conventions.md).)

### Ownership

- **A tab's variables live in the tab class.** That covers its widgets, its constants (`INV_*`, `MAP_*`,
  `IDENTITY_*`), its layout numbers and its panel size (`static const int PANEL_ROWS`, which the form reads
  when it creates the panel). A form keeps only what every tab shares: the tab strip, the scroller the
  panels sit in, the selection, the module, and the tab controllers.
- **Tab ids come from the tab manager, never from a form.** `m_TabIdX = m_Tabs.AddTab( ... )`, and inside a
  tab `GetTabId()`. No `static const int TAB_X = 0`, no `SetSelection( 2 )`, no `IsTabActive( 1 )`. Another
  mod may add a tab to the same strip. See `naming.md`, "Adding and removing things".
- **A constant lives on the class that reads it.** Check who reads it before moving it: `SHOCK_CONSCIOUS`,
  `MAP_HEIGHT` and the two `IDENTITY_*` widths looked shared, but every other form had its own copy.
- **A form orchestrates; it does not implement.** A form owns selection, tab wiring and the module. A
  self-contained region of it - the player list, a filter menu, a right-click menu - is a component class
  the form holds by `ref`.

### Things that must not change

- **Doc comments are never dropped.** Comments move with the member they sit above. This was an explicit
  complaint about an earlier cleanup ("Stop removing documentation comments"); the tools verify it: every character of a class, comments included, must be present after the move (see 4.1).
- **Extension points stay on the form** even when the work moves out (see 3.4).
- **Comments and identifiers are ASCII.** 15 comments in five files were mojibake (`// ??? Title ????`) from
  box-drawing characters; they became `// --- Title -----`. Do not paste box-drawing characters.

## 2. Procedure

Do these in order. Verify (section 5) after every numbered step, not at the end.

1. **Back up.** The cleanup ran on an uncommitted tree, so git could not undo it. Copy `Scripts`,
   `ExampleScriptOverride`, `AutoTest` and `languagecore` somewhere outside the repo first. Better: commit
   the working state before starting, so the cleanup is its own diff.
2. **Audit the file.** `audit_members.py <file> <class>` prints the member table (line, size, kind) and, per
   field, how often it is used in its own file versus elsewhere. Read the biggest methods and the runs of
   5-8 line methods (those are shims).
3. **Mechanical passes, whole tree** (each is idempotent and safe to re-run):
   - `reorder.py Scripts ExampleScriptOverride AutoTest --apply` - the layout in 1.
   - `compact.py Scripts ExampleScriptOverride AutoTest --apply` - the field spacing rule.
4. **Move tab-owned variables** into their tab (script the move, see 4.3), rewriting `JMForm.NAME` to `NAME`
   in the receiving class.
5. **Extract components** for self-contained regions (see 3). Move whole members, doc comments included;
   rewrite receivers; leave forwarders for anything external code calls.
6. **Re-run 3**, so new members land in the right group.
7. **Verify, then write down what is now removed** in `deprecations.md` (a `static const` cannot forward or
   warn, so a moved constant is a break).

## 3. Design lessons

### 3.1 Find the shims first

`JMPlayerForm` was 3,279 lines mainly because ~120 methods of 5-8 lines each only forwarded to a tab
(`if ( m_TabActionsCtrl ) m_TabActionsCtrl.Click_HealPlayer( eid, action );`), about 890 lines. Two causes:

- **Compatibility.** The form used to own the tab's methods; a sub-mod may still override them. These carry
  `JMDeprecated.WarnOnce` and are documented in `deprecations.md`. They cannot be deleted before the next
  major release, but they can be moved into one file (`modded class JMPlayerForm` in the same layer) so the
  main file is not buried in them.
- **A design gap.** `ConfirmAction`, `PromptInput`, `CreateAdvancedPlayerConfirm` and
  `CreateConfirmation_Three` resolve their string callbacks **on the form**, so a tab that raises a
  confirmation needs a same-named method on the form. About 45 of the 120 shims exist only for this.
  Giving those helpers a target-object parameter lets the tab pass `this` and removes the shims. This has
  **not been done**.

To classify a shim as dead or live, grep for the string literal (`"Name"`) and for `.Name(` calls, and also
check who the callback **target** is (`this` in the tab means the form's copy is dead). Do not count the
`JMDeprecated` message text or `m_Tab...Ctrl.Name(` inside the form itself - those inflate the count. A first
attempt at this scan reported every shim as live for exactly that reason.

### 3.2 Component boundaries that worked

| Component | Owns | Reaches the form through |
|---|---|---|
| `JMPlayerRoster` | left panel widgets, row pool, sort, role grouping and fold animation, counters, select-all, the two selection-preset slots | `m_Form` |
| `JMPlayerRosterFilter` | filter button, dropdown, the six "show ..." switches | `m_Form`, `m_Roster` |
| `JMPlayerRowMenu` | right-click menu, message prompt, status repairs | `m_Form` |
| `JMWeatherModeBanner` | the mode banner: icon, text, hover, click | `m_Form` |
| `JMESPViewTypeWidget` (existing) | one view-type row; now takes the Filters tab, not the form | the tab |
| `JMTeleportFilter` | category filter button, menu pages, the `SUB_*` / `FILTER_*` ids | `m_Form` |
| `JMTeleportLocationPopup` | save-location popup and its category picker | `m_Form` |
| `JMTeleportMapOverlays` | player/vehicle/custom markers drawn on the map | `m_Form` |
| `JMLootMarkerMenu` | right-click menu on a map marker, shared by both maps | `m_Form` |
| `JMVehiclesTypeFilter` | type filter button, dropdown, shown-types bitmask | `m_Form` |
| `JMObjectSpawnerProperties` | the property card: per-class rows, the values the admin picked, range toggle | `m_Form` |
| `JMCameraTravelPanel` | the Traveling panel: waypoints, playback, saved paths | `m_Form` |

Sizes after the pass (lines): `JMWeatherForm` 737, `JMESPForm` 435, `JMLootAnalysisForm` 632, `JMVehiclesForm` 751,
`JMObjectSpawnerForm` 2,003 -> 1,047, `JMCameraForm` 1,228 -> 525. On 2026-09-21 the first four stood at 793, 438, 651 and
745 (the Weather form grew with the dynamic-weather work; the Presets tab and its helpers are separate files).

Guidelines:

- A component takes the form in its constructor and keeps it as a plain (non-`ref`) member; the form holds the
  component by `ref`. That avoids a reference cycle.
- Components are plain classes, not `JMFormTab`s: they have no tab panel of their own.
- Keep selection state (`m_SelectedInstance`, `m_LastSelectedGuid`) on the form; the tabs already read it there.
- A UI callback registered with a string (`SetOnClick( this, "Name" )`) resolves on the target object. When
  code moves to a component, the component becomes the target and the method moves with it. Only callbacks
  that a `JMFormBase` helper resolves on the form (3.1) need a forwarder left behind.
- Do not fold selection-preset dialogs into their own class: the dialogs call back by name on the form, so
  the class would only add forwarders.

### 3.3 Fields worth a second look

- A field with 0 uses in its own file and several elsewhere belongs to the file that uses it.
- A field used once, to create a widget and hand it to `AddTab`, is a local. The five panel widgets on
  `JMPlayerForm` were exactly this.
- Compatibility mirrors stay. `m_ApplyStats`, `m_Stamina` and `m_RefreshStats` are documented in their own
  comments as members DayZ-Expansion's `modded class JMPlayerForm` names; removing them breaks Expansion.
  Read the comment before deleting a field that looks unused.

### 3.4 Extension points are API

The examples in `ExampleScriptOverride/` are the public API contract. On `JMPlayerForm` they read
`m_PlayerMenu`, `m_PlayerMenuGUID`, `OnPlayerRow_RightClick`, `PassesListFilters` (protected) and
`UpdatePlayerList`. After extraction those all still exist on the form:

- The form's `OnPlayerRow_RightClick` delegates to the row menu, which calls `m_Form.SetPlayerMenu( menu,
  guid )` each time it opens so `m_PlayerMenu` keeps its old meaning.
- `PassesListFilters` stays `protected` and overridable, and delegates to the filter. The roster is another
  class and cannot call a protected method, so a public one-line `FilterPlayer( player )` wraps it.

Before moving a member, `grep` the examples, `docs/systems/mod-compatibility.md` and the field comments.
Deliberately-`protected` fields are called out in comments ("protected, not private: DayZ-Expansion ...").

### 3.5 Duplicate constants and stale copies

Several "shared" constants turned out to have a private copy in the other class
(`JMRoleManagerFormTabPlayers.IDENTITY_*`, `JMTeleportForm.MAP_HEIGHT`, `JMESPActionMenu.SHOCK_CONSCIOUS`).
Search by whole word (`grep -w`) before deciding a constant is shared.

### 3.6 Traps that only the game compiler finds

The lint (`verify-fix`) does not check member lookup or signatures. Each of these passed lint and failed the
real compile, one per build:

- **Inherited members.** `extract_component.py` and `move_members.py` only know the class they cut from.
  A moved method that calls a member of the base class (`JMFormBase`: `m_Window`, `layoutRoot`, `AddOverlay`,
  `RemoveOverlay`, `GetWindow`, `PromptInput`, ...) compiles nowhere else. Route it through `m_Form`
  (`m_Form.AddOverlay( ... )`, `m_Form.GetLayoutRoot()`, `m_Form.GetWindow()`), and add a public getter on the
  form if the member is protected (`GetPreviewItem`, `GetModule`, `GetRightScroller`). `extract_component.py`
  now reports these as `TODO ... uses INHERITED JMFormBase members bare`. Globals with the same name as a
  form method (`GetPermissionsManager()`) are fine bare.
- **Forwarder signatures.** A forwarder that stays on the form must keep the callback signature the caller
  uses (`Click_OpenSavePopup( UIActionBase action )`, `OnClick_CategoryFilter( UIEvent, UIActionBase )`), not
  the new component's. Check every string callback and every call site.
- **Duplicate declarations.** When a component is created for a state the tab already declares (the ESP
  filters tab owned `m_ESPTypeList` already), moving the fields in produces "Multiple declaration of
  variable". Check the target class for the name before moving.
- **Cascades.** `Bad type 'map'`, `Bad type CF_Modules`, `Bad type JsonFileLoader` further down the log are
  follow-ons of the first real error. Fix the first `(E)` in `JM/COT` and rebuild.
- **`GetMousePos`, `GetPermissionsManager`, `JM_GetSelected`** are global functions, usable bare from any class.

The moved code keeps its string callbacks: `CreateOverlayMenu( m_Form, this, "OnClick_X" )`,
`CreateIconButton( parent, icon, this, "OnClick_X" )` - the component is the target, so the handler moves
with it. `CreateOverlayMenu` and `CreateOverlayFilterMenu` take the **form** first and the target second.

Where a form's confirmation chain (`ConfirmAction`, `PromptInput`, `CreateAdvancedPlayerConfirm`) names
callbacks on the form, the chain stays on the form. `JMAntiCheatForm` (`ConfirmClear/Kick/Ban` and
`m_PendingGuid`) and `JMLootAnalysisForm` (`SaveCEData`, `SaveCEData_Yes`, `DeleteAll_Yes`) were left as they
were for that reason.

## 4. The tools

All in `Workbench/Batchfiles/cleanup/`. Python 3, standard library only, no install. `reorder.py` and
`compact.py` need the roots as arguments: `python reorder.py Scripts AutoTest ExampleScriptOverride --apply`
(with no root they do nothing and say `files changed: 0`).

### 4.1 `reorder.py`

Rewrites class member order (layout in 1). Options: `--apply` (otherwise a dry run), `--verbose`.

How it works, so you can trust or fix it:

- Masks comments and string literals, then finds each `class` at brace depth 0 and splits its body into
  units ending at `;` or at the closing `}` of a method. A `;` after `}` (array initialisers) is absorbed.
- Depth-0 `#if...#endif` blocks are single units; other depth-0 `#` lines are single units.
- A comment directly above a member, and any detached comment block above it, travels with it. A `//` on
  the same line after a terminator belongs to the member before it.
- A titled banner (`// --- Section ---`) over a constructor, destructor, getter or setter is not carried
  with it; it is handed to the next "everything else" method so a section label is not stranded over
  the wrong code. Pure rule lines (`// ------`) frame a doc block and stay with their member.
- Skips a class, and reports it, when a unit does not have balanced braces or cannot be classified.

Safety nets (a failure aborts that class or file and prints it):

- the non-whitespace characters of a class must be identical before and after;
- the same for the whole file;
- CRLF and BOM are preserved; the repo runs `core.autocrlf=true`.

Known imperfections:

- Sorting getters A-Z can separate a comment that introduced a *group* of methods from the group. The
  comment is kept, but it may now sit above one member of it.
- Name-based getter/setter detection is a heuristic.

### 4.2 `compact.py`

Removes blank lines between consecutive fields when the second has no comment above it. Same safety nets.
`reorder.py` applies the same spacing rule when it emits a class, so it will not reintroduce it. Neither tool knows the static-first rule, and
`compact.py` collapses the blank line between a static and a member field (see [conventions.md](conventions.md)).

### 4.3 `members.py`

Library for one-off edits: `parse_class` returns a class as records (`kind`, `name`, `text`, `blank`),
`insert_fields` and `replace_class` put them back. The tab-constant move and the component extraction were
each a short script on top of it: cut records by name, paste them into the target class, regex the receivers
(`m_Module` -> `m_Form.m_Module`, `UpdateUI()` -> `m_Form.UpdateUI()`), then hand-edit the constructor and
whatever the regexes could not know. Write those scripts fresh per file; they encode file-specific names.

### 4.4 `audit_members.py`

Read-only member table and per-field usage (see step 2).

### 4.5 `move_members.py`

`python move_members.py spec.json [--apply]` - moves named fields, constants and static helpers from one
class to another (usually a form to the tab that owns them). The spec is a list of
`{from_file, from_class, to_file, to_class, names}`. It cuts the members with their comments, puts fields after
the target's last field and methods at the end, and rewrites `Source.NAME` to `Target.NAME` across `Scripts`,
`ExampleScriptOverride` and `AutoTest`. Bare uses left in the source file are rewritten only for statics; for an
instance member it prints `CHECK bare use ...` and you edit by hand. It also warns when a moved member uses
something that stayed behind. Used for the weather tab constants and the loot map-layer ids.

### 4.6 `extract_component.py`

`python extract_component.py spec.json [--apply]` - cuts a group of members out of a form into a new component
class (see the docstring for the spec: `names`, `rename`, `public`, `rules`, `form_rules`, `doc`). The new class
gets the doc comment, a `protected <Form> m_Form;` field, the moved fields, a constructor, then the moved methods.
Always dry-run first; the `TODO` lines are the work list:

- `uses form members bare: [...]` - members that stayed on the form. Use `rules` to rewrite them
  (`(?<![\w.])m_Module\b` -> `m_Form.GetModule()`) and add the getter.
- `uses INHERITED JMFormBase members bare: [...]` - see 3.6.

After `--apply`, by hand: add the component field to the form, create it in the form constructor, replace the
form's own init code with a `Build(...)` on the component, move constructor initialisers (arrays, defaults and
their comments) into the component constructor, and add forwarders for external callers. Then run
`reorder.py`, `compact.py` and `scan_inherited.py <Class> <file>` (lists a component's bare uses of `JMFormBase`
members, for code that was edited after the extraction).

Write JSON spec files with the editor's Write tool, not a shell heredoc: the Bash tool here halves
backslashes, so a regex like `(?<![\\w.])` arrives as `(?<![\w.])` and is invalid JSON.

### 4.7 `wait_build.sh`

`bash wait_build.sh <epoch>` - waits for the build that started after a moment and prints its real result
(exit 0 = booted with no `JM/COT` errors, 1 = compile failure or COT errors, 2 = timeout). Use it as in 5.

### 4.8 `organize_tabs.py`

`python organize_tabs.py [--apply]` - sorts module folders into `Tabs/<Name>/` and `Components/` (layout in 1).
It scans `Scripts`, `AutoTest` and `ExampleScriptOverride` for every mention of each class in the module folder
(comments ignored). A class belongs to a tab when every file that mentions it is in that tab's group, repeated
until nothing changes so a helper of a helper follows. A component is detected by its back-reference field to a
`...Form...` / `...Tab...` class, or by being part of a base-class-plus-subclasses family in one folder; it is only looked for in `5_Mission` folders that hold a form. The plan is
recomputed from the current tree, so the tool is safe to re-run and does nothing when the layout is already right.

Never moved: files with `modded class` (their load order follows the path) and files with no class. Tracked
files move with `git mv`, so history follows. It prints a `NOTE` for a moved subclass whose base class is in the
same layer.

Moving files changes only the path, but the compiler loads a layer's files in path order, so run a full build
afterwards (5). Four passes (tabs, back-reference components, class families, UI classes) each booted clean; see module-folder-layout.md.

## 5. Verification

Run all of these after each step. None of them exercises the client UI (item 5).

1. `./dz-project-manager.exe verify-fix` - the real lint. Baseline for the tree at the time of writing:
   `0 error(s), 175 advisory(ies)`. The advisory count should not grow.
   - `lint-scripts` can print `[dry-run] ... no operation dispatched` when the AI-agent gate trips. That is
     not a clean result; use `verify-fix`.
2. Make sure no game process is left from the previous boot (`tasklist | grep -i dayz`; `taskkill //F //IM
   DayZDiag_x64.exe`). A server that stays up after a `BOOT_OK` makes the next `build` replay the old verdict
   without building. Then record the time and build, **unpiped** and in the background (a piped or chained
   build gives false `CRASH` verdicts; `--confirm build` is required after enough actions in a session):

   ```
   date +%s > "$TEMP/buildstart.txt"
   ./build.ps1 -NoBump --confirm build          # run_in_background
   bash Workbench/Batchfiles/cleanup/wait_build.sh "$(cat $TEMP/buildstart.txt)"
   ```
3. Trust only a result whose script log is newer than `buildstart.txt`. `.logs/build_verdict.txt`, the harness
   output and the background task's "completed" notice can all belong to the previous run or to a build that
   never started (`Workbench/Logs/CI1.log` empty, no new `script_*.log`). Three wrong verdicts were caught this
   way: a stale `BOOT_OK`, a replayed `COMPILE_FAIL`, and a background task that finished before the game booted.
   Cap the wait (`timeout 240 bash wait_build.sh ...`) so one call never blocks for ten minutes.
4. `Workbench/Batchfiles/CheckRPCRanges.ps1` if any RPC enum was touched.
5. **Client UI is untested.** A dedicated-server boot proves the scripts compile and load. It does not run
   `OnCreate`, widget building, roster updates or menus. After a form refactor, open the form in-game and
   try each tab, the list, the filter, the right-click menu and save/load selection.

## 6. Enforce Script and DayZ gotchas found

The current, fuller list (including runtime-only failures) is [conventions.md](conventions.md) section 11.

- No block scope: variables declared in one `if` branch are visible in the next; use distinct names.
- No ternary, no multi-line `||`/`&&`/`+` expressions.
- Constructors are `void ClassName()`; destructors `void ~ClassName()`. Guard the destructor with
  `if ( !g_Game ) return;`.
- `#define` scope is per file, not per compiled layer (see the header of `JMPlayerForm.c`).
- A `modded class` cannot touch a `private` member; that is why so many fields are `protected`.
- A string callback that names a method that does not exist fails silently. After moving a method, grep the
  quoted name and confirm the target object is the class that now holds it.
- A protected method cannot be called from another class, and overriding it with a different access level is
  best avoided; wrap it with a public method instead (3.4).
- `CallLater( Method, ... )` and `GetUpdateQueue(...).Insert( Method )` bind to an instance: after a move
  they must name the component's method (`m_Roster.UpdateGroupAnimation`), and the `Remove` must match.
- Implicit object-to-string concatenation inside a `Print()` in an override crashes the script compiler
  natively (bisected, and noted in the file). The linter does not catch it.
- `grep -P` fails on this machine's locale; use `-E`, or a script.
- Writing Python into a shell heredoc that contains `\n` or `\t` escapes can arrive with the escapes
  already expanded. Write non-trivial scripts to a file.

## 7. Data checks worth running on any pass

- **Stringtable**: parse `languagecore/stringtable.csv` and check duplicate keys, column counts (16), keys
  referenced in code and layouts but missing, and keys removed but still referenced. Key prefixes built
  by concatenation (`"#STR_COT_PLAYER_MODULE_TOGGLE_" + key`) show up as missing; ignore those.
- **A duplicate key is not always a mistake to merge.** `STR_COT_ESP_MODULE_TT_CATEGORY_FILTER` was defined
  twice with different text because two widgets (the Filters tab search box and the Objects tab filter
  button) were meant to say different things. Merging them dropped the button's text. Check every call site
  before choosing which row to keep, and split into two keys if the sites differ.
- **Recovering deleted text**: the build copies the mod to a Temp folder (`DayZ Projects/Temp/JM/COT/...`) and
  Claude keeps file snapshots; a `grep -rl` of a distinctive phrase over both found the deleted row.
- **Comment retention**: count `//` lines removed versus added per file (`git diff -U0`). A big drop in a file
  that was split is expected only if the sum across the new files is not lower.
- **Permissions and RPCs**: every server RPC handler should call `JMPermissions.HasRPC` (directly or through
  a helper). A heuristic scan for handlers without one is a good triage list, not a verdict - some check in a
  wrapper (`CanServe`, per-entry permission in `JMActionHistory`).

## 8. Working notes

- Prefer small, verified steps to one big rewrite. Each of these was built and booted before the next:
  reorder, spacing, constants, roster/filter/row menu, tab ids, tab-owned variables.
- Grep results are noisy. Print counts and short tables, not raw hits; exclude your own definitions and
  deprecation message strings.
- When a scan disagrees with a hand read, trust the hand read and fix the scan.

## 9. What is left on `JMPlayerForm`, and where to go next

Done on `JMPlayerForm`: 3,279 -> 1,666 lines (still 1,666 on 2026-09-21; `JMPlayerFormDeprecated.c` does not exist yet); 39 constants moved to tabs; roster, filter and row menu
extracted; tab ids and panel sizing owned by the tabs.

Still open:

1. **Move the deprecated shims** (about 500 lines) into `JMPlayerFormDeprecated.c` as `modded class
   JMPlayerForm`.
2. **Give the confirmation helpers a target parameter** in `JMFormBase` (`ConfirmAction`, `PromptInput`,
   `CreateAdvancedPlayerConfirm`, `CreateConfirmation_Three`), then delete the ~45 live shims that exist only
   as callback landing pads.
3. **Client-side smoke test** of everything above (section 5, item 5).

Applied since (each built and booted clean): weather, ESP, teleport, loot analysis, vehicles, object spawner,
camera. Sizes as of 2026-09-21 (lines; re-count with `wc -l` before relying on them):

| File | Lines | Notes |
|---|---|---|
| `JMESPModule.c` | 4,531 | server handlers and client senders in one class; RPC handlers are dispatched by name on the module, so splitting needs a forwarder per RPC |
| `JMPlayerModule.c` | 4,363 | same shape |
| `JMESPActionMenu.c` | 2,437 | `Build*` page builders paired with `Do*` actions; cohesive, cutting it only adds forwarders |
| `JMPlayerFormTabInventory.c` | 2,302 | one tab; the inventory grid and its popup menu are separable |
| `JMTeleportForm.c` | 1,830 | filter, popup and map overlays extracted; the row list, row menu and coordinates card are the next candidates |
| `JMObjectSpawnerForm.c` | 1,047 | property card extracted; the class list, preview and export/spawn modes are still in the form |
| `JMVehiclesModule.c` | 1,915 | not audited |
| `JMWeatherModule.c` | 1,108 | the payload classes live in `JMWeatherPreset.c`; the dynamic state machine is in its own files (see [../modules/weather-dynamic.md](../modules/weather-dynamic.md)) |
| `JMMapEditorForm.c` | 1,123 | not audited; the tick / drag / gizmo state is one candidate (`Tick` is 141 lines) |
| `JMCameraForm.c` | 525 | Traveling extracted; the Effects and Bookmarks panels are the same shape and still inline |

Left as they are on purpose: `JMAntiCheatForm` (`m_PendingGuid` and `ConfirmClear/Kick/Ban` are callbacks the
confirmation helper resolves on the form), the CE-save chain on `JMLootAnalysisForm` (same reason),
`JMExampleForm` (a sample for mod authors).

Recipe for the next file: audit (2), mechanical passes (3), find shims and tab-owned variables (3.1, 3.3),
extract one component at a time, verify after each (5), document removals (2, step 7).
