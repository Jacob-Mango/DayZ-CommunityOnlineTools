---
name: Action History Module
description: Audit logging of administrative actions executed in COT, RPC history sync, and client-side history viewing form
type: project
---

## Overview

The `JMActionHistoryModule` tracks administrative actions executed by COT module operators. It captures command executions, targeted player modifications, spawn operations, and setting alterations, broadcasting or persisting action logs for auditing and client inspection.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/ActionHistory/JMActionHistoryModule.c` — Module controller, RPC handlers (`RPC_Sync`, `RPC_Clear`), log persistence
- `Scripts/5_Mission/CommunityOnlineTools/modules/ActionHistory/JMActionHistory.c` — Entry model (`JMActionHistoryEntry`) representing timestamp, actor identity, action type, and details
- `Scripts/5_Mission/CommunityOnlineTools/modules/ActionHistory/JMActionHistoryForm.c` — UI window for browsing and searching logged actions

## RPC Range

Range: `10960` – `10962` (`JMActionHistoryModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10960` | `Sync` | Server → Client | Syncs action history entries to client GUI |
| `10961` | `Clear` | Client → Server | Clears administrative history buffer (gated by permissions) |
| `10962` | `Log` | Server → Client / Local | Appends a single action entry to active session history |

## Permissions

- `PERM_ACTIONHISTORY_VIEW` (`ActionHistory.View`) — View the action history log in COT UI
- `PERM_ACTIONHISTORY_CLEAR` (`ActionHistory.Clear`) — Clear logged action records

## Flow & Functionality

1. **Logging**: Whenever an admin executes an action (e.g., healing a player, spawning an item, changing weather), `JMActionHistoryModule.LogAction(actor, target, actionText)` is called.
2. **Persistence**: Log entries are kept in memory on the server and optionally dumped to server logs/webhooks.
3. **UI Sync**: When an admin opens the Action History form, the client sends a `Sync` request. The server verifies permissions and streams the entry buffer.
