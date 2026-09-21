# Community Online Tools — Project Context for Claude

DayZ server mod. Enforce Script (`.c`) + config files (`.cpp`) + assets.
P-drive (`P:\`) must be mounted before any build. `dz-project-manager.exe` now mounts it
automatically (`WorkDrive.exe /Mount`) before any action that touches it, so a missing
P: no longer needs manual intervention. Check state with
`./dz-project-manager.exe mount-workdrive status`; `DZPM_NO_AUTO_MOUNT=1` disables the
automatic mount.

---

## Communication style

Default terse (caveman mode) for all chat responses in this repo — drop filler/hedging,
keep full technical accuracy. Code, commit messages, PRs, and security-relevant text:
write normal, no compression.

---

## Model routing

Use the cheapest model that can reliably complete the current phase. Re-evaluate when the
work type changes, not after every tool call. Avoid model thrashing.

| Current work | Model | Routing rule |
|---|---|---|
| Mechanical search, file discovery, log extraction, formatting, repetitive edits, simple documentation | **Haiku** | Delegate to a Haiku subagent when the task is isolated and its result can be verified cheaply. |
| Normal implementation, Enforce Script changes, routine debugging, tests, refactors, code review | **Sonnet** | Default model for the main coding session and most project work. |
| Architecture, ambiguous cross-system bugs, concurrency/security-sensitive work, large multi-module refactors, difficult root-cause analysis | **Opus** | Escalate only when deeper reasoning materially improves correctness. |

Routing procedure:

1. Start and remain on **Sonnet** unless the current phase clearly matches another tier.
2. Prefer delegation over switching the main session: use a model-specific subagent for
   isolated work, then return only its relevant findings to the main context.
3. Use **Haiku** only for bounded, mechanical work with an objective verification step.
   Never let Haiku make final architecture decisions or overwrite a stronger model's design.
4. Escalate to **Opus** when any of these apply:
   - Sonnet produced two materially failed approaches.
   - The root cause spans several systems or is still unclear after evidence gathering.
   - The change defines architecture or affects many modules.
   - Correctness depends on subtle lifecycle, state, performance, security, or concurrency reasoning.
5. After Opus produces the plan or resolves the hard reasoning, return implementation and
   routine verification to Sonnet unless the remaining work is still high-risk.
6. Do not switch models merely because a task is long. Switch based on reasoning difficulty,
   risk, and how cheaply the result can be verified.
7. Before a main-session model switch, compact or summarize context when useful. A switch
   causes the next model response to re-read the conversation history.
8. Claude cannot execute the interactive `/model` command from `CLAUDE.md`. When a main-session
   switch is warranted, state the recommended target model and reason in one terse sentence;
   otherwise route automatically through configured subagents.
9. Before spawning an Agent for isolated work, run
   `./dz-project-manager.exe route --task <read|find|write|fix|architect> --dry-run` and pass its
   resolved model as the Agent tool's `model` param instead of guessing. This makes the AI
   Settings policy (`ai.toml`, edited via the tool's GUI) the actual source of truth for
   subagent model choice, not just an audit trail. Refusals (cost/opus-hour cap) are informational
   only here — they don't block; if refused, fall back to the table above.

Recommended project subagents in `.claude/agents/`:

- `model: haiku` — repository search, log triage, documentation lookup, mechanical transforms.
- `model: sonnet` — implementation, debugging, tests, standard reviews.
- `model: opus` — architecture and difficult investigation.

---

## Documentation

Before writing or modifying scripts, check the relevant doc first.

| Question | Where to look |
|---|---|
| "How does system X work?" | [`Docs/`](Docs/) |
| "Enforce Script syntax / gotchas, what not to do, commit rules" | [`docs/systems/conventions.md`](docs/systems/conventions.md) |

---

## Build workflow

**Always use the harness — do not hand-run a .bat + grep the log.** The harness is the
native Rust tool, `dz-project-manager.exe` at the repo root. Call it through
`build.ps1`, which stamps the build version first and then forwards every argument
to the real action:

```
./build.ps1 [--build-only] [--check PATH[:NEEDLE]] [--kill] [--timeout 480] [--freeze 60]
./build.ps1 -Channel stable          # override channel auto-detection
./build.ps1 -NoBump                  # regenerate constants, do not increment the build number
```

`build.ps1` runs `Workbench/Batchfiles/GenerateVersion.ps1`, which writes the two
GENERATED files the mod reports its own version from — `Scripts/Data/Version.hpp`
and `Scripts/3_Game/CommunityOnlineTools/JMBuildInfo.c`. Never hand-edit either;
edit the generator. Channel is auto-detected (tag → `stable`, main/master → `beta`,
`exp*` branch → `experimental`, feature branch or dirty tree → `internal`) and can
be forced with `-Channel` or `$env:COT_BUILD_CHANNEL`.

Calling `./dz-project-manager.exe build` directly still works and is fine for a
throwaway compile check — it just leaves the version constants at whatever the last
stamped build wrote.

It polls until ONE terminal state and prints a compact verdict block:
`BOOT_OK` (full boot — "Average server FPS" in the RPT) · `COMPILE_FAIL` · `BUILD_FAIL`
· `FROZEN` (RPT mtime stalled while alive — the freeze a plain "is it running?" check
misses) · `CRASH` · `TIMEOUT`. Exit 0 = BOOT_OK/BUILD_OK with 0 script errors and all `--check`s.

**Server Lifecycle & Verdict Notification:** When testing the build process, the server will **never shut down on its own if everything is fine** (it remains running). The Rust harness monitors the logs and notifies the agent of the result two ways: **live**, every step is printed to stdout **and flushed** the instant it's decided — the script log/RPT appearing, a ~10s heartbeat while waiting, then the exact keyword that decided the terminal state (`"Average server FPS"` for `BOOT_OK`, `Build.failure` for `BUILD_FAIL`, etc.); and **durably**, the full transcript plus the verdict block is written to `.logs/build_verdict.txt` on every run (overwritten).

**Watching a server you didn't just build** (already running, or launched via `launch-server`/`launch-mp`): `./dz-project-manager.exe await-boot [--timeout 360] [--freeze 50] [--keyword TEXT]` polls the same way `build` does but never kills, rebuilds, or rotates anything.

```
Any .c script edited?
├── NO  → ./dz-project-manager.exe build --build-only
└── YES → ./dz-project-manager.exe build
```

**Verify BEFORE claiming a fix works.** A change isn't done until `build` reports
`BOOT_OK` (or `.logs/build_verdict.txt` indicates `BOOT_OK`). Pre-flight a .c change with
`./dz-project-manager.exe lint-scripts --changed` to catch compile errors.

**Filtering logs.** Don't grep raw RPT/script logs by hand — the tool has filters:

```
./dz-project-manager.exe logs --errors-only --hide-junk [--source server|client|both] [--search TEXT]
./dz-project-manager.exe tail-log errors
./dz-project-manager.exe show-server-errors
./dz-project-manager.exe show-boot
```

Run `build` with `run_in_background: true`; it self-terminates with the verdict. Full
action list: `./dz-project-manager.exe list`.

**AI-agent confirmation gate.** After enough actions in one session, the tool starts
refusing to actually run `build`/`lint-scripts` for an AI-agent caller: `build`
returns `CONFIRMATION_REQUIRED: action 'build' requires explicit confirmation`, and
`lint-scripts` silently no-ops with `[dry-run] action 'lint-scripts' authorized; no
operation dispatched` instead of erroring — don't mistake that dry-run line for a
real clean result. This is a `[ai] max_cost_per_task_override_usd` guard in
`.dz-project-manager/project.toml`, not a bug. Append `--confirm build` (or
`--confirm lint-scripts`) to push the action through for real once you hit it — Claude
is authorized to add this flag itself without asking, since the human has already
approved the underlying build/lint action by asking for the change. If `lint-scripts`
still won't produce a real result even with `--confirm`, use `./dz-project-manager.exe
verify-fix` instead — same linter, always executes for real, reports `OK lint N
error(s)... across M file(s)`.

---

## Repo layout

Community Online Tools is structured as follows:
- `Scripts/` — holds the script files across all Enforce layers (`3_Game/`, `4_World/`, `5_Mission/`).
- `GUI/` — holds layout files.
- `languagecore/` — holds localization stringtables.
- `Objects/` — holds assets and models.

Each of these directories acts as a separate addon containing a `config.cpp` file.

---

## Critical Enforce Script rules

1. **No multi-line expressions** — `||`/`&&`/`+` must stay on one line
2. **No block scope** — `if/else if` branches share function namespace; use distinct variable names
3. **No ternary `? :`** — use `if/else` assignment
4. **`ConfigGetChildrenCount` returns 0 on flat arrays** — use `ConfigGetTextArray` / `ConfigGetFloatArray`
5. **One class per file, filename = class name**
6. **`foreach` over a method's return value NPEs at runtime** — assign to a local first
7. **`new X().Method()` does not compile** — use a helper that takes and returns the object
8. **No temporaries as call arguments** next to string-returning calls (`arr[i]`, `obj.Get()[i]`) — hoist to locals

---

## What not to do

Full list with the evidence behind each rule: [`docs/systems/conventions.md`](docs/systems/conventions.md). Read it before editing scripts.

- **Never delete or shorten a comment.** Doc comments move with their member; a wrong comment is corrected, not removed.
- **Do the change asked for, no more.** No extra hooks, reports, flag resets, settings or fallbacks. Suggest them in words. `Has*`/`Is*`/`Can*` checks have no side effects (no logging, no anti-cheat reporting); only `JMPermissions.HasRPC` reports.
- **Register permissions in the module's `DeclarePermissions()`** (runs on server and client), never in a form. Use `JMConstants.PERM_*` constants, not string literals. Always re-check on the server.
- **Version `#ifdef`s gate on the old version**: `#ifndef DAYZ_1_29` (tag `//! 1.30+`), not `#ifdef DAYZ_1_30`. Double-check polarity.
- **Imagesets and widget styles go in `config.cpp`**, not script loaders.
- **Whitespace, moves and renames are their own commits**, never mixed with logic. Preserve each file's line endings (`core.autocrlf=true`, mixed CRLF/LF).
- **Never narrow `protected` to `private`**; no removing a public member without a `JMDeprecated` forwarder. Other mods (Expansion) override them.
- **Comments and identifiers are ASCII.**
- **Build unpiped and in the background**; never pipe or `;`-chain `build.ps1` with `await-boot`. Verify the script log is newer than the build start; stop any leftover `DayZDiag_x64` first.
- **A server boot does not run client UI.** Say what was not exercised. Expansion-guarded code currently cannot be compiled here; say so.
- **Native crash, no script trace: instrument first** (`Print` before and after suspects, one repro), fix second.
- **Bash tool: no large heredocs** (they fail and run nothing). Use Write/Edit; keep helper scripts in files.
