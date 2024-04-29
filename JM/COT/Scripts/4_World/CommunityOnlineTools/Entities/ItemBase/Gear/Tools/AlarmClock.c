modded class AlarmClock_ColorBase
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla OnDebugSpawn which sets off the alarm
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}
}
