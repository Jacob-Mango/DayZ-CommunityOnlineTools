// Plage RPC dediee (distincte de COT_AnimalManager qui occupe 10600+) pour eviter
// tout conflit si les deux mods sont charges en meme temps.
enum JMBuildingsModuleRPC
{
	INVALID = 10700,
	RequestServerBuildings,
	SendServerBuildings,
	DeleteBuilding,
	DeleteBuildingGroup,
	DeleteBuildingAll,
	COUNT
}
