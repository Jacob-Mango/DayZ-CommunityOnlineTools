modded class CAContinuousCraft
{		
	override void Setup(ActionData action_data)
	{
		super.Setup(action_data);

	#ifdef SERVER
		PlayerIdentity identity = action_data.m_Player.GetIdentity();
		if (identity && GetCommunityOnlineToolsBase().IsActive(identity) && GetPermissionsManager().HasPermission("Actions.QuickActions", identity))
	#else
		if (GetCommunityOnlineToolsBase().IsActive() && GetPermissionsManager().HasPermission("Actions.QuickActions"))
	#endif
			m_AdjustedTimeToComplete = 0.1;
	}
}
