modded class ZombieBase
{
	//! Synced so the ESP menu can report the state it is about to toggle - a
	//! client cannot see an input controller override any other way.
	protected bool m_COT_Immobilized;

	void ZombieBase()
	{
		RegisterNetSyncVariableBool("m_COT_Immobilized");
	}

	//! Hold this infected still without killing it.
	//!
	//! Two halves, and both are needed: the agent kept in idle stops the AI
	//! deciding to go anywhere, and the input controller overrides stop the
	//! command already running from carrying it the rest of the way.
	void COT_SetImmobilized(bool state)
	{
		m_COT_Immobilized = state;

		COT_ApplyImmobilized();

		SetSynchDirty();
	}

	bool COT_IsImmobilized()
	{
		return m_COT_Immobilized;
	}

	protected void COT_ApplyImmobilized()
	{
		if (GetAIAgent())
			GetAIAgent().SetKeepInIdle(m_COT_Immobilized);

		DayZInfectedInputController controller = GetInputController();

		if (!controller)
			return;

		controller.OverrideMovementSpeed(m_COT_Immobilized, 0.0);
		controller.OverrideTurnSpeed(m_COT_Immobilized, 0.0);
		controller.OverrideHeading(m_COT_Immobilized, GetOrientation()[0] * Math.DEG2RAD);
		controller.OverrideAlertLevel(m_COT_Immobilized, false, 0, 0.0);
	}

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

