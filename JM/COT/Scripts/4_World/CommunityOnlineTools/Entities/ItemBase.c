modded class ItemBase
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer.COTGetReceiveDamageDealt() && !GetHierarchyRootPlayer())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, "Torso", ammo, "0 0 0", speedCoef);
			return false;
		}

		return true;
	}

	override void OnEnergyConsumed()
	{
		Man player = GetHierarchyRootPlayer();
		if (player && !player.GetAllowDamage())  //! UNLIMITED POWER
		{
			GetCompEM().SetEnergy0To1(1.0);
			return;
		}

		super.OnEnergyConsumed();
	}
};

