modded class CAContinuousRepeat
{
	override void Setup(ActionData action_data)
	{
		super.Setup(action_data);

		if (GetPermissionsManager().HasQuickActionAccess(action_data.m_Player))
			m_TimeToComplete = 0.1;
	}
}
