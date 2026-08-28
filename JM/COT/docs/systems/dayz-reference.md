---
name: DayZ Base Scripts Reference
description: Vanilla DayZ scripts at C:\Users\Guillaume\Documents\DayZ Projects\scripts — entity hierarchy, PlayerBase stats, inventory, weather API, entity creation ECE flags
type: reference
---

## Location

`C:\Users\Guillaume\Documents\DayZ Projects\scripts\`

This is the **vanilla DayZ game scripts** — the base API reference when writing mods. Not a mod itself.

## Layer System

| Layer | Path | Role |
|-------|------|------|
| 1_core | scripts/1_core/ | Engine constants, proto wrappers (C++ bindings) |
| 2_gamelib | scripts/2_gamelib/ | Abstract game framework (Game, InputManager) |
| 3_game | scripts/3_game/ | Core game systems (406 files) |
| 4_world | scripts/4_world/ | Game content & mechanics (2,093 files) |
| 5_mission | scripts/5_mission/ | Mission/UI (207 files) |

## Entity Hierarchy

```
EntityAI  (C++ engine base)
├── ItemBase           → scripts/4_world/entities/itembase/
├── ManBase            → scripts/4_world/entities/manbase/
│   └── PlayerBase     → scripts/4_world/entities/manbase/playerbase.c  (9510 lines)
├── Building           → scripts/4_world/entities/building/
└── Car / Transport    → scripts/4_world/entities/vehicles/
```

## Global Singletons

```c
GetGame()         // DayZGame singleton
GetPlayer()       // Local PlayerBase
GetWeather()      // Weather singleton (proto native from GetGame())
```

## PlayerBase — Stats System

File: `scripts/4_world/entities/manbase/playerbase.c`

Stats use a generic `PlayerStat<T>` wrapper:
```c
PlayerStats stats = GetPlayerStats();

// Typed stat access:
GetStatHealth()      // PlayerStat<float>  0–100
GetStatBlood()       // PlayerStat<float>  0–5000
GetStatShock()       // PlayerStat<float>  0–100
GetStatEnergy()      // PlayerStat<float>  0–MAX
GetStatWater()       // PlayerStat<float>  0–MAX
GetStatStamina()     // PlayerStat<float>
GetStatHeatBuffer()  // PlayerStat<float>
GetStatBloodType()   // PlayerStat<int>

// Setting values:
GetStatHealth().Set( 100.0 );
GetStatEnergy().Set( GetStatEnergy().GetMax() );

// Reading:
float hp = GetStatHealth().Get();
float maxEnergy = GetStatEnergy().GetMax();
```

PlayerStats class:
```c
PlayerStatBase GetStatObject( int id )   // by EPlayerStats_current enum
void ResetAllStats()
void SaveStats( ParamsWriteContext ctx )
bool LoadStats( ParamsReadContext ctx, int version )
```

## PlayerBase — Inventory

```c
// Every EntityAI has inventory:
GameInventory inv = entity.GetInventory();

// Create item in player inventory:
EntityAI item = player.CreateInInventory( "M4A1" );  // returns NULL if no space

// Manual location-based creation:
InventoryLocation loc = new InventoryLocation();
if ( inv.FindFirstFreeLocationForNewEntity( "Ammo_762x39", FindInventoryLocationType.ANY, loc ) )
{
    EntityAI ammo = GameInventory.LocationCreateEntity( loc, "Ammo_762x39", ECE_IN_INVENTORY, RF_DEFAULT );
}

// Copy item into inventory:
ItemBase copy = CreateCopyOfItemInInventory( srcItem );
// Transfers properties via MiscGameplayFunctions.TransferItemProperties()
```

## Entity Creation — ECE Flags

File: `scripts/3_game/ce/centraleconomy.c`

```c
// Individual flags:
const int ECE_SETUP           = 2;
const int ECE_TRACE           = 4;       // trace under entity (surface align)
const int ECE_UPDATEPATHGRAPH = 32;      // update navmesh
const int ECE_CREATEPHYSICS   = 1024;    // create collision physics
const int ECE_LOCAL           = 1073741824; // local only (no network)
const int ECE_NOSURFACEALIGN  = 262144;
const int ECE_KEEPHEIGHT      = 524288;

// Common combinations:
const int ECE_IN_INVENTORY    = 787456;  // CREATEPHYSICS | KEEPHEIGHT | NOSURFACEALIGN
const int ECE_PLACE_ON_SURFACE = 1060;  // CREATEPHYSICS | UPDATEPATHGRAPH | TRACE
const int ECE_OBJECT_SWAP     = 787488; // for cargo swaps

// Rotation flags (RF_):
const int RF_DEFAULT  = 512;   // config-based rotation
const int RF_ORIGINAL = 128;   // original config rotation
const int RF_FRONT    = 1;
const int RF_TOP      = 2;

// Usage:
EntityAI obj = g_Game.CreateObject( "Sedan_02", position, ECE_PLACE_ON_SURFACE, RF_DEFAULT );
```

## Weather API

File: `scripts/3_game/weather.c`

```c
Weather w = GetGame().GetWeather();

// Weather phenomena (all use same API):
w.GetOvercast().Set( 0.8, 120.0, 60.0 );   // forecast, transitionTime, minDuration
w.GetRain().Set( 0.5, 60.0, 30.0 );
w.GetSnowfall().Set( 0.3, 60.0 );
w.GetFog().Set( 0.4, 30.0 );

// Wind:
w.SetWindSpeed( 5.0 );           // m/s, minimum 0.1
w.SetWindMaximumSpeed( 15.0 );
w.GetWindDirection().Set( 1.5, 60.0 );  // radians [-PI, +PI]
w.SetWindFunctionParams( 0.1, 0.5, 0.5 );  // min, max, speed

// Storm:
w.SetStorm( 0.8, 0.6, 5.0 );    // density, threshold, lightningTimeout

// Thresholds:
w.SetRainThresholds( 0.6, 0.9, 30.0 );    // overcastMin, overcastMax, time
w.SetSnowfallThresholds( 0.6, 0.9, 30.0 );

// Dynamic volumetric fog:
w.SetDynVolFogDistanceDensity( 0.3, 30.0 );
w.SetDynVolFogHeightDensity( 0.1, 30.0 );
w.SetDynVolFogHeightBias( 0.5, 30.0 );

// Freeze/mission control:
w.SetWeatherUpdateFreeze( true );    // pause random weather changes
w.MissionWeather( true );            // enable mission-controlled weather

// Reading current values:
float currentOvercast = w.GetOvercast().GetActual();   // 0.0–1.0
float targetOvercast  = w.GetOvercast().GetForecast(); // target
```

## Key COT Usage Patterns for Vanilla APIs

When building COT modules that interact with DayZ:

```c
// Heal player fully:
player.GetBleedingManagerServer().RemoveAllSources();
CommunityOnlineToolsBase.HealEntityRecursive( player, true, true );
player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
player.GetStatWater().Set( player.GetStatWater().GetMax() );

// Teleport player:
player.SetLastPosition();           // save for teleport-back
player.SetWorldPosition( position );

// Spawn item in player inventory:
EntityAI item = player.CreateInInventory( "M4A1" );

// Spawn object in world:
EntityAI obj = g_Game.CreateObject( "ClassName", position, ECE_PLACE_ON_SURFACE, RF_DEFAULT );

// Set weather:
GetGame().GetWeather().GetOvercast().Set( 0.0, 120.0, 60.0 );  // clear sky

// Get all players (server-side):
array<Man> players = new array<Man>();
g_Game.GetWorld().GetPlayerList( players );
```
