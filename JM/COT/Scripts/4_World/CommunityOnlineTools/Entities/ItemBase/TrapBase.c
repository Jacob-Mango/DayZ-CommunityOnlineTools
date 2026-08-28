modded class TrapBase
{
	//! Prevent ItemBase::COT_OnDebugSpawn calling vanilla OnDebugSpawn which tries to activate the trap
	override void COT_OnDebugSpawn(PlayerBase player)
	{
	}

	//! Arming is the trap's own attribution point: a trap armed where it lies
	//! never passes through OnInventoryExit or OnPlacementComplete, so without
	//! this its kills resolve to nobody.
	override void StartActivate(PlayerBase player)
	{
		super.StartActivate(player);

		if (g_Game.IsServer() && player)
			COT_SetKillOwner(player);
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(COT_QuickActionActivateTrap);
	}
}
