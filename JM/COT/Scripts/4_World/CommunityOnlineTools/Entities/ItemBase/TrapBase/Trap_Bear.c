modded class BearTrap
{
	//override void COT_OnDebugSpawn(PlayerBase player)
	//{
		//SetActive();
	//}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(COT_ActionTriggerBearTrap);
	}
}
