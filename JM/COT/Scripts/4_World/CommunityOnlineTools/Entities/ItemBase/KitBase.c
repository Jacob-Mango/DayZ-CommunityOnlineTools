modded class KitBase
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla OnDebugSpawn which spawns materials + tools on ground
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}
}
