# Codebase Architecture & Systems

Documentation covering the internal structure, network routing, permissions model, and core systems of Community Online Tools.

---

## Documents

- [architecture.md](architecture.md) — Directory layout, script module layers (`1_Core`/`3_Game`/`4_World`/`5_Mission`), registered module lifecycle, profile paths, sidebar navigation flow.
- [rpc.md](rpc.md) — RPC enum allocation ranges, `ScriptRPC` read/write/send mechanics, network routing, server authority enforcement, never-targeted rule.
- [permissions.md](permissions.md) — Permission string registration, `JMPermissions.Has` & `HasRPC` checks, `roles.json` hierarchy, player instance permissions.
- [module-creation.md](module-creation.md) — Step-by-step guide to building a new COT module: `DescribeModule`, `JMModuleAction`, RPC enum allocation, registration, settings JSON persistence.
- [code-comments-reference.md](code-comments-reference.md) — Synthesized reference index of inline code comments, language quirks, UI constraints, and architectural notes.
- [mod-compatibility.md](mod-compatibility.md) — API contracts with dependent mods (e.g. DayZ-Expansion): `protected` vs `private` rules, override safety, compatibility audit script.
- [deprecations.md](deprecations.md) — Mapping of deprecated methods and classes, forwarder warnings (`WarnOnce`), legacy compatibility layer.
- [advanced.md](advanced.md) — Deep technical notes on `JMScriptInvokers`, notifications, active state flags, JSON serializers, chat commands, and framework integrations.
