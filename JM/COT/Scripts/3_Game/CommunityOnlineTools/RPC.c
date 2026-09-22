enum JMClientRPC
{
	INVALID = 10100,
	RefreshClients,
	RefreshClientPositions,
	RemoveClient,
	UpdateClient,
	UpdateClientPosition,
	SetClient,
	//! Appended: one array send for the whole roster instead of the old
	//! per-player unicast loop in RPC_RefreshClients / RPC_RefreshClientPositions.
	UpdateClientBatch,
	UpdateClientPositionBatch,
	COUNT
}

enum JMRoleRPC
{
	INVALID = 10120,
	RefreshRoles,
	UpdateRole,
	RemoveRole,
	COUNT
}

enum JMPermissionRPC
{
	INVALID = 10130,
	COUNT
}

enum JMCOTBaseRPC
{
	INVALID = 10135,
	TransportSync,
	COUNT
}

enum JMCOTRPC
{
	INVALID = 10140,
	Active,
	COUNT
}

enum JMCameraModuleRPC
{
	INVALID = 10160,
	Enter,
	Leave,
	Leave_Finish,
	UpdatePosition,
	COUNT
}

enum JMVehiclesModuleRPC
{
	//! Was 10180: its 20 ids ran into JMItemSetSpawnerModuleRPC (10200). Ranges are
	//! checked by Workbench/Batchfiles/CheckRPCRanges.ps1.
	INVALID = 10650,
	RequestServerVehicles,
	SendServerVehicles,
	DeleteVehicle,
	DeleteVehicleAll,
	DeleteVehicleUnclaimed,
	DeleteVehicleDestroyed,
	TeleportToVehicle,
	TeleportVehicleToMe,
	RepairVehicle,
	RefuelVehicle,
	UnstuckVehicle,
	CoverVehicle,
	LockVehicle,
	UnPairVehicle,
	// Appended after 1.x - delta-refresh slots. Old clients never see these.
	SendVehicleUpsert,
	SendVehicleRemove,
	RequestVehicleUpsert,
	//! Undo: the only vehicle teleport that names its own destination.
	TeleportVehicleTo,
	ClearVehicleCargo,
	SpawnVehicleKey,
	COUNT
}

enum JMItemSetSpawnerModuleRPC
{
	INVALID = 10200,
	Load,
	SpawnPosition,
	SpawnPlayers,
	COUNT
}

enum JMObjectSpawnerModuleRPC
{
	INVALID = 10220,
	Position,
	Inventory,
	Delete,
	COUNT
}

enum JMTeleportModuleRPC
{
	INVALID = 10240,
	Load,
	Position,
	PositionRaycast,
	Location,
	AddLocation,
	RemoveLocation,
	EditLocation,
	COUNT
}

enum JMVehicleSpawnerModuleRPC
{
	INVALID = 10260,
	Load,
	SpawnPosition,
	COUNT
}

enum JMWeatherModuleRPC
{
	//! Was 10280: SpecialWeatherStatus landed on 10301 == JMESPModuleRPC.Log.
	INVALID = 10600,
	Load,
	Storm,
	Fog,
	DynamicFog,
	Rain,
	RainThresholds,
	Snow,
	SnowThresholds,
	Overcast,
	WindFunctionParams,
	WindMagnitude,
	WindDirection,
	Date,
	UsePreset,
	CreatePreset,
	UpdatePreset,
	RemovePreset,
	FreezeTime,
	DynamicWeather,
	Sandstorm,
	SpecialWeatherStatus,
	DynamicControl,
	DynamicStatus,
	COUNT
}

enum JMESPModuleRPC
{
	INVALID = 10300,
	Log,
	SetPosition,
	SetOrientation,
	SetHealth,
	DeleteObject,

	BaseBuilding_Build,
	BaseBuilding_Dismantle,
	BaseBuilding_Repair,

	Vehicle_Unstuck,
	Vehicle_Refuel,

	Heal,

	MakeItemSet,
	DuplicateAll,
	DeleteAll,
	MoveToCursor,

	ObjectAction,
	ObjectActionResult,

	//! Appended, never inserted - these are wire values. It belongs with the
	//! other BaseBuilding_ entries by meaning, but slotting it there would
	//! renumber every id below it and silently mismatch an older client.
	BaseBuilding_SetPartHealth,

	//! Sent once per move/rotate gesture (not per throttled position/
	//! orientation update) so the server can push one JMActionHistory entry
	//! for the whole drag. See JMESPWidgetHandler.EndDrag.
	RecordTransformHistory,

	//! Weapon attachment add/remove/swap by classname - not routed through
	//! ObjectAction because that channel's wire format is (int, float) only
	//! and has no string slot for a classname.
	SetAttachment,
	RemoveAttachment,

	COUNT
}

//! Everything the ESP object panel can do to a target that is not worth its
//! own RPC. One channel with an action id keeps RPC.c from growing a line per
//! button, and every one of them still carries its own permission check on the
//! server - see JMESPModule.PermissionForAction.
enum JMESPObjectAction
{
	INVALID = 0,

	SetQuantity,
	SetLiquid,
	SetCleanness,
	SetFoodStage,
	SetOpen,

	SetLock,
	SetCode,
	GetCode,

	//! Appended, never inserted: these are wire values, and renumbering an
	//! existing entry silently mismatches a client on an older build.

	//! Weapons. ivalue is the on/off for the two toggles; LoadMagazine takes
	//! none and works the type out from the weapon's own config.
	SetJammed,
	SetChambered,
	LoadMagazine,

	//! Fireplaces. SetStoneCircle and SetOven are the two build-quality
	//! upgrades a fireplace can carry; CookAll finishes everything on it.
	SetBurning,
	FireplaceRefuel,
	SetStoneCircle,
	SetOven,
	CookAll,

	//! Every combination lock within reach of the target, not just its own -
	//! a base is a dozen separate entities and locking them one at a time is
	//! a dozen right-clicks.
	SetLockAll,

	SetLockWheels,
	RefillCoolant,
	RepairAndFillSlots,
	SetGrenadePin,
	TriggerTrap,
	SetFenceOpen,
	SetTentOpen,
	SetCarDoors,

	//! Vehicle fluids as a fraction of their own capacity - fvalue is 0..1.
	//! RefillCoolant above is the same thing pinned at 1, kept because it is a
	//! wire value an older client may still send.
	SetFuel,
	SetCoolant,

	//! A weapon that takes no detachable magazine loads into the gun itself.
	FillInternalMagazine,

	//! Every construction part of one object at once, in dependency order.
	BuildAll,
	DismantleAll,

	//! Every already-built construction part of one object, healed to full.
	RepairAll,

	//! A flag pole's mast, fvalue 0..1 raised, refresher timer moved with it.
	SetFlagRaised,

	//! Delete everything sitting in an object's cargo, the object kept.
	ClearCargo,

	//! Pin a creature's AI: kept in idle, with its input controller held still.
	SetImmobilized,

	//! Arm or disarm a trap without anything stepping on it.
	SetTrapArmed,

	//! Swap the target's classname, and any already-attached child's, for the
	//! sibling variant carrying a different color token - ivalue indexes into
	//! JMObjectSpawnerModule.GetColorTokens().
	ChangeColor,

	//! A static map building's door - ivalue is open/close or lock/unlock (per
	//! action), fvalue carries the Building door index picked by the raycast,
	//! since there is no per-door target to route through separately.
	SetHouseDoorOpen,
	SetHouseDoorLocked,

	COUNT
}

enum JMStatType
{
	Health = 0,
	Blood,
	Shock,
	Energy,
	Water,
	Stamina,
	HeatBuffer,
	COUNT
}

enum JMPlayerModuleRPC
{
	//! Was 10320 - JMESPModuleRPC grew past that boundary once SetAttachment/
	//! RemoveAttachment were appended (23 entries from its own INVALID=10300),
	//! so its wire values collided with this block's (SetAttachment/
	//! RemoveAttachment landed on SetBloodyHands/RepairTransport). Bumped with
	//! headroom for both blocks; still well under JMNamalskEventManagerRPC's 10400.
	INVALID = 10340,

	SetStat,

	SetBloodyHands,
	RepairTransport,
	TeleportTo,
	TeleportSenderTo,
	TeleportToPrevious,
	StartSpectating,
	EndSpectating,
	EndSpectating_Finish,
	SetCannotBeTargetedByAI,
	SetGodMode,
	SetFreeze,
	SetInvisible,
	SetReceiveDamageDealt,
	SetRemoveCollision,
	SetUnlimitedAmmo,
	SetUnlimitedStamina,
	SetAdminNVG,
	SetBrokenLegs,
	Heal,
	Strip,
	ClearCargo,
	Dry,
	Kick,
	KickMessage,
	Ban,
	BanMessage,
	Message,
	Notif,
	StopBleeding,
	Vomit,
	SetScale,
	SetPermissions,
	SetRoles,
	VONStartedTransmitting,
	VONStoppedTransmitting,
	AddDisease,
	RemoveDisease,
	RemoveAllDiseases,
	SendDiseaseMask,
	AddBleedingPart,
	StopBleedingPart,
	SendBleedingState,
	//! Appended, never inserted: these are wire values, and renumbering an
	//! existing entry silently mismatches a client on an older build.
	RequestInventory,
	InventoryDelete,
	InventoryRepair,
	InventoryTake,
	RequestPlayerStats,
	InventoryModify,
	InventoryGroupOp,
	RequestExpansionInfo,
	SetRagdoll,
	ActivateModifier,
	DeactivateModifier,
	COUNT
}

//! Which edit an InventoryModify request is asking for. One RPC covers all of
//! them because they share every argument but the value: the item is addressed
//! the same way, checked against the same permission and answered with the same
//! refreshed listing, so five ids would have been five copies of one handler.
//!
//! Wire values - append only.
enum JMInventoryModifyOp
{
	UNJAM = 0,
	QUANTITY,      //!< ammo count for magazines, quantity for everything else
	TEMPERATURE,
	FOOD_STAGE,    //!< FoodStageType
	LIQUID_TYPE,   //!< LIQUID_*
	HEALTH,        //!< 0..100
	COUNT
}

//! What an InventoryGroupOp request is asking for. The item addressed by the
//! request is the CONTAINER; the operation is applied to everything it holds
//! directly - its cargo and its attachments - and never to the container itself.
//!
//! Wire values - append only.
enum JMInventoryGroupOp
{
	TAKE_ALL = 0,
	REPAIR_ALL,
	DELETE_ALL,
	COUNT
}

enum JMNamalskEventManagerRPC
{
	INVALID = 10400,
	LoadEvents,
	RequestEvents,
	StartEvent,
	CancelEvent,
	COUNT
}

enum JMCommandModuleRPC
{
	INVALID = 10420,
	PerformCommand,
	COUNT
}

enum JMWebhookCOTModuleRPC
{
	INVALID = 10440,
	Load,
	AddConnectionGroup,
	RemoveConnectionGroup,
	EditConnectionGroup,
	AddType,
	RemoveType,
	TypeState,
	COUNT
}

enum JMLoadoutModuleRPC
{
	INVALID = 10460,
	Load,
	Create,
	Delete,
	SpawnCursor,
	SpawnTarget,
	SpawnPlayers,
	COUNT
}

enum JMCompensationsModuleRPC
{
	INVALID = 10480,
	Load,
	SpawnCursor,
	SpawnTarget,
	SpawnPlayers,
	Delete,
	COUNT
}

enum JMRoleManagerModuleRPC
{
	INVALID = 10500,

	// Client -> Server
	RequestRoleList,
	CreateRole,
	DeleteRole,
	SetRolePermissions,

	// Server -> Client
	RoleList,

	COUNT
}

enum JMLootAnalysisModuleRPC
{
	INVALID = 10520,

	// Client -> Server
	RequestItemScan,
	RequestLootDistribution,
	DeleteAllItems,

	// Server -> Client
	SendItemScanResults,
	SendLootDistribution,

	COUNT
}

// Generic JMEntityManagerModule RPCs. Every concrete manager (events, garage,
// territory) gets its own range below that reuses the same three slots:
//   Request  - client -> server: "send me entities"
//   Send     - server -> client: "here are the entities"
//   Action   - client -> server: "perform action id X on entity Y"
//
// SendUpsert / SendRemove were appended afterwards for the delta-refresh path
// (server -> client). They MUST stay at the end of each enum so pre-delta
// clients still match on the first four slots.

enum JMEventsModuleRPC
{
	INVALID = 10540,
	Request,
	Send,
	Action,
	SendUpsert,
	SendRemove,
	COUNT
}

enum JMTerritoryModuleRPC
{
	INVALID = 10560,
	RequestTerritories,
	SendTerritories,
	SetLevel,
	COUNT
}


enum JMBanModuleRPC
{
    INVALID = 10800,

    // Client -> Server
    RequestBanList,
    RequestBan,
    UnbanPlayer,
    EditBanDuration,

    // Server -> Client
    BanList,

    COUNT
}


// Map editor - 3D place / scale / move / delete of world objects.
// Stored objects survive mission restart (per-mission JSON on the server).
// RPCs are deliberately coarse: one RPC per mutation, GUID-list batched on read.
enum JMMapEditorModuleRPC
{
	INVALID = 10900,

	// Client -> Server
	RequestList,
	SpawnObject,
	TransformObject,
	DeleteObject,
	ClearAll,
	BulkTransform,
	BulkDelete,
	CloneObject,
	Undo,
	Redo,

	// Server -> Client
	List,
	//! RESERVED, not yet implemented: intended to push undo/redo stack depth to
	//! the client so the form can grey out unavailable buttons. The stacks are
	//! server-side (m_UndoStack), so the client currently cannot know. Never
	//! sent and deliberately has no OnRPC case -- rpc_check flags this, see
	//! ai/tools/README.md.
	UndoState,

	COUNT
}

// Anti-cheat detection module - server-side only, periodic poll.
// Server fires webhooks; admin UI reads flag state via dedicated RPCs.
enum JMAntiCheatModuleRPC
{
	INVALID = 10920,

	// Client -> Server
	RequestFlags,
	ClearFlag,

	// Server -> Client
	Flags,

	COUNT
}


// Shared undo/redo stack trigger. Both are parameterless client -> server
// requests; the server replies through the notification system, not an RPC.
// (Were JMESPModuleRPC.UndoLastAction / RedoLastAction until the stack got a
// module of its own - see JMActionHistoryModule.)
enum JMActionHistoryModuleRPC
{
	INVALID = 10960,
	Undo,
	Redo,
	COUNT
}

// Server performance broadcast - sampled on the server, pushed to admin
// clients on a timer. No client -> server direction: nothing is requested.
enum JMServerStatsModuleRPC
{
	INVALID = 10940,

	// Server -> Client
	Stats,

	COUNT
}
