---
name: RPC System
description: How client-server RPC works in COT: enum offsets, ScriptRPC write/send, ParamsReadContext read, full flow example, all known RPC ranges
type: project
---

## Overview

COT uses DayZ's `ScriptRPC` system. Each module owns a numeric range of RPC IDs defined by an enum with a unique offset. The module's `OnRPC` dispatches to handler methods (`RPC_<Name>`, `protected`); a client -> server admin action can instead be a
`JMModuleAction` and skip the hand-written handler (see [module-creation.md](module-creation.md), Step 1b).

## All Known RPC Ranges (from RPC.c)

| Enum | Start | Last id | Module |
|------|-------|---------|--------|
| `JMClientRPC` | 10100 | 10108 | Client refresh/sync (roster batch updates) |
| `JMRoleRPC` | 10120 | 10123 | Role management |
| `JMPermissionRPC` | 10130 | 10130 | Permissions (reserved, no ids yet) |
| `JMCOTBaseRPC` | 10135 | 10136 | COT base (`TransportSync`) |
| `JMCOTRPC` | 10140 | 10141 | COT active state |
| `JMCameraModuleRPC` | 10160 | 10164 | Camera module |
| `JMItemSetSpawnerModuleRPC` | 10200 | 10203 | Item spawner (enum still in `RPC.c`) |
| `JMObjectSpawnerModuleRPC` | 10220 | 10223 | Object spawner |
| `JMTeleportModuleRPC` | 10240 | 10247 | Teleport module |
| `JMVehicleSpawnerModuleRPC` | 10260 | 10262 | Vehicle spawner (enum still in `RPC.c`) |
| `JMESPModuleRPC` | 10300 | 10321 | ESP module |
| `JMPlayerModuleRPC` | 10340 | 10394 | Player module (large range; moved from 10320 when ESP grew into it) |
| `JMNamalskEventManagerRPC` | 10400 | 10404 | Namalsk events |
| `JMCommandModuleRPC` | 10420 | 10421 | Command module |
| `JMWebhookCOTModuleRPC` | 10440 | 10447 | Webhook module |
| `JMLoadoutModuleRPC` | 10460 | 10466 | Loadout module |
| `JMCompensationsModuleRPC` | 10480 | 10485 | Compensations module |
| `JMRoleManagerModuleRPC` | 10500 | 10505 | Role Manager module |
| `JMLootAnalysisModuleRPC` | 10520 | 10525 | Loot Analysis module |
| `JMEventsModuleRPC` | 10540 | 10545 | Events module |
| `JMTerritoryModuleRPC` | 10560 | 10563 | Territory module (Expansion base building) |
| `JMWeatherModuleRPC` | 10600 | 10623 | Weather module (moved from 10280 - its last id equalled `JMESPModuleRPC.Log`) |
| `JMVehiclesModuleRPC` | 10650 | 10670 | Vehicles module (moved from 10180 - its ids reached the Item spawner range) |
| `JMBuildingsModuleRPC` | 10700 | - | Third party: `COT_BaseBuildingManager` (not defined in this repo) |
| `JMKillFeedModuleRPC` | 10720 | - | Third party: `COT_PvPInspector` (not defined in this repo) |
| `JMAnimalsModuleRPC` | 10740 | - | Third party: `COT_AnimalManager` (not defined in this repo) |
| `JMXMLEditorModuleRPC` | 10760 | - | Third party: `Mu_XMLEditor` (not defined in this repo) |
| `JMBanModuleRPC` | 10800 | 10805 | Ban module |
| `JMMapEditorModuleRPC` | 10900 | 10912 | Map Editor module |
| `JMAntiCheatModuleRPC` | 10920 | 10923 | Anti-Cheat module |
| `JMServerStatsModuleRPC` | 10940 | 10941 | Server Stats module |
| `JMActionHistoryModuleRPC` | 10960 | 10962 | Action History module |

`JMESPObjectAction`, `JMStatType`, `JMInventoryModifyOp` and `JMInventoryGroupOp` in `RPC.c` are payload enums (an
action id or an operation carried *inside* an RPC), not RPC ranges. All of them are append-only wire values.

**New modules: pick an offset above the highest id in use (10962 today) - 11000, 11100, ... - and leave 100 free after the last id.**
The 10700-10799 block is held by the third-party addons in the table above - keep it clear. The "Last id" column is
what `CheckRPCRanges.ps1` computes (`INVALID` plus the number of entries); the ranges are re-derived from the code on every run.

Ranges are hand-allocated, so they are checked: run `Workbench/Batchfiles/CheckRPCRanges.ps1` after adding or growing an enum. It fails
when two `...RPC` enums claim the same id. (Ranges that touch are why Weather and Vehicles moved: an enum that grows by one id
silently starts delivering another module's messages to itself.)

## RPC Enum Pattern

```c
enum JMMyModuleRPC
{
    INVALID = 11000,    // ← unique starting offset

    // Client → Server requests
    RequestData,
    DoAction,

    // Server → Client responses
    Data,
    ActionResult,

    COUNT               // ← upper bound for GetRPCMax()
}
```

## Module RPC Wiring

```c
// In DescribeModule( JMModuleInfo info ) - feeds GetRPCMin() / GetRPCMax()
info.SetRPCRange( JMMyModuleRPC.INVALID, JMMyModuleRPC.COUNT );

override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
{
    switch ( rpc_type )
    {
    case JMMyModuleRPC.RequestData:  RPC_RequestData( ctx, sender, target ); break;
    case JMMyModuleRPC.DoAction:     RPC_DoAction( ctx, sender, target );    break;
    case JMMyModuleRPC.Data:         RPC_Data( ctx, sender, target );        break;
    default:                         RunAction( sender, rpc_type, ctx );     break;   // JMModuleAction ids
    }
}
```

`GetRPCMin()` / `GetRPCMax()` can still be overridden directly (`JMCommandModule` and `JMEntityManagerModule` do); an override
wins over the descriptor.

## Full Client → Server → Client Flow

### 1. Client sends request
```c
void RequestData( string filter )
{
    ScriptRPC rpc = new ScriptRPC();
    rpc.Write( filter );
    rpc.Send( NULL, JMMyModuleRPC.RequestData, true, NULL );
    //        ^to server  ^type            ^reliable ^sender=auto
}
```

### 2. Server receives and responds
```c
private void RPC_RequestData( ParamsReadContext ctx, PlayerIdentity sender, Object target )
{
    if ( !IsMissionHost() ) return;   // server-only guard

    if ( !JMPermissions.HasRPC( JMConstants.PERM_MYMODULE_VIEW, sender ) )
        return;

    string filter;
    if ( !ctx.Read( filter ) ) return;

    // Build and send response
    array<string> results = GetFilteredData( filter );
    ScriptRPC rpc = new ScriptRPC();
    rpc.Write( results.Count() );
    foreach ( string item : results )
        rpc.Write( item );
    rpc.Send( NULL, JMMyModuleRPC.Data, true, sender );
    //              ^type              ^reliable ^specific client
}
```

### 3. Client receives response
```c
private void RPC_Data( ParamsReadContext ctx, PlayerIdentity sender, Object target )
{
    if ( IsMissionHost() ) return;    // client-only guard

    int count;
    if ( !ctx.Read( count ) ) return;

    array<string> results = new array<string>();
    for ( int i = 0; i < count; i++ )
    {
        string item;
        if ( !ctx.Read( item ) ) break;
        results.Insert( item );
    }

    JMMyForm form;
    if ( Class.CastTo( form, GetForm() ) )
        form.PopulateList( results );
}
```

## ScriptRPC Reference

```c
ScriptRPC rpc = new ScriptRPC();

// Write types (must match Read order exactly)
rpc.Write( int_val );
rpc.Write( string_val );
rpc.Write( float_val );
rpc.Write( bool_val );
rpc.Write( vector_val );
rpc.Write( Object_ref );

// Send
rpc.Send( Object target, int rpc_type, bool reliable, PlayerIdentity to );
// target=NULL → no specific target
// target=obj → specific target
// to=NULL     → broadcast to all clients
// to=identity → specific player only
```

> When passing a target for an RPC sent from server to client, consider if
> the target is in client's netbubble (if the RPC is a reaction to what a
> client requested, this is usually the case). A target sent by a client
> RPC is usually guaranteed to exist on the server, but there may be races
> where the target is deleted on server before the client knows about the
> deletion. Targeted RPCs therefore should always check if target is NULL
> in their handler.

```c
// Reading in handler
int val;
bool ok = ctx.Read( val );    // false = failed / out of data
string s;
ctx.Read( s );
```

## Important Guards

| Guard | Where to use |
|-------|-------------|
| `if ( !IsMissionHost() ) return;` | Top of every server-side RPC handler |
| `if ( IsMissionHost() ) return;` | Top of every client-side RPC handler |
| `if ( !ctx.Read( x ) ) return;` | Every Read call — never use unread data |
| Permission check | Server side, after host guard, before any action |

**Never trust client-side permission state for server actions — always re-check on server.**

## Targeted vs untargeted RPCs

If we only have the network ID on client, don't send a targeted RPC (i.e. netid might have been acquired previously but object it belongs to moved out of client netbubble, so client object reference is then NULL and netid is all we have left to go by)

**pattern** — send untargeted, carry the network id in the payload:

```c
// sender
int netLow, netHigh;
target.GetNetworkID( netLow, netHigh );

ScriptRPC rpc = new ScriptRPC();
rpc.Write( netLow );
rpc.Write( netHigh );
rpc.Write( position );
rpc.Send( NULL, JMESPModuleRPC.SetPosition, true, NULL );
```

```c
// handler
int netLow, netHigh;
if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
    return;

Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
if ( !obj )
    return;
```

This is already the established pattern in the mod — `JMPlayerModule`'s
spectate-by-GUID RPC, `JMVehiclesModule` throughout, and `JMESPModule.RPC_DeleteObject`.

> **Known exceptions:** Do not change existing (targeted) RPC code.

## Avoid RPC spam from continuous input

Anything driven by a held mouse button, a drag, a slider, or a per-frame update
will fire at frame rate if you let it. Rules:

- **Never send from a per-frame handler unthrottled.** Move locally for the visual,
  and send on a timer. `JMESPWidgetHandler` uses `DRAG_SEND_MS = 100` plus a
  minimum-delta check, so a slow drag that barely moves sends nothing at all.
- **Always commit the final state once when the gesture ends.** The throttle will
  usually swallow the last movement, so the end of the gesture (mouse-up) must
  send the exact final value. Do not rely on the mid-gesture stream being complete.
- **Prefer a local preview over a server round-trip.** The client can move the
  entity for itself every frame; only the authoritative commit needs the network.
  For a physics body, zero the velocities when previewing or the local solver
  drags it back out of place between frames.
- **Watch what each server-side RPC actually costs.** A cheap-looking 10/sec
  transform stream on a vehicle drives `ForceTransportPositionAndOrientation`
  (pause body, teleport, `Synchronize()`, restore velocity) ten times a second,
  plus a webhook post and a log write per accepted call unless separately
  throttled (see `ShouldRecordMove`).

### The rate limiter will silently eat your second RPC

`JMModuleBase.OnRPC` drops any RPC arriving within `JMConstants.RPC_RATE_LIMIT_S`
(100 ms) of that sender's previous one, keyed by sender GUID only — **not** by
rpc id — and returns without reading `ctx` (the sender gets an
`STR_COT_NOTIFICATION_RATE_LIMITED` notification). So two RPCs sent in the same frame
mean the second is discarded, however unrelated they are.

Note this gate lives in `JMModuleBase.OnRPC` and only applies if your module's
`OnRPC` override actually calls `super.OnRPC(...)`. Almost no module does - deliberately, because one click routinely
sends a burst of RPCs to a module and the gate would drop all but the first. Throttle handlers that cost disk or HTTP with
`JMRPCThrottle` instead, which is keyed per action.

### Debugging a crash with no script stack trace

A native crash (segfault with memory dump) produces no or potentially
misleading Enforce stack trace, so reasoning from the dump alone can mislead
you about which script path is at fault. Instrument and reproduce:

- Bracket suspicious calls with `Print()` before and after. A "before" with no
  matching "after" could be the killing call, but note that it can also be the
  result of much earlier memory corruption. Hard and fast rule, origin of a
  segfault is not always necessarily the script code that was executed
  immediately before it. If it's just an exception (crash log) without segfault
  and memdump, this method is much more accurate in determining the cause.
- Put at least one print **before** any early return or `ctx.Read`, so that its
  absence proves the handler was never entered rather than that it bailed out.
- A negative result is valuable — proving a path never executes eliminates it.
  That is what identified this bug, after three fixes aimed at code that was
  never running.
