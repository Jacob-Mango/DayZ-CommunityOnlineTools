modded class Crossbow_Base
{
	override void OnDebugSpawnEx(DebugSpawnParams params)
	{
		if (GetHierarchyParent() && !GetHierarchyRootPlayer())
		{
			//! Vanilla DebugSpawn spawns a bunch of stuff on ground or in player inventory, we don't want that when attached
			SpawnAmmo("Ammo_HuntingBolt", SAMF_DEFAULT);
			GetInventory().CreateInInventory("ACOGOptic_6x");
		}
		else
		{
			super.OnDebugSpawnEx(params);
		}
	}
}
