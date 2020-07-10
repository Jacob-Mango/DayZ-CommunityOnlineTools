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

enum JMCOTRPC
{
	INVALID = 10140,
	Active
	COUNT
};