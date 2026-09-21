---
name: Anti-Cheat Module
description: Basic server-side cheat detection hooks, movement/speed anomaly checks, and admin notification / kick responses
type: project
---

## Overview

`JMAntiCheatModule` provides server-side detection hooks for movement anomalies (speed hacks, teleports, illegal position updates) and invalid inventory/item operations. It alerts connected admins and optionally logs or disconnects offending clients.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/AntiCheat/JMAntiCheatModule.c` — Detection logic, tick checks, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/AntiCheat/JMAntiCheatForm.c` — Admin settings UI form for configuring sensitivity and action rules

## RPC Range

Range: `10920` – `10923` (`JMAntiCheatModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10920` | `Sync` | Server → Client | Syncs anti-cheat settings to client admins |
| `10921` | `Update` | Client → Server | Updates threshold values or toggle flags |
| `10922` | `Detection` | Server → Client | Notifies admins of a cheat detection event |
| `10923` | `Kick` | Server → Client | Triggers a client kick/disconnect action for repeated detections |

## Permissions

- `PERM_ANTICHEAT_VIEW` (`AntiCheat.View`) — View anti-cheat alerts and settings form
- `PERM_ANTICHEAT_MANAGE` (`AntiCheat.Manage`) — Configure detection thresholds and kick rules

## Detection Vectors

- **Speed / Teleport Checks**: Compares player position changes against maximum allowable delta distance per server frame.
- **Item / Inventory Validation**: Checks for illegal item creation or item property manipulation bypassing game rules.
- **Admin Alerts**: Sends high-priority notifications to connected admins when a threshold violation occurs.
