---
name: Item Stats Module
description: Diagnostic tool for inspecting item property values, health, quantity, and configuration parameters
type: project
---

## Overview

`JMItemStatsModule` is a diagnostic module used by developers and server admins to inspect detailed item stats and config definitions (e.g. weight, volume, health max, damage levels, inventory slots, attachments) for items held in hands or selected in the world.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/ItemStats/JMItemStatsModule.c` — Diagnostic scanner module
- `Scripts/5_Mission/CommunityOnlineTools/modules/ItemStats/JMItemStatsForm.c` — Inspection window rendering item config parameters

## Key Capabilities

- **Config Inspector**: Reads `cfgVehicles` or `cfgWeapons` config entries for target item class.
- **State Details**: Displays exact item health, wetness, temperature, energy level, and liquid quantity.
- **Attachment Tree**: Lists attached child items and container slot allocations.
