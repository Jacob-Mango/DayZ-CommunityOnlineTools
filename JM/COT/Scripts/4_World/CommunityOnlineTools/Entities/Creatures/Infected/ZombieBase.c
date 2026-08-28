modded class ZombieBase
{
	//! Infected are not ItemBase, so without this the tracker cannot see them
	//! at all. EEInit/EEDelete rather than ctor/dtor -- see ItemBase.c.
	override void EEInit()
	{
		super.EEInit();

		if (g_Game.IsServer())
			JMEntityTracker.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		if (g_Game && g_Game.IsServer())
			JMEntityTracker.Unregister(this);

		super.EEDelete(parent);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer.COTGetReceiveDamageDealt())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, "Torso", ammo, "0 0 0", speedCoef);
			return false;
		}

		return true;
	}
}

