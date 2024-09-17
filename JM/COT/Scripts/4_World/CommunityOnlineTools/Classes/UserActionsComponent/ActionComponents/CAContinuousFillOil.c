modded class CAContinuousFillOil
{
	float m_COT_OriginalQuantityUsedPerSecond;

	void CAContinuousFillOil(float quantity_used_per_second, float time_to_progress)
	{
		m_COT_OriginalQuantityUsedPerSecond = quantity_used_per_second;
	}

	override void Setup(ActionData action_data)
	{
		super.Setup(action_data);

		if (GetPermissionsManager().HasQuickActionAccess(action_data.m_Player))
			m_QuantityUsedPerSecond = m_COT_OriginalQuantityUsedPerSecond * 100;
		else
			m_QuantityUsedPerSecond = m_COT_OriginalQuantityUsedPerSecond;
	}
}
