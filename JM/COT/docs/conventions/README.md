# Project Conventions & Coding Standards

Standards, guidelines, gotchas, and refactoring playbooks for COT code contributions.

---

## Documents

- [conventions.md](conventions.md) — **Essential reading before editing scripts**: what not to do (comments, scope creep, permissions, preprocessor `#ifdef` rules, build/verify steps, Enforce runtime gotchas, mod compatibility).
- [naming.md](naming.md) — Identifier naming grammar: `Add`/`Remove` pairs, lifecycle hooks (`On<Subject><Phase>`), `Exec_` vs `RPC_`, popups, verbs.
- [style-guide.md](style-guide.md) — Code formatting, line endings (`CRLF`/`LF`), tab indentation, variable declaration order, comment formatting.
- [cleanup-playbook.md](cleanup-playbook.md) — Step-by-step playbook for refactoring legacy forms: member ordering, tab state ownership, component extraction, verification workflows.
- [module-folder-layout.md](module-folder-layout.md) — Module file organization standards: `Tabs/<Name>/`, `Components/`, shared helpers, `organize_tabs.py` batch tool.
