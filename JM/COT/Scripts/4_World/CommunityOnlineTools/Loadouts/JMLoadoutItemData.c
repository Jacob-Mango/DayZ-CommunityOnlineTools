class JMLoadoutItemData
{
	float m_Health = -1;
	float m_Quantity = -1;
	int m_LiquidType = -1;
	int m_Temperature;

	void JMLoadoutItemData(EntityAI entity)
	{
		if (!entity.IsPlainObject() && MiscGameplayFunctions.GetTypeMaxGlobalHealth(entity.GetType()) > 0)
		{
			if ( g_Game.IsServer() )
				m_Health = entity.GetHealth();
			else
				m_Health = entity.GetHealthLevelValue(entity.GetHealthLevel());
		}

		if (entity.HasQuantity())
		{
			m_Quantity = entity.GetQuantity();
			m_LiquidType = entity.GetLiquidType();
		}
		else
		{
			Magazine mag;
			Weapon_Base weapon;
			if (Class.CastTo(mag, entity))
				m_Quantity = mag.GetAmmoCount();
			else if (Class.CastTo(weapon, entity) && weapon.HasInternalMagazine(-1))
				m_Quantity = weapon.GetInternalMagazineCartridgeCount(0);
		}

		if (entity.CanHaveTemperature())
			m_Temperature = entity.GetTemperature();
	}
}
