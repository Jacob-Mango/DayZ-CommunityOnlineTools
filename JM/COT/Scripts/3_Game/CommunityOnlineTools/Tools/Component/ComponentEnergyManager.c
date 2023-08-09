//! This would be preferable to modding ItemBase, but ComponentEnergyManager is not moddable :-(
/*
modded class ComponentEnergyManager
{
	override float AddEnergy(float added_energy)
	{
		if (added_energy < 0)
		{
			PlayerBase player = PlayerBase.Cast(m_ThisEntityAI.GetHierarchyRootPlayer());
			if (player && player.COTHasGodMode())
				return 0;  //! UNLIMITED POWER
		}

		return super.AddEnergy(added_energy);
	}
}
*/
