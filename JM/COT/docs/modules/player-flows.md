---
name: Player Form, Spectating, and Ban Flow
description: JMPlayerForm player roster and tabs, stat slider ranges, spectating system internals, how the player list stays updated, ban lookup at login, PluginAdminLog webhook hooks
type: project
---

## JMPlayerForm - roster and tabs

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c`

The form is a shell: it owns the selection (`JMPlayerInstance m_SelectedInstance`), the tab strip and the module. Everything
else is a class it holds (see [../systems/module-folder-layout.md](../systems/module-folder-layout.md)).

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

### Stat sliders - ranges

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

## Spectating System

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c`

### Start Spectating Flow

1. **Client:** `StartSpectating( string guid )` (works for players outside the net bubble) or `StartSpectating( Object )`
   - Blocked while in a vehicle; needs `PERM_PLAYER_SPECTATE`
   - Remembers the admin (`m_SpectatorClient`), `COT_TempDisableOnSelectPlayer()`, `COT_RememberVehicle()`
   - Sends `JMPlayerModuleRPC.StartSpectating`: the GUID variant writes the GUID and sends untargeted; the Object variant still
     sends targeted (see the known exception in [../systems/rpc.md](../systems/rpc.md))

2. **Server:** `RPC_StartSpectating` -> `Server_StartSpectating( guid | object, ident )`; the `guid` overload resolves the player and
   calls the `Object` one, which holds the body (DayZ-Expansion AI overrides it, see
   [../systems/deprecations.md](../systems/deprecations.md)). It is `Server_`, not `Exec_`: it only runs on the server and has the
   `Client_StartSpectating` counterpart ([../systems/naming.md](../systems/naming.md))
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
file: it returns whether a timed ban was marked expired (see [../systems/deprecations.md](../systems/deprecations.md)).

### JMPlayerBan Structure

```c
class JMPlayerBan
{
    string SteamID;         // canonical ID for lookups (never PlayerName)
    string PlayerName;      // display name at ban time
    string Message;         // message shown on kick screen
    int BannedAt;           // unix timestamp when banned
    int BanDuration;        // -1 = permanent, else absolute expiry unix timestamp
    string IssuedBy;        // admin steam ID
    string IssuedByName;    // admin display name

    bool IsPermanent()      { return BanDuration == -1; }
    bool IsActive()         { /* check current time vs BanDuration */ }
    string GetExpiryString() { /* returns "3d 2h 15m" / "Permanent" / "Expired" */ }
}
```

Stored in one `Bans.json` via `JsonFileLoader<JMPlayerBanStore>`; `JMPlayerBanStore` has `array<ref JMPlayerBan> Bans`.
`JMPlayerBan.Load` matches on SteamID (plain ID) or GUID. `JMBanModule.Ban()` takes a duration in **relative seconds** and
stores the absolute expiry; it also prunes expired bans once a minute and offers `Unban` and `EditBanDuration`. (The old
one-file-per-ban layout under `JMConstants.DIR_BANS` is gone; `JMPlayerBan.Save` is a deprecated forwarder to the module.)

## PluginAdminLog - Webhook Hooks

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Plugins/PluginAdminLog.c`

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
