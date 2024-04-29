modded class PortableGasStove
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla PortableGasStove::OnDebugSpawn which spawns waterbottle on ground
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}
}
