modded class BearTrap
{
	//override void COT_OnDebugSpawn()
	//{
		//SetActive();
	//}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(COT_ActionTriggerBearTrap);
	}
}
