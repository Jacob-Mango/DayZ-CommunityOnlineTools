---
name: RPC System
description: How client-server RPC works in COT: enum offsets, ScriptRPC write/send, ParamsReadContext read, full flow example, all known RPC ranges
type: project
---

## Overview

COT uses DayZ's `ScriptRPC` system. Each module owns a numeric range of RPC IDs defined by an enum with a unique offset. The module's `OnRPC` dispatches to private handler methods.

## All Known RPC Ranges (from RPC.c)

| Enum | Start | Module |
|------|-------|--------|
| `JMClientRPC` | 10100 | Client refresh/sync |
| `JMRoleRPC` | 10120 | Role management |
| `JMPermissionRPC` | 10130 | Permissions |
| `JMCOTBaseRPC` | 10135 | COT base |
| `JMCOTRPC` | 10140 | COT active state |
| `JMCameraModuleRPC` | 10160 | Camera module |
| `JMVehiclesModuleRPC` | 10180 | Vehicles module |
| `JMItemSetSpawnerModuleRPC` | 10200 | Item spawner |
| `JMObjectSpawnerModuleRPC` | 10220 | Object spawner |
| `JMTeleportModuleRPC` | 10240 | Teleport module |
| `JMVehicleSpawnerModuleRPC` | 10260 | Vehicle spawner |
| `JMWeatherModuleRPC` | 10280 | Weather module |
| `JMESPModuleRPC` | 10300 | ESP module |
| `JMPlayerModuleRPC` | 10320 | Player module (large range, 10320–10400) |
| `JMNamalskEventManagerRPC` | 10400 | Namalsk events |
| `JMCommandModuleRPC` | 10420 | Command module |
| `JMWebhookCOTModuleRPC` | 10440 | Webhook module |
| `JMLoadoutModuleRPC` | 10460 | Loadout module |
| `JMBanModuleRPC` | 10800 | Ban module |

**New modules: pick an offset well above 10800 (e.g. 10900, 11000, ...)**

## RPC Enum Pattern

```c
enum JMMyModuleRPC
{
    INVALID = 10900,    // ← unique starting offset

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
override int GetRPCMin() { return JMMyModuleRPC.INVALID; }
override int GetRPCMax() { return JMMyModuleRPC.COUNT; }

override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
{
    switch ( rpc_type )
    {
    case JMMyModuleRPC.RequestData:  RPC_RequestData( ctx, sender, target ); break;
    case JMMyModuleRPC.DoAction:     RPC_DoAction( ctx, sender, target );    break;
    case JMMyModuleRPC.Data:         RPC_Data( ctx, sender, target );        break;
    }
}
```

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

    if ( !GetPermissionsManager().HasPermission( "Admin.MyModule.View", sender ) )
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
// target=NULL → server
// to=NULL     → broadcast to all clients
// to=identity → specific player only
```

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
