---
name: Command Module
description: Chat command processing (/cot, /tp, /god, etc.), admin permission checks, and server-side command execution dispatcher
type: project
---

## Overview

`JMCommandModule` registers and handles text-based chat commands (e.g. `/cot`, `/tp`, `/god`, `/heal`, `/kick`). It intercepts chat messages sent by players, checks if the sender has permission to execute the specified command, parses arguments, and triggers the corresponding action.

## Key Files

- `Scripts/4_World/CommunityOnlineTools/Classes/Commands/JMCommandModule.c` — Chat message interceptor, command registration, argument parser, RPC handlers

## RPC Range

Range: `10420` – `10421` (`JMCommandModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10420` | `Execute` | Client → Server | Sends parsed command string and parameters from client to server |
| `10421` | `Feedback` | Server → Client | Sends command execution feedback message to admin chat / notification overlay |

## Key Features

- **Command Syntax**: Listens for commands prefixed with configurable trigger characters (default `/`).
- **Permission Checking**: Every registered command has a linked permission node (e.g., `/god` checks `PERM_PLAYER_GOD`).
- **Feedback Messaging**: Sends success/failure notifications directly to the executor's chat box or COT notification banner.
