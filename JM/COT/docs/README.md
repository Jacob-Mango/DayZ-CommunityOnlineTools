# COT Documentation

Reference docs for DayZ Community Online Tools.

## Layout

```
docs/
├── ui/          DayZ vanilla UI reference + COT form patterns
├── systems/     COT architecture, RPC, permissions, modules
├── modules/     Per-module deep dives (player flows, etc.)
├── missions/    Mission/task logs (research notes per session)
├── audits/      Module audits (LootAnalysis, etc.)
└── history/     Chronological recap of UI / layout changes
```

## Quick links

### DayZ UI & forms

- [ui/README.md](ui/README.md) — index of every UI doc (vanilla reference + COT forms)
- [ui/forms.md](ui/forms.md) — `JMFormBase` lifecycle, `UIActionManager` API, confirmations, permission gating
- [ui/limitations.md](ui/limitations.md) — known Enforce Script layout limitations and workarounds
- [ui/window-and-sidebar.md](ui/window-and-sidebar.md) — `JMWindowBase` drag/embed/z-order, `JMCOTSideBar`, `CommunityOnlineToolsBase` lifecycle
- [ui/widget-input-localization.md](ui/widget-input-localization.md) — layout XML widget types, `UIActionSlider/SelectBox`, all 46 input action names, stringtable format
- [ui/styling/style-guide.md](ui/styling/style-guide.md) — naming conventions, patterns, best practices for COT forms
- [ui/styling/textures-edds-and-paa.md](ui/styling/textures-edds-and-paa.md) - converting an image to `.edds` or `.paa`, the `ImageToPAA` suffix rules, the `.edds` container

### COT systems

- [systems/architecture.md](systems/architecture.md) — repo structure, layer system (1_Core/3_Game/4_World/5_Mission), module registration
- [systems/module-creation.md](systems/module-creation.md) — how to create a new module: base classes, required overrides, registration
- [systems/rpc.md](systems/rpc.md) — RPC enum pattern, client↔server flow, `ScriptRPC` write/read/send
- [systems/permissions.md](systems/permissions.md) — registering permissions, `HasPermission` checks, roles, `JMPlayerInstance`
- [systems/advanced.md](systems/advanced.md) — `JMScriptInvokers`, COT active state, notifications, admin logging, JSON persistence, chat commands, ESP, loadouts, object spawner, weather Send/Exec pattern, MissionServer overrides, CF framework, camera, teleport persistence, vehicle enumeration, MissionGameplay, Expansion integration
- [systems/playerbase.md](systems/playerbase.md) — COT `PlayerBase` properties/methods (godmode/freeze/invisibility), `Exec_*` pattern, all player permissions, static helpers, `config.cpp` defines
- [systems/dayz-reference.md](systems/dayz-reference.md) — vanilla DayZ scripts at `DayZ Projects/scripts`: entity hierarchy, `PlayerBase` stats, inventory, weather API, ECE spawn flags
- [systems/mod-compatibility.md](systems/mod-compatibility.md) — COT's API contract with dependent mods (DayZ-Expansion): why `private` breaks `modded class`, which members are held `protected`, and the audit script to re-check

### Modules

- [modules/player-flows.md](modules/player-flows.md) — `JMPlayerForm` player list / stat sliders, spectating start/end flow, ban check at login, `JMPlayerBan` structure, `PluginAdminLog` webhook hooks

### Mission & research logs

- [missions/](missions/) — session research notes (one file per mission date)

### Audits

- [audits/](audits/) — per-module audits (e.g. loot analysis correctness review)
- [audits/2026-07-15-todo-mods-audit.md](audits/2026-07-15-todo-mods-audit.md) — the 8 third-party mods in `TODO/`: what they have that COT lacks, 5 COT bugs they expose, and what not to port

### Change history

- [history/LAYOUT_CHANGES_RECAP.md](history/LAYOUT_CHANGES_RECAP.md) — chronological recap of every UI / layout change

## Conventions used in these docs

- **Code references** use the `file:line` format and link relative to the repo root.
- **Engine truth** boxes mark behaviour verified against the engine bindings.
- **Convention** boxes mark project-level conventions that aren't enforced by the engine.
- Cross-doc links use relative paths (`../systems/rpc.md`) so docs work both in repo browsing and external viewers.

## Last updated

2026-07-06 — reorganised into `ui/`, `systems/`, `modules/`, `missions/`, `audits/`, `history/`. UI files previously at root (`ui_forms.md`, `window_and_sidebar.md`, `widget_input_localization.md`, `UI_*`) moved into `ui/`. System files moved into `systems/`. Mission logs and audits got their own folders. Duplicate `MEMORY.md` removed (cross-session memory lives outside the repo).