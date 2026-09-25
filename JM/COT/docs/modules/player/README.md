---
name: Player Module
description: Core player management overview, roster tabs, stat sliders, spectating system, player flows, and ban checks
type: project
---

## Overview

The Player Module (`JMPlayerModule`) is the central module for player administration. It handles online player listing, player state modifications (godmode, freeze, invisibility, unconsciousness, bleeding, sickness), stat adjustments (health, blood, water, energy, stamina, heat), spectating, inventory inspection, and player action triggers (heal, strip, dry, kill, kick, ban).

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

---

## JMPlayerForm - Roster and Tabs

File: `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c`

The form is a shell: it owns the selection (`JMPlayerInstance m_SelectedInstance`), the tab strip and the module. Everything else is a class it holds (see [../../conventions/module-folder-layout.md](../../conventions/module-folder-layout.md)).

**Left panel - `JMPlayerRoster`** (`Components/JMPlayerRoster.c`, filter in `JMPlayerRosterFilter.c`):

- A **fixed pool of 200** `JMPlayerRowWidget`s, reused on every rebuild (grid rows `Content_Row_00`, `Content_Row_01`, ...).
  A live player gets `SetPlayer( guid )`, an empty slot `SetPlayer( "" )`.
- Grouped **by role**: a header per role, its members underneath, and the header collapses the group with a ~160 ms
  fold animation. Grouping runs after filtering, so a header's count is what is actually under it. Collapsed roles are
  remembered by name (the row pool is reused, so a row index would be wrong after a refresh).
- Header stats: online (against the server's slot count where the client knows it), session peak, rows shown after the
  filter, rows ticked.
- Search box (`UIActionSearchBox m_PlayerListFilter`) filters name/GUID; sort toggle A-Z / Z-A; select all / deselect all;
  two selection-preset slots (`m_PlayersPref1/2`, save / load).
- `UpdatePlayerList( force )` polls every 1500 ms. It asks the server for the roster (`RefreshClients`), but rebuilds the
  rows only when `GetPermissionsManager().GetRosterVersion()` changed or `force` is true, so an unchanged roster is a cheap no-op.
- The right panel is disabled when no player is selected. The first player is auto-selected when the form opens
  (`m_AutoSelect = true`).

**Right panel - tabs** (each a `JMFormTab` class under `Tabs/<Name>/`, created the first time it is shown; ids come from the
tab strip, never numbered):

| Tab | Class | Holds |
|---|---|---|
| General | `JMPlayerFormTabGeneral` | identity (name, GUID, Steam64ID, role, CFTools / profile links), status badges (frozen, god mode, invisible, unconscious, bleeding, sick, ...), stat sliders + Apply, temperature card, toggles (the `JMPlayerToggle` registry) |
| Position | `JMPlayerFormTabPosition` | X/Y/Z editable text (live or synced source), copy / paste, map, teleport to / from the player, teleport history |
| Inventory | `JMPlayerFormTabInventory` | inventory grid of the selected player, take / delete / repair / modify / group operations |
| Statistics | `JMPlayerFormTabStatistics` | session and combat statistics (kills, deaths, headshots, hit ratio, playtime, ...), plus an Expansion section (faction, group, money, reputation) |
| Actions | `JMPlayerFormTabActions` | Heal, Strip, Dry, Kill, Repair, Vomit, Scale, Spectate, add / remove diseases, bleeding parts, Message, Kick, Ban |

Roles and per-permission editing are done in the Role Manager module (`JMRoleManagerModule`), not in this form.

### Stat Sliders - Ranges

The stat sliders are rows of the `JMPlayerStat` registry (`JMPlayerModule.RegisterStats()`); the server clamps to the
`JMConstants.STAT_*_MIN/MAX` range regardless of what the slider allows.

| Slider | Slider range | Server clamp | Notes |
|--------|-----|-----|-------|
| Health | 0-100 | 0-100 | |
| Blood | 0-5000 | 0-5000 | |
| Energy | 0-`PlayerConstants.SL_ENERGY_MAX` | 0-20000 | |
| Water | 0-`PlayerConstants.SL_WATER_MAX` | 0-5000 | |
| Shock | 0-100 | 0-100 | |
| Stamina | 0-`CfgGameplayHandler.GetStaminaMax()` | 0-100 | |
| HeatBuffer | 0-3, step 0.1 (sent x10) | 0-30 | |
| HeatComfort | -0.75-0.75 | - | **READ-ONLY** (display only, `Disable()`d) |

---

## Spectating System

File: `Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c`

### Start Spectating Flow

1. **Client:** `StartSpectating( string guid )` (works for players outside the net bubble) or `StartSpectating( Object )`
   - Blocked while in a vehicle; needs `PERM_PLAYER_SPECTATE`
   - Remembers the admin (`m_SpectatorClient`), `COT_TempDisableOnSelectPlayer()`, `COT_RememberVehicle()`
   - Sends `JMPlayerModuleRPC.StartSpectating`: the GUID variant writes the GUID and sends untargeted; the Object variant still
     sends targeted (see [../../codebase/rpc-design.md](../../codebase/rpc-design.md))

2. **Server:** `RPC_StartSpectating` -> `Server_StartSpectating( guid | object, ident )`; the `guid` overload resolves the player and
   calls the `Object` one, which holds the body (DayZ-Expansion AI overrides it). It is `Server_`, not `Exec_`: it only runs on the server and has the
   `Client_StartSpectating` counterpart ([../../conventions/naming-conventions.md](../../conventions/naming-conventions.md))
   - Checks `PERM_PLAYER_SPECTATE`; refuses self-spectate (`CANT_SPECTATE_SELF` notification)
   - Stores `m_Spectators[ident.GetId()] = playerSpectator`; `SetLastPosition()`; `COT_RememberVehicle()`
   - Sets `playerSpectator.m_JM_SpectatedObject = spectateObject`
   - `g_Game.SelectPlayer( ident, NULL )` + `g_Game.SelectSpectator( ident, "JMSpectatorCamera", headPosition )`
   - Enables godmode: `COTSetGodMode( true, false )` (does not overwrite the saved preference)
   - Answers the client with the target's network id (`networkLow`, `networkHigh`), untargeted

3. **Client:** `RPC_StartSpectating` (client branch) -> `Client_Check_StartSpectating( networkLow, networkHigh )`
   - Polls every 34 ms until the spectator camera and the target object both exist, then `Client_StartSpectating( object )`:
     `CurrentActiveCamera.SelectedTarget( object )`, `SetActive( true )`, and `GetPlayer().GetInputController().SetDisabled( true )`
   - The camera is `JMSpectatorCamera`; the previous camera is kept in `COT_PreviousActiveCamera`

### End Spectating Flow

1. Client `EndSpectating()` sends `JMPlayerModuleRPC.EndSpectating` (multiplayer only, needs `PERM_PLAYER_SPECTATE`)
2. **Server:** `Exec_EndSpectating( ident )` -> `Server_EndSpectating`: clears `m_JM_SpectatedObject`, removes the entry from
   `m_Spectators`, logs "Stopped spectating". If the admin had no freecam position it resets the spectator
   (`COTResetSpectator()`) and reselects the player (immediately, or after a 250 ms / 5000 ms wait depending on whether the
   last position is water). Replies with `(switchToPreviousCamera, waitForPlayerIdleTimeout)`
3. **Client:** `Client_EndSpectating()` deletes the spectator camera, restores `COT_PreviousActiveCamera` if it was a
   `JMCinematicCamera`, otherwise resets DOF and re-enables the input controller

End spectating keyboard shortcut: `UAPlayerModuleStopSpectating` (default: Numpad 1, bound to `EndSpectating`).

Camera positions are synced to the server via `UpdateSpectatorPositions()`, called every 1 s from `MissionServer.OnUpdate()`.
Object lookup by network ID: `g_Game.GetObjectByNetworkId( networkLow, networkHigh )`.

---

## Player List - How It Stays Updated

Server push flow (triggered by the roster's `UpdatePlayerList()`):

1. `JMPlayerRoster.UpdatePlayerList()` calls `GetCommunityOnlineTools().RefreshClients()` (client only, not offline)
2. Client sends `JMClientRPC.RefreshClients` to the server
3. Server `RPC_RefreshClients()` (`CommunityOnlineTools.c`) checks `PERM_PLAYER_READ` (`Admin.Player.Read`), then for each player:
   - calls `player.Update()` to refresh cached state
   - writes the player's GUID + `OnSend( ctx )` (permissions/roles/state) into **one** batched RPC
   - sends `JMClientRPC.UpdateClientBatch` to the requesting admin only
   (position-only refresh: `RefreshClientPositions` -> `UpdateClientPositionBatch`, gated on `PERM_PLAYER_TELEPORT_POSITION`)
4. Client `RPC_UpdateClientBatch()` stores the data through `GetPermissionsManager().UpdatePlayer()`, which bumps its roster version
5. The roster sees the new version on its next poll and rebuilds the rows

`JMClientRPC` ids (from `INVALID = 10100`): `RefreshClients`, `RefreshClientPositions`, `RemoveClient` (server -> client: player
disconnected), `UpdateClient` (full player data, single), `UpdateClientPosition`, `SetClient` (your own data),
`UpdateClientBatch`, `UpdateClientPositionBatch`. Ids are wire values and append-only - the two batch entries were added last
and replace the old per-player unicast loop; the single-player `UpdateClient` still exists for one-off updates.

---

## Ban System - Full Flow

Two separate things share the word "ban":

- **The ban record** - `Bans.json`, owned by `JMBanModule` (Ban Manager). Written by the Ban Manager form and by the Anti-Cheat
  form (`JMBanModule.Ban( steamID, name, message, seconds, issuedBy, issuedByName )`), checked at login.
- **The Player module's Ban action** - `JMPlayerModule.Ban( guids, message, duration )` from the Player form, the row menu
  and the ESP action menu.

> **The Player module's Ban currently only kicks.** `JMPlayerModule.Exec_Ban` skips the sender and other staff (holders of
> the `"COT"` node), sends the ban message and disconnects the player 500 ms later (`Exec_Ban_Single`), then logs and
> webhooks it as `Ban`. It never calls `JMBanModule.Ban()`, so no record reaches `Bans.json` and the player can reconnect.
> `duration` is accepted but unused. Use the Ban Manager (or call `JMBanModule.Ban` from `Exec_Ban_Single`) to make a ban stick.

### At Login (MissionServer.OnEvent)

```c
case ClientNewEventTypeID:
    if ( IsCOTBanned( identity ) )
    {
        OnClientDisconnectedEvent( identity, NULL, 0, true );  // kick
        return;
    }
```

### IsCOTBanned Check

```c
protected bool IsCOTBanned( PlayerIdentity identity )
{
    JMPlayerBan banData = JMPlayerBan.Load( identity.GetId(), identity.GetPlainId() );

    if ( banData )
    {
        if ( banData.BanDuration > 0 )
        {
            if ( CF_Date.Now( true ).GetTimestamp() > banData.BanDuration )
            {
                JMPlayerBan.DeleteBanFile( guid, steamid );
                return false;   // expired -> allow entry
            }
        }
        // active ban -> kick
        return true;
    }
    return false;
}
```

`JMPlayerBan.Load` is a static that reads `$profile:CommunityOnlineTools\Bans\Bans.json` directly, so it works before
`JMBanModule` has loaded. `IsActive()` on the record does the time check for the lookup. `DeleteBanFile()` no longer deletes a
file: it returns whether a timed ban was marked expired.

---

## PluginAdminLog - Webhook Hooks

File: `Scripts/4_World/CommunityOnlineTools/Plugins/PluginAdminLog.c`

Hooks override parent `PluginAdminLog` methods:

| Method | Webhook type | Content |
|--------|-------------|---------|
| `OnPlacementComplete(player, item)` | `"TerritoryDeployed"` or `"ItemDeployed"` | Item class, position |
| `PlayerList()` | `"PlayerCount"` | Current online count |
| `PlayerKilled(player, source)` | `"PlayerDeath"` (or `"PlayerDeathNoPos"` when the position is left out) | Cause (thirst/hunger/blood/suicide/weapon/explosion), position, killer, distance, damage breakdown |
| `PlayerHitBy(...)` | `"PlayerDamage"` | Damage type (melee/firearm/explosion/stun/fall), health/blood/shock delta, zone, ammo |

Webhook message format:
```c
auto msg = m_Webhook.CreateDiscordMessage();
msg.GetEmbed().AddField( "Field Name", "value", false );
msg.GetEmbed().SetColor( 16711680 );  // red for deaths
m_Webhook.Post( "WebhookType", msg );
```

`CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_* )` starts a message with one of the palette colours instead.
Webhook types are declared per module (`JMModuleInfo.AddWebhookType`) or by `JMWebhookConstructor` for the built-in connections;
each connection in the settings subscribes to specific type strings (e.g. a "deaths" webhook vs a "general" webhook).
