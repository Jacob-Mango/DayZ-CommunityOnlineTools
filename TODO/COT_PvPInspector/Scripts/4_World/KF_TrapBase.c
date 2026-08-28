// Affinage : armement d'un piege (bear trap, mine, tripwire) via l'action COT/vanilla.
// StartActivate recoit le joueur qui arme -> attribution la plus precise.
modded class TrapBase
{
	override void StartActivate(PlayerBase player)
	{
		super.StartActivate(player);

		if (GetGame() && GetGame().IsServer() && player)
			KF_SetOwner(player);
	}
}
