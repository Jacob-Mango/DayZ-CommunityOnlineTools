---
name: Loot Analysis Module
description: Central Economy item density scanner, loot distribution analysis, world item count reporting, and export
type: project
---

## Overview

`JMLootAnalysisModule` scans the active server world to analyze Central Economy (CE) item distribution and item density. It categorizes items by type, zone, tier, and container state, providing server administrators with data to troubleshoot economy issues or item shortages.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/LootAnalysis/JMLootAnalysisModule.c` — Server economy scanner, report generator, RPC handlers
- `Scripts/5_Mission/CommunityOnlineTools/modules/LootAnalysis/JMLootAnalysisForm.c` — UI window displaying item counts, category filters, and export tools

## RPC Range

Range: `10520` – `10525` (`JMLootAnalysisModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10520` | `StartScan` | Client → Server | Initiates a world loot scan |
| `10521` | `SyncProgress` | Server → Client | Streams scan progress updates to client UI |
| `10522` | `SyncData` | Server → Client | Sends completed scan dataset |
| `10523` | `Export` | Client → Server | Triggers export of scan results to file |
| `10524` | `CancelScan` | Client → Server | Aborts an active scan |
| `10525` | `Log` | Server → Client | Audit log entry |

## Permissions

- `PERM_LOOTANALYSIS_VIEW` (`LootAnalysis.View`) — Open Loot Analysis form
- `PERM_LOOTANALYSIS_RUN` (`LootAnalysis.Run`) — Run loot analysis scans on server

## Report Features

- **Categorized Counts**: Groups spawned items by weapons, ammo, medical, food, tools, clothing, vehicles, and custom categories.
- **Comparison to `db/types.xml`**: Compares current world counts against configured `nominal`, `min`, and `cost` limits.
- **Export Options**: Generates formatted CSV/JSON summary reports in `$profile:COT/LootAnalysis/`.
