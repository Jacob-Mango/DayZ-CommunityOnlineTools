---
name: Server Stats Module
description: Real-time server telemetry (FPS, uptime, memory usage, player count, active entity count) streaming
type: project
---

## Overview

`JMServerStatsModule` collects and streams server performance telemetry to connected administrators. It monitors server FPS (frames per second), memory footprint, active player connections, session duration, and world tick times.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/ServerStats/JMServerStatsModule.c` — Telemetry sampler, stats calculation loop, RPC handler
- `Scripts/5_Mission/CommunityOnlineTools/modules/ServerStats/JMServerStatsForm.c` — UI window displaying live telemetry graphs and performance indicators

## RPC Range

Range: `10940` – `10941` (`JMServerStatsModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10940` | `Sync` | Server → Client | Streams periodic telemetry packet (FPS, RAM, player count, tick delta) |
| `10941` | `Request` | Client → Server | Subscribes or unsubscribes client UI to server telemetry stream |

## Telemetry Metrics

- **Average Server FPS**: Measured via engine tick rate delta (`1.0 / delta_time`).
- **Memory Footprint**: Active RAM consumption reported by server instance.
- **Connection Count**: Active players online vs server max slots (`g_Game.GetPlayers()`).
- **World Up-Time**: Total elapsed server runtime in seconds.
