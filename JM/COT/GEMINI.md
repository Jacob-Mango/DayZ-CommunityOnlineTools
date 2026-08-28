# Community Online Tools — Project Context for Gemini

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
8. Gemini cannot execute the interactive `/model` command from `GEMINI.md`. When a main-session
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
| "Enforce Script syntax / gotchas" | [`ai/docs/enforce-script/conventions.md`](ai/docs/enforce-script/conventions.md) |

---

## Build workflow

**Always use the harness — do not hand-run a .bat + grep the log.** The harness is the
native Rust tool, `dz-project-manager.exe` at the repo root — call it directly:

```
./dz-project-manager.exe build [--build-only] [--check PATH[:NEEDLE]] [--kill]
                                [--timeout 480] [--freeze 60]
```

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
