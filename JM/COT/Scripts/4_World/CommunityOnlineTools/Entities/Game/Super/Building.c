modded class BuildingBase
{
	//! Prevent calling vanilla EntityAI::OnDebugSpawn
	override void OnDebugSpawn()
	{
	}

	void COT_OnDebugSpawn()
	{
		OnDebugSpawn();

		COT_Refuel();
	}

	void COT_Refuel()
	{
		CommunityOnlineToolsBase.Refuel(this);
	}
}
