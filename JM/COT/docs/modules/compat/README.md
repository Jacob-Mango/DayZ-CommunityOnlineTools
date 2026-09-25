---
name: Compat Module
description: Cross-mod compatibility layer, legacy method forwarders, and DayZ engine/mod API abstraction
type: project
---

## Overview

`JMCompatModule` handles cross-mod compatibility hooks and maintains backward compatibility for third-party DayZ server mods (such as DayZ-Expansion, CF, or standalone COT addons) that interact with COT's API.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Compat/JMCompatModule.c` — Compatibility initializers, mod detection, legacy wrapper functions

## Functionality

- **Mod Detection**: Checks for loaded mod signatures at runtime using `IsModLoaded(string typeName)`.
- **Legacy API Wrappers**: Wraps old method signatures to prevent mod crashes when third-party mods call deprecated COT interfaces (see [../../codebase/deprecations.md](../../codebase/deprecations.md)).
- **Safety Fallbacks**: Provides safe fallback implementations when optional mod dependencies are absent.
