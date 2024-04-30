modded class TrapBase
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla OnDebugSpawn which tries to activate the trap
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(COT_QuickActionActivateTrap);
	}
}
