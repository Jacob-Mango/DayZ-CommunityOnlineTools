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

	#ifdef SERVER
		PlayerIdentity identity = action_data.m_Player.GetIdentity();
		if (identity && GetCommunityOnlineToolsBase().IsActive(identity) && GetPermissionsManager().HasPermission("Actions.QuickActions", identity))
	#else
		if (GetCommunityOnlineToolsBase().IsActive() && GetPermissionsManager().HasPermission("Actions.QuickActions"))
	#endif
			m_QuantityUsedPerSecond = m_COT_OriginalQuantityUsedPerSecond * 100;
		else
			m_QuantityUsedPerSecond = m_COT_OriginalQuantityUsedPerSecond;
	}
}
