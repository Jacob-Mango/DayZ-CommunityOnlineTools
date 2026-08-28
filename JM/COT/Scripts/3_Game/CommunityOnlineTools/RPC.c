enum JMClientRPC
{
	INVALID = 10100,
	RefreshClients,
	RefreshClientPositions,
	RemoveClient,
	UpdateClient,
	UpdateClientPosition,
	SetClient,
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
	INVALID = 10180,
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
	INVALID = 10280,
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
	INVALID = 10320,

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

	// Server -> Client
	Flags,

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
