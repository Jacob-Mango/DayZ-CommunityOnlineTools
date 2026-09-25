---
name: Ban Manager Module
description: Ban list persistence (bans.json), timed and permanent bans, GUID/IP lookups, and player connection check at login
type: project
---

## Overview

The Ban Manager (`JMBanModule`) manages server bans. It provides permanent and duration-based (temporary) banning by GUID and IP address, persisting records to `$profile:COT/bans.json`. It intercepts client login events via `OnClientConnect` to deny access to banned players.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMBanModule.c` — Main module, connection interceptor, ban list loader/saver, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMBanForm.c` — Ban management UI form
- `Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMPlayerBan.c` — Data model for ban records (GUID, Steam64 ID, IP, ban reason, expiry timestamp, issuer)

## RPC Range

Range: `10800` – `10805` (`JMBanModuleRPC` in `JMBanModule.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10800` | `Sync` | Server → Client | Syncs active ban list to admin UI |
| `10801` | `Ban` | Client → Server | Adds a new ban entry (permanent or timed) |
| `10802` | `Unban` | Client → Server | Removes an existing ban entry by GUID/IP |
| `10803` | `Update` | Client → Server | Updates reason or duration of an existing ban |
| `10804` | `Search` | Client → Server | Searches stored bans by string filter |
| `10805` | `Log` | Server → Client | Sends ban audit log entry to client |

## Permissions

- `PERM_BAN_VIEW` (`Ban.View`) — View the ban list in COT UI
- `PERM_BAN_ADD` (`Ban.Add` / `Ban.Create`) — Create new bans (player list or manual GUID/IP)
- `PERM_BAN_REMOVE` (`Ban.Remove` / `Ban.Delete`) — Lift bans

## Persistence

Bans are saved in `$profile:COT/bans.json`.
- **Automatic Expiry**: Timed bans automatically expire when `g_Game.GetTime()` / UTC epoch exceeds the ban expiry timestamp.
- **Login Check**: During `OnClientConnect`, `JMBanModule` checks the incoming player's GUID and IP against the loaded ban list. If a match is found and not expired, `g_Game.DisconnectPlayer()` is triggered with the ban reason string.
