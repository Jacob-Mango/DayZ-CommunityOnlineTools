---
name: Webhook Module
description: Discord webhook integration, administrative event logging stream, embed message formatting, and JSON configuration
type: project
---

## Overview

`JMWebhookModule` / `JMWebhookCOTModule` streams COT administrative actions, ban events, player connections, and security alerts to Discord webhooks via HTTP POST requests. It formats log entries into structured Discord embeds.

## Key Files

- `Scripts/4_World/CommunityOnlineTools/Classes/Webhook/JMWebhookModule.c` — Base webhook HTTP dispatcher, JSON payload formatting
- `Scripts/5_Mission/CommunityOnlineTools/modules/Webhook/JMWebhookCOTModule.c` — COT event listener, webhook trigger callbacks, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/Webhook/JMWebhookForm.c` — UI window for configuring webhook URLs, active channel filters, and message templates

## RPC Range

Range: `10440` – `10447` (`JMWebhookCOTModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10440` | `Sync` | Server → Client | Syncs webhook configuration settings to admin UI |
| `10441` | `Update` | Client → Server | Updates webhook URLs or event toggle flags |
| `10442` | `Test` | Client → Server | Sends a test notification embed to configured webhooks |
| `10443`..`10447` | `Manage` | Client ↔ Server | Channel rules, filter masks, audit logs |

## Permissions

- `PERM_WEBHOOK_VIEW` (`Webhook.View`) — Open Webhook configuration UI
- `PERM_WEBHOOK_MANAGE` (`Webhook.Manage`) — Configure webhook URLs and event subscription channels

## Event Subscriptions

Webhooks can be configured to listen to specific categories:
- Admin Actions (Heal, Kill, Teleport, Give, Strip)
- Spawner Operations (Object spawn, Vehicle spawn)
- Player Connections (Connect, Disconnect, Kick, Ban)
- Anti-Cheat & Security Alerts
