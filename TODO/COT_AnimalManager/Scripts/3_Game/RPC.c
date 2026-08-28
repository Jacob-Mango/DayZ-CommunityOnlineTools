// Plage RPC dediee (au-dessus des enums COT existants pour eviter tout conflit).
enum JMAnimalsModuleRPC
{
	INVALID = 10600,
	RequestServerAnimals,
	SendServerAnimals,
	DeleteAnimal,
	DeleteAnimalAll,
	DeleteAnimalRadius,
	TeleportToAnimal,
	HealAnimal,
	COUNT
}
