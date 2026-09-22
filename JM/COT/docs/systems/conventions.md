# Project conventions

The rules for writing, committing and verifying changes in this repo, and the mistakes that already cost time. Read
it before editing scripts. Every "do not" below comes from something that actually happened: a reverted commit, a
build cycle burned on a wrong verdict, a compile error the lint missed. The evidence is named so a rule can be
re-checked instead of trusted.

Related, not repeated here: Enforce Script layout limits in [../ui/limitations.md](../ui/limitations.md); naming grammar
in [naming.md](naming.md); the API contract with other mods in [mod-compatibility.md](mod-compatibility.md); the full
class member order in [../ui/module-form-patterns.md](../ui/module-form-patterns.md) section 15 and
[cleanup-playbook.md](cleanup-playbook.md) section 1.

## Quick list: what not to do

| Do not | Instead | Section |
|---|---|---|
| Delete, shorten or "tidy" a comment | Move it with its member; fix it if it is wrong | 3 |
| Add behaviour nobody asked for (a hook, a report, a flag reset) | Do the change asked for; suggest the extra in words | 4 |
| Reset a flag, or report or log, inside a generic check | A check answers yes or no and nothing else | 4 |
| Register a permission from a form, or on one side only | `DeclarePermissions()` on the module, runs on both sides | 5 |
| Guard a permission call against a `NULL` manager or sender so it quietly answers `false` | Let it fail loudly; pass the sender | 5 |
| Prefix a server-only method that has a client counterpart with `Exec_` | `Server_<Action>` / `Client_<Action>`; `Exec_` is for work that runs on both | naming.md |
| Gate new-version code with `#ifdef DAYZ_1_30` | Gate on the old version: `#ifndef DAYZ_1_29` | 6 |
| Load imagesets or widget styles from script | Register them in `config.cpp` | 7 |
| Edit `Version.hpp` or `JMBuildInfo.c` | Edit the generator; run `build.ps1` | 8 |
| Pipe or chain `build.ps1` | Run it unpiped in the background, then `await-boot` as its own call | 9 |
| Trust a verdict without checking its log time | Compare the script log time with the build start | 9 |
| Call `lint-scripts` clean because it printed nothing | Use `verify-fix`, then a real build | 9 |
| Fix a native crash from a theory | Instrument, reproduce once, then fix | 10 |
| `delete` a widget-owning object | `Widget.Unlink()`, then rebuild | 11 |
| Chain `foreach`, `new X().M()` or a temporary as an argument | Use a local (section 12) | 12 |
| Mix whitespace, moves or renames into a logic commit | Commit each on its own (section 2) | 2 |
| Narrow `protected` to `private`, or remove a public member without a forwarder | See [mod-compatibility.md](mod-compatibility.md) | 13 |

## 1. Class fields: statics first, then a blank line, then members

Static fields (`static`, `static const`) go at the top of the class. One empty line separates them from the
instance (member) fields.

```c
class JMExample
{
	static const int ROW_HEIGHT = 24;
	static ref array<string> s_Names;

	protected string m_Name;
	protected int m_Count;
```

- Applies only when the class has both kinds. No statics: no blank line is added; no members: likewise.
- The existing rule for doc comments still holds inside each group: fields without a `//!` comment sit directly
  under each other, and a field with a `//!` comment gets a blank line above it.
- `reorder.py` / `compact.py` in `Workbench/Batchfiles/cleanup/` do **not** know this rule yet (checked 2026-09-21: neither
  handles `static` fields): `compact.py` collapses the blank line between a static and a member field. Do not run it on files
  without fixing that first.
- Everything in a file follows the code already around it: tabs, spaces inside parentheses (`if ( x )`), brace on its own
  line, and the same comment density. New code that looks different from its neighbours is a review comment.

## 2. Commits: mechanical changes stand alone

Three kinds of change are committed **on their own, with nothing else in the commit**:

| Change | Commit contains |
|---|---|
| Whitespace (indentation, trailing spaces, blank lines, line endings) | whitespace changes only |
| Re-ordering (moving members, methods, fields, files, blocks) | moves only, no edits to the moved code |
| Re-naming (variables, methods, classes, files, keys) | renames and the call-site updates they force, nothing else |

Why: a reviewer can skim a pure whitespace / move / rename diff, and `git blame` and bisect stay usable. Mixed in
with logic changes, the same diff hides real edits.

**When to do them.** Only when the working tree has **no local changes to code**. If there are uncommitted code
edits, finish and commit those first, then do the mechanical change as its own commit. Exception: files that are
new and not yet committed - reformatting, reordering or renaming inside a file that has never been committed
carries no history to protect, so it can be folded into the commit that adds it.

**Check before committing.** `git diff --staged` must show only the one kind of change. For whitespace,
`git diff --staged -w` must be empty. For a move, `git diff --staged --color-moved` should show every hunk as moved.

**Line endings.** The repo runs `core.autocrlf=true` and working files are a mix of CRLF and LF. A script that opens a
file in text mode and rewrites it flips every line and turns a one-line edit into a whole-file diff. Read and write with
`newline=''` and re-apply the file's own line ending (`git diff --stat` after the edit shows it at once).

## 3. Comments are part of the code

- **Never delete or shorten a documentation comment** while editing, moving or "simplifying" code. It travels with
  the member it sits above. A whole-file check is in [cleanup-playbook.md](cleanup-playbook.md) 7 (count `//` lines
  removed versus added).
- **A comment that says to keep it, is kept.** `JMCinematicCamera` has a constant `0.447214` with a comment explaining
  how it is derived and ending "keep this comment INTACT". An edit trimmed it to a bare multiplication; the commit
  that restored it is titled "Stop removing documentation comments" (97c560c7).
- **A comment that is wrong is corrected, not removed.** If the code changed, update the text.
- **A comment written to stop a mistake stays.** `JMPermission.m_Sync` documents "once set to true, may NEVER be set to
  false in the same session" (97e899d1); it exists because an earlier edit did exactly that (section 4).
- `//!` starts a doc comment on a member; `//` is for a note inside a body. Match the neighbours.
- **Comments and identifiers are ASCII.** Box-drawing characters and typographic quotes turned into `???` in 15 comments
  once. Use `// --- Title ---`.

## 4. Do the change asked for, and no more

Two commits exist only to undo helpful additions by an assistant, both in the permission code:

- **A reset nobody asked for (e9374a3d).** `JMPermission.Clear()` was given code that set the `m_Sync` flag back to
  `false` "so callers can detect there are no custom permissions". The flag is set on the root and, by design, never
  goes back in a session. The addition broke that and was removed.
- **A side effect inside a generic check (292dc694).** The `JMPermissionManager.HasPermission` overload was given a call
  that reported every refusal to the anti-cheat module. That overload is used everywhere, not only for incoming RPCs, so
  every UI check would have reported. A permission check is not an RPC gate. Only `JMPermissions.HasRPC` reports a refusal
  (see [permissions.md](permissions.md)); `Has` stays a pure yes or no.

Rules that follow:

- A predicate (`Has*`, `Is*`, `Can*`) has no side effects: no logging, no reporting, no state change.
- Before adding a call to shared code, find who else reaches it. `grep -w` for the name; a "single choke point" is usually not.
- Do not add a hook, a signal, a flag reset, a new setting or a fallback because it "seems useful". Note it in the
  reply and let the owner decide.
- A fix touches the lines the bug is in. A cleanup that comes along goes in its own commit (section 2).
- If the comment above a member says never or must, it is a constraint. Read it before editing that member.

## 5. Permissions

Already documented in [permissions.md](permissions.md); the mistakes that keep coming back:

- **Register in `DeclarePermissions()` on the module, never in a form, `OnInit` or `OnCreate`.** It must run on the server and
  on every client before the mission loads. A form exists on the client only and too late (e915566a, c2a749e5 removed the
  registrations an example mod had put in a form, and moved them to the example module).
- **Use a `JMConstants.PERM_*` constant, never a string literal.** A typo is not an error: an unregistered node resolves to its
  nearest registered parent and the check quietly tests something else. A constant's value is the key stored in role files;
  never change one, add a new constant.
- **Do not guard a permission call against a `NULL`.** `JMPermissions.Has` / `HasRPC` / `Register` used to return `false` when the
  manager or the sender was `NULL`; that hid the caller's bug (7b46e56f). A `NULL` there is now a loud exception in the log, and
  it is meant to be: fix the caller. The sender of an RPC received on the server is never `NULL`.
- **Validate on the server, always.** Client checks only hide controls. Every server RPC handler calls
  `JMPermissions.HasRPC`, directly or through a helper.
- Example mods (`ExampleScriptOverride/`) are public API. They must follow every rule here, since mod authors copy them.

## 6. Game-version `#ifdef`s

The mod supports several DayZ versions at once. Version symbols look like `DAYZ_1_29`. Two rules, both from cleaning up
1.29/1.30 branches (3c3d1276, 198906d4):

- **Gate on the old version, not the new one.** Code for 1.30 and later is `#ifndef DAYZ_1_29`, not `#ifdef DAYZ_1_30`. The
  second stops working the day 1.31 ships and nobody notices until then. Code that only the old version needs is
  `#ifdef DAYZ_1_29` or `#ifdef DAYZ_1_28` (`DestroyWidget( layoutRoot )` in `JMConfirmation` is that).
- **Say which side is which.** Put `//! 1.30+` under an `#ifndef DAYZ_1_29` (as `PlayerBase` and `JMWeatherPreset` do). It is
  easy to get the polarity wrong: `Construction.c` had `#ifndef DAYZ_1_29` around a call that only 1.29 needs and had to be
  swapped back.
- When an old version is dropped, delete its `#ifdef` branch and keep the other one. Do not leave a dead branch.
- The whole file can be gated (`MotorbikeScript.c`, `COTPPERequester_SandstormEffect.c` start with `#ifndef DAYZ_1_29`).
  The `#endif` at the end of the file is part of that, do not lose it when editing.

## 7. Configuration belongs in `config.cpp`

Imagesets and widget styles are registered in `Scripts/config.cpp` (`imageSets`, `widgetStyles` entries), not loaded by
script. Loading from script needed `DayZGame` and `MissionGameplay` hooks plus a 91-line `JMWidgetStyles` class, all deleted
in dca90c83. A new imageset or style is a `config.cpp` entry and a file, nothing else. Check config before writing
loader code.

## 8. Generated files and the `.dz-project-manager` folder

- `Scripts/Data/Version.hpp` and `Scripts/3_Game/CommunityOnlineTools/JMBuildInfo.c` are **generated** by
  `Workbench/Batchfiles/GenerateVersion.ps1` (through `build.ps1`). They appear as modified in `git status` after every
  build. Never edit them by hand; edit the generator.
- `-NoBump` regenerates the constants without incrementing the build number. Use it for verification builds.
- `.dz-project-manager/local.toml` is machine specific and uncommitted. A change to the mod set goes there, and is backed
  up and restored byte-exact afterwards.

## 9. Building and verifying

Full procedure and the tools are in [cleanup-playbook.md](cleanup-playbook.md) section 5 and
[../ui/module-form-patterns.md](../ui/module-form-patterns.md) 15. What went wrong before:

- **Run the build unpiped and in the background** (`./build.ps1 -NoBump --confirm build`, `run_in_background`). Piping it,
  or chaining `await-boot` after it with `;`, killed the server early and gave four `CRASH` verdicts in a row for code that
  was fine. Renamed constants and removed overrides were bisected for nothing. A `CRASH` whose script log ends at
  "Module: World" with no error line means suspect the harness first, rebuild unpiped, and only then touch code.
- **`await-boot` is a separate call** after the build starts.
- **A stale server gives a stale verdict.** A `DayZDiag_x64` process left from the last boot makes `build` attach to its logs
  and print `BOOT_OK` without compiling. Stop it first, record the time, and trust only a script log newer than that time.
  `.logs/build_verdict.txt`, the harness output and a "task completed" notice can each belong to the previous run.
- **`lint-scripts` can be a silent no-op.** After enough actions in a session it prints `[dry-run] ... no operation
  dispatched`, which is not a pass. Use `verify-fix`, which always runs. Neither checks member lookup or call signatures:
  only a real build catches those (see [cleanup-playbook.md](cleanup-playbook.md) 3.6).
- **A server boot proves the scripts compile, not that the UI works.** `OnCreate`, widget building, menus and every
  client-only path do not run. State that plainly in the reply and list what was not exercised. A registry that fills in
  `DeclarePermissions()` is one of the few paths a server boot does run, so it is a cheap smoke test.
- **Read the first `(E)` under `JM/COT`.** `Bad type 'map'`, `Bad type CF_Modules`, `Bad type JsonFileLoader` further down
  are follow-ons.
- **Expansion-guarded code cannot be build-verified at the moment.** The COT plus Expansion set booted on 2026-09-01 and
  fails on the current game build (2026-09-21) inside Expansion's own script (`Expected attribute call` in
  `dayzplayerimplement.c`). `JMTerritory*` and the `DZ_Expansion_Core` blocks in `JMLoadoutModule` were checked by reading
  only. Try that build once at most; do not loop on it. Say in the reply that the code was not compiled.
- Do not report a change as working before a build reports `BOOT_OK` with 0 `JM/COT` errors. Say what was checked and what was not.

## 10. Debugging native crashes

- **Instrument before fixing.** A crash with no script stack trace: add `Print()` before and after each suspect call, get one
  reproduction, and read which path ran. During the ESP drag crash, three fixes written from the dump alone all targeted a
  path that never executed; one instrumented run found the real one. A "before" with no matching "after" names the killing
  call. Put one print before any early return so that its absence proves the handler was never entered. Strip them once
  confirmed.
- **Crash dumps are not in the Profiles folder.** They are `%LOCALAPPDATA%\CrashDumps\DayZDiag_x64.exe.<PID>.dmp`. Server
  and client are the same executable and file size does not tell them apart; search the dump for `serverDZ.cfg` /
  `Profiles\Server` against `Profiles\ClientA` (check ASCII and UTF-16LE).
- **`crash_*.log` in the Profiles folders is usually not a crash.** With `newErrorsAreWarnings 1` it collects assertion and RPC
  warnings on every boot. A real native crash is an `Unhandled exception ... SEH exception thrown` block.
- **Do not reason about order from `dz-project-manager logs --search`.** It reorders lines. Read the raw `script_*.log` /
  `.RPT` when order matters, and take time of death from the RPT.
- The client `script_*.log` prints `UIActionContextMenu.OnClick: row id=...` for every menu click; grep it to see what a menu
  row really reports.

## 11. Widget lifecycle: `delete` vs `Unlink`

**Never `delete` a widget-owning object.** `delete` on anything holding a `Widget` frees memory
the engine's own widget tree still references, which segfaults later — reliably on mission
finish, when everything tears down at once. Use `Widget.Unlink()` to detach a widget before
recreating or discarding it. Three patterns already used throughout
`Scripts/5_Mission/CommunityOnlineTools/gui/` cover every case:

- Rebuilding a container in place: `if ( m_Wrapper ) m_Wrapper.Unlink();` then reassign it.
- `JMWindowBase.DestroyWidget( layoutRoot )` / `COT_ScriptedWidgetEventHandler.DestroyWidget()` -
  a null- and `"INVALID"`-checked `w.Unlink()` helper, called from destructors.
- `UIActionManager.ClearChildren( parent )` - detaches every child of a widget one by one, for
  rebuilding a dynamic list without disturbing the parent itself.

Two exceptions, both `delete this` on the handler object itself, never on a `Widget`:
`COT_ScriptedWidgetEventHandler.Destroy()` / `COT_WidgetHolder.Destroy()`, and
`JMWebhookModule`'s `delete cfg` (a `ConfigFile`, not a widget).

Evidence: `da1fe2ab`, `ebb09735`, `22fb6f9e`, `51e06035`.

## 12. Enforce Script: what compiles but breaks

Syntax limits that the compiler rejects (no ternary, no block scope, no multi-line expressions, one class per file) are in
`CLAUDE.md`. These pass the compiler and fail later:

- **`foreach ( T x : obj.GetList() )` throws `NULL pointer to instance` at runtime.** Assign the result to a local, then
  iterate the local. A member field or a local is fine. Found in `JMPlayerModule.DeclarePermissions`; a compile-only boot
  passes and the exception shows only when the loop runs.
- **`new X().Method()` does not compile** (`Broken expression (missing ';'?)`). Pass the `new X()` to a helper that returns it,
  and chain on the helper's return.
- **A temporary passed as an argument is overwritten by string-returning calls beside it.**
  `menu.AddRow( members[i], CategoryLabel( members[i] ), CategoryIcon( members[i] ), c )` registered the icon path as the row
  id, so a filter matched nothing (`JMItemCategoryPicker.BuildGroupPage`, `JMTeleportFilter`). Constants and plain locals are
  safe. Put every argument in a local first when the call also has string-returning calls in its argument list.
- **No block scope**: two locals with one name in a function collide, even in different branches or `case`s. It bites when
  code is generated in a loop.
- **A string callback naming a method that does not exist fails silently** (`SetOnClick( this, "Name" )`, `CallLater`,
  `CreateOverlayMenu`). After a move, grep the quoted name and check that the target is the class that now holds the method.
- **`CallLater( Method, ... )` and update-queue entries bind to an instance.** After a move they must name the new owner
  (`m_Roster.UpdateGroupAnimation`), and the matching `Remove` must name it too.
- **Implicit object-to-string concatenation inside `Print()` crashes the script compiler natively.** Format explicitly. The
  linter does not catch it.
- **`ConfigGetChildrenCount` returns 0 on flat arrays**: use `ConfigGetTextArray` / `ConfigGetFloatArray`.
- **`#define` scope is per file**, not per compiled layer.
- **A `modded class` cannot touch a `private` member**; that is why fields are `protected`.
- **Guard destructors** with `if ( !g_Game ) return;`.
- **Deferred tabs:** null-check every widget touched from a timer or a response handler; the tab may never have been built.
- Set the flags before the size (`SetFlags( HEXACTSIZE | VEXACTSIZE )` then `SetSize`), and mutate state before firing the
  event that reads it. More layout traps: [../ui/limitations.md](../ui/limitations.md), [../ui/reference/pitfalls.md](../ui/reference/pitfalls.md).

## 13. Do not break other mods

DayZ-Expansion and other mods use `modded class` on COT forms and modules and override their members.

- **Never narrow `protected` to `private`.** Do not remove or rename a public member without a forwarder
  (`JMDeprecated.WarnOnce`) and an entry in [deprecations.md](deprecations.md). A moved `static const` cannot forward: list it
  under "Removed with no forwarder".
- **Read the comment before deleting a field that looks unused.** Compatibility mirrors (`m_ApplyStats`, `m_Stamina`,
  `m_RefreshStats`) exist because another mod names them.
- **Do not reorder or remove parameters** of `UIActionManager.Create*`; add new ones at the end.
- The examples in `ExampleScriptOverride/` are the public contract. Grep them before moving a member.
- Removed or renamed user-facing text: check every call site of the stringtable key first; two keys with different text
  can be deliberate ([cleanup-playbook.md](cleanup-playbook.md) 7).

## 14. Working in this environment

- **Big heredocs fail in the Bash tool.** A command with a few dozen lines of mixed quotes is rejected with `unexpected EOF while
  looking for matching '` and runs nothing. Use the Write or Edit tool for multi-line content; keep helper scripts in
  files. Small `<<'EOF'` bodies are fine.
- **Write JSON and Python specs as files.** A shell heredoc can halve backslashes or expand `\n` and `\t`, which corrupts a
  regex or a spec.
- `grep -P` fails on this machine's locale; use `-E`.
- Grep output is noisy: print counts and short tables rather than raw hits, and exclude your own definitions and deprecation
  message strings. When a scan and a hand read disagree, trust the hand read and fix the scan.
- Small steps, each built and booted, beat one large rewrite.
