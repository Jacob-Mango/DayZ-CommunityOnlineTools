modded class Bottle_Base
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla OnDebugSpawn which sets quantity to max
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}
}
