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

enum JMRoleRPC : JMClientRPC
{
	INVALID,
	RefreshRoles,
	UpdateRole,
	RemoveRole,
	COUNT
};

enum JMPermissionRPC : JMRoleRPC
{
	INVALID,
	COUNT
};

enum JMCOTRPC : JMPermissionRPC
{
	INVALID,
	Active,
	COUNT
};

enum JMCameraModuleRPC : JMCOTRPC
{
	INVALID,
	Enter,
	Leave,
	Leave_Finish,
	UpdatePosition,
	COUNT
};

enum JMVehiclesModuleRPC : JMCameraModuleRPC
{
	INVALID,
	RequestServerVehicles,
	SendServerVehicles,
	DeleteVehicle,
	DeleteVehicleAll,
	DeleteVehicleUnclaimed,
	DeleteVehicleDestroyed,
	TeleportToVehicle,
	COUNT
};

enum JMItemSetSpawnerModuleRPC : JMVehiclesModuleRPC
{
	INVALID,
	Load,
	SpawnPosition,
	SpawnPlayers,
	COUNT
};

enum JMObjectSpawnerModuleRPC : JMItemSetSpawnerModuleRPC
{
	INVALID,
	Position,
	Inventory,
	Delete,
	COUNT
};

enum JMTeleportModuleRPC : JMObjectSpawnerModuleRPC
{
	INVALID,
	Load,
	Position,
	PositionRaycast,
	Location,
	COUNT
};

enum JMVehicleSpawnerModuleRPC : JMTeleportModuleRPC
{
	INVALID,
	Load,
	SpawnPosition,
	COUNT
};

enum JMWeatherModuleRPC : JMVehicleSpawnerModuleRPC
{
	INVALID,
	Load,
	Storm,
	Fog,
	Rain,
	RainThresholds,
	Overcast,
	Wind,
	WindFunctionParams,
	Date,
	UsePreset,
	CreatePreset,
	UpdatePreset,
	RemovePreset,
	COUNT
};

enum JMESPModuleRPC : JMWeatherModuleRPC
{
	INVALID,
	Log,
	SetPosition,
	SetOrientation,
	SetHealth,
	DeleteObject,

	BaseBuilding_Build,
	BaseBuilding_Dismantle,
	BaseBuilding_Repair,

	Car_Unstuck,
	Car_Refuel,
	Car_Repair,

	MakeItemSet,
	DuplicateAll,
	DeleteAll,
	MoveToCursorRelative,
	MoveToCursorAbsolute,
	COUNT
};

enum JMPlayerModuleRPC : JMESPModuleRPC
{
	INVALID,
	SetHealth,
	SetBlood,
	SetShock,
	SetEnergy,
	SetWater,
	SetStamina,
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
	SetBrokenLegs,
	Heal,
	Strip,
	Dry,
	Kick,
	KickMessage,
	StopBleeding,
	SetPermissions,
	SetRoles,
	VONStartedTransmitting,
	VONStoppedTransmitting,
	COUNT
};

enum JMNamalskEventManagerRPC : JMPlayerModuleRPC
{
	INVALID,
	LoadEvents,
	RequestEvents,
	StartEvent,
	CancelEvent,
	COUNT
};

enum JMCommandModuleRPC : JMNamalskEventManagerRPC
{
	INVALID,
	PerformCommand,
	COUNT
};

enum JMWebhookCOTModuleRPC : JMCommandModuleRPC
{
	INVALID,
	Load,
	AddConnectionGroup,
	RemoveConnectionGroup,
	AddType,
	RemoveType,
	TypeState,
	COUNT
};
