---
name: Namalsk Event Manager Module
description: Integration module for Namalsk map weather events (EVR storms, blizzard controls, aurora triggers)
type: project
---

## Overview

`JMNamalskEventManagerModule` is a map-specific integration module registered automatically when Namalsk mods (`NamEventManager` and `EVRStorm`) are loaded. It grants admins direct control over EVR storm sequences, blizzards, aurora displays, and Namalsk specific environmental events.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Namalsk/JMNamalskEventManagerModule.c` — Namalsk event integration, event hooks, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Namalsk/JMNamalskEventManagerForm.c` — UI form for triggering and scheduling Namalsk weather events

## RPC Range

Range: `10400` – `10404` (`JMNamalskEventManagerRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10400` | `Sync` | Server → Client | Syncs Namalsk storm state and timers |
| `10401` | `TriggerEVR` | Client → Server | Manually triggers EVR storm sequence |
| `10402` | `CancelEVR` | Client → Server | Aborts active EVR storm sequence |
| `10403` | `SetBlizzard` | Client → Server | Configures blizzard intensity and duration |
| `10404` | `SetAurora` | Client → Server | Toggles visual aurora effects |

## Conditions

The module registers dynamically during `JMModuleConstructor.RegisterModules()` only if `IsModLoaded("NamEventManager")` and `IsModLoaded("EVRStorm")` evaluate to true.
