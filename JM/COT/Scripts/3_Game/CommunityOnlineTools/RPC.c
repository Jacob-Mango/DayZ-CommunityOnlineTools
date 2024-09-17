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
};

enum JMRoleRPC
{
	INVALID = 10120,
	RefreshRoles,
	UpdateRole,
	RemoveRole,
	COUNT
};

enum JMPermissionRPC
{
	INVALID = 10130,
	COUNT
};

enum JMCOTBaseRPC
{
	INVALID = 10135,
	TransportSync,
	COUNT
};

enum JMCOTRPC
{
	INVALID = 10140,
	Active,
	COUNT
};

enum JMCameraModuleRPC
{
	INVALID = 10160,
	Enter,
	Leave,
	Leave_Finish,
	UpdatePosition,
	COUNT
};

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
	COUNT
};

enum JMItemSetSpawnerModuleRPC
{
	INVALID = 10200,
	Load,
	SpawnPosition,
	SpawnPlayers,
	COUNT
};

enum JMObjectSpawnerModuleRPC
{
	INVALID = 10220,
	Position,
	Inventory,
	Delete,
	COUNT
};

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
};

enum JMVehicleSpawnerModuleRPC
{
	INVALID = 10260,
	Load,
	SpawnPosition,
	COUNT
};

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
	Wind, // LEGACY
	WindFunctionParams,
	WindMagnitude,
	WindDirection,
	Date,
	UsePreset,
	CreatePreset,
	UpdatePreset,
	RemovePreset,
	COUNT
};

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
};

enum JMPlayerModuleRPC
{
	INVALID = 10320,

	// Player Vitals
	SetHealth,
	SetBlood,
	SetShock,
	SetEnergy,
	SetWater,
	SetStamina,
	SetHeatBuffer,

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
	COUNT
};

enum JMNamalskEventManagerRPC
{
	INVALID = 10400,
	LoadEvents,
	RequestEvents,
	StartEvent,
	CancelEvent,
	COUNT
};

enum JMCommandModuleRPC
{
	INVALID = 10420,
	PerformCommand,
	COUNT
};

enum JMWebhookCOTModuleRPC
{
	INVALID = 10440,
	Load,
	AddConnectionGroup,
	RemoveConnectionGroup,
	AddType,
	RemoveType,
	TypeState,
	COUNT
};

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
};

