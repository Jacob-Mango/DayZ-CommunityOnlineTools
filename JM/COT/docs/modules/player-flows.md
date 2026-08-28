---
name: Player Form, Spectating, and Ban Flow
description: JMPlayerForm player list population, stat slider values, spectating system internals, ban file lookup at login, PluginAdminLog webhook hooks
type: project
---

## JMPlayerForm — Player List

File: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerForm.c`

- Creates **200 pre-allocated** `JMPlayerRowWidget` instances in a scrollable grid on `OnInit()`
- Filter: `UIActionEditableTextPreview m_PlayerListFilter` — live filter on name/GUID
- Sort toggle: A→Z / Z→A button
- Auto-selects first player when form opens (`m_AutoSelect = true`)
- Selected instance stored in `JMPlayerInstance m_SelectedInstance`

### Stat Sliders — Ranges

| Slider | Min | Max | Notes |
|--------|-----|-----|-------|
| Health | 0 | 100 | |
| Blood | 0 | 5000 | |
| Energy | 0 | `PlayerConstants.SL_ENERGY_MAX` | |
| Water | 0 | `PlayerConstants.SL_WATER_MAX` | |
| Shock | 0 | 100 | |
| Stamina | 0 | `CfgGameplayHandler.GetStaminaMax()` | |
| HeatBuffer | 0 | 3 | step 0.1 |
| HeatComfort | -0.75 | 0.75 | **READ-ONLY** (display only) |

Each slider has a corresponding `m_HealthUpdated`, `m_BloodUpdated`, etc. flag to avoid sending redundant RPCs.

### Right Panel Layout
- Identity: Name, GUID, Steam64ID (as copyable UIActionText)
- Position: X/Y/Z editable text with Apply button
- Stats: sliders (above)
- Flags: checkboxes (GodMode, Freeze, Invisible, UnlimitedAmmo, AdminNVG, UnlimitedStamina, BrokenLegs, ReceiveDmgDealt, CannotBeTargetedByAI, RemoveCollision)
- Actions: Heal, StopBleeding, Strip, Dry, Kill, Vomit, SetScale
- Teleport: TeleportToMe, TeleportMeTo, TeleportToPrevious
- Moderation: Message, Kick, Ban (with duration dropdown)
- Permissions: Roles + per-permission tree

Both panels disabled when no player is selected.

## Spectating System

Files: `JM/COT/Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c`

### Start Spectating Flow

1. **Client:** calls `StartSpectating(guid)`
   - Validates: not in vehicle, target exists
   - Sends RPC: `JMPlayerModuleRPC.StartSpectating`

2. **Server:** `Server_StartSpectating(ident, targetGUID)`
   - Looks up spectate target by GUID
   - Prevents self-spectate
   - Stores: `m_Spectators[ident.GetId()] = playerSpectator`
   - Sets: `playerSpectator.m_JM_SpectatedObject = spectateObject`
   - Calls: `g_Game.SelectPlayer()` + `g_Game.SelectSpectator()`
   - Enables godmode: `COTSetGodMode(true, false)`

3. **Client:** `Client_StartSpectating()`
   - Saves current camera to `COT_PreviousActiveCamera`
   - Sets spectate target: `CurrentActiveCamera.SelectedTarget = spectateObject`
   - Activates camera: `CurrentActiveCamera.SetActive(true)`
   - Disables input: `GetPlayer().GetInputController().SetDisabled(true)`
   - Uses `JMSpectatorCamera` class

### End Spectating Flow

1. Client sends `JMPlayerModuleRPC.EndSpectating` RPC
2. **Server:** clears `m_JM_SpectatedObject`, removes from `m_Spectators` map
3. **Client:** `Client_EndSpectating()`
   - Waits 250ms polling for player idle
   - Restores `COT_PreviousActiveCamera` if available
   - Re-enables input controller

End spectating keyboard shortcut: `UAPlayerModuleStopSpectating` (default: Numpad 1)

Camera positions synced to server via `UpdateSpectatorPositions()` called every 1s from `MissionServer.OnUpdate()`. Object lookup by network ID: `g_Game.GetObjectByNetworkId(networkLow, networkHigh)`.

## Player List — How It Stays Updated

Server push flow (triggered by `RefreshClients` RPC):

1. `JMPlayerForm.UpdatePlayerList()` calls `GetCommunityOnlineTools().RefreshClients()`
2. Client sends `JMClientRPC.RefreshClients` (10101) to server
3. Server `RPC_RefreshClients()` checks `Admin.Player.Read` permission, then for each player:
   - Calls `player.Update()` to refresh cached state
   - Writes PlayerObject + GUID + `OnSend(ctx)` (permissions/roles) into one RPC
   - Sends `JMClientRPC.UpdateClient` (10103) to the requesting admin only
4. Client `RPC_UpdateClient()` stores data in `GetPermissionsManager()`
5. `JMPlayerForm` loops its 200 pre-allocated `JMPlayerRowWidget` slots, calls `SetPlayer(guid)` per live player, `SetPlayer("")` for empty slots

JMClientRPC enum offsets:
```c
enum JMClientRPC {
    INVALID = 10100,
    RefreshClients,          // 10101 — client → server
    RefreshClientPositions,  // 10102
    RemoveClient,            // 10103 — server → client: player disconnected
    UpdateClient,            // 10104 — server → client: full player data
    UpdateClientPosition,    // 10105 — server → client: position only
    SetClient,               // 10106 — server → client: your own data
    COUNT
}
```

`CreateActionRows(parent)` creates a pre-structured container with 100 named child grids (`Content_Row_00`…`Content_Row_99`) — used to display the 200-player list split into two action rows.

## Ban System — Full Flow

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
            int now = CF_Date.Now( true ).GetTimestamp();
            if ( now > banData.BanDuration )
            {
                JMPlayerBan.DeleteBanFile( guid, steamid );
                return false;   // expired → allow entry, clean up file
            }
        }
        // active ban → kick
        return true;
    }
    return false;
}
```

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

Stored via `JsonFileLoader<JMPlayerBanStore>`. `JMPlayerBanStore` has `array<ref JMPlayerBan> Bans`.
Lookup searches by SteamID (plain ID) and GUID.

## PluginAdminLog — Webhook Hooks

File: `JM/COT/Scripts/4_World/CommunityOnlineTools/Plugins/PluginAdminLog.c`

Hooks override parent `PluginAdminLog` methods:

| Method | Webhook type | Content |
|--------|-------------|---------|
| `OnPlacementComplete(player, item)` | `"TerritoryDeployed"` or `"ItemDeployed"` | Item class, position |
| `PlayerList()` | `"PlayerCount"` | Current online count |
| `PlayerKilled(player, source)` | `"PlayerDeath"` | Cause (thirst/hunger/blood/suicide/weapon/explosion), position, killer, distance |
| `PlayerHitBy(...)` | `"PlayerDamage"` | Damage type (melee/firearm/explosion/stun/fall), health/blood/shock delta, zone, ammo |

Webhook message format:
```c
auto msg = m_Webhook.CreateDiscordMessage();
msg.GetEmbed().AddField( "Field Name", "value", false );
msg.GetEmbed().SetColor( 16711680 );  // red for deaths
m_Webhook.Post( "WebhookType", msg );
```

Module webhook types are filtered — each connection in settings subscribes to specific type strings (e.g. a "deaths" webhook vs a "general" webhook).
