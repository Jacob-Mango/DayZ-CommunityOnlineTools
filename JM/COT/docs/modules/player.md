---
name: Player Module
description: Core player management overview, linking to player-flows.md deep dive, player state toggles, stats, heal, strip, and spectate
type: project
---

## Overview

The Player Module (`JMPlayerModule`) is the central module for player administration. It handles online player listing, player state modifications (godmode, freeze, invisibility, unconsciousness, bleeding, sickness), stat adjustments (health, blood, water, energy, stamina, heat), spectating, inventory inspection, and player action triggers (heal, strip, dry, kill, kick, ban).

For an in-depth breakdown of `JMPlayerForm` roster widgets, tab structures, spectating camera flow, and login ban checks, see [player-flows.md](player-flows.md).

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c` — Main player management module, RPC execution, spectator setup, server stat clamping
- `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c` — Master player form container holding roster list and action tabs

## RPC Range

Range: `10340` – `10394` (`JMPlayerModuleRPC` in `RPC.c`)

| ID | Name | Direction | Description |
|---|---|---|---|
| `10340` | `SetGodMode` | Client → Server | Toggles god mode on target player |
| `10341` | `SetFreeze` | Client → Server | Toggles movement freeze state |
| `10342` | `SetInvisibility` | Client → Server | Toggles character mesh invisibility |
| `10343` | `SetHealth` | Client → Server | Sets player health slider value |
| `10344` | `SetBlood` | Client → Server | Sets player blood slider value |
| `10345` | `SetShock` | Client → Server | Sets player shock slider value |
| `10346` | `SetEnergy` | Client → Server | Sets player energy/food value |
| `10347` | `SetWater` | Client → Server | Sets player water/thirst value |
| `10348` | `SetStamina` | Client → Server | Sets player max stamina |
| `10350` | `Heal` | Client → Server | Fully restores health, blood, water, energy, and cures bleeding/sickness |
| `10351` | `Strip` | Client → Server | Removes all clothing and inventory items from target player |
| `10352` | `Kill` | Client → Server | Slays target player character |
| `10353` | `StartSpectating` | Client ↔ Server | Initiates spectator camera focus on target player |
| `10354` | `EndSpectating` | Client ↔ Server | Terminates spectating session and returns admin control |

## Permissions

- `PERM_PLAYER_VIEW` (`Player.View`) — Open player management form
- `PERM_PLAYER_GOD` (`Player.God`) — Toggle godmode state
- `PERM_PLAYER_FREEZE` (`Player.Freeze`) — Freeze/unfreeze player movement
- `PERM_PLAYER_INVISIBLE` (`Player.Invisible`) — Toggle character invisibility
- `PERM_PLAYER_HEAL` (`Player.Heal`) — Restore player stats / cure conditions
- `PERM_PLAYER_KILL` (`Player.Kill`) — Kill player character
- `PERM_PLAYER_SPECTATE` (`Player.Spectate`) — Spectate target player
