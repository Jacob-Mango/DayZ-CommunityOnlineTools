modded class CAContinuousFillPowerGenerator
{	
	override void Setup(ActionData action_data)
	{
		super.Setup(action_data);

		if (GetPermissionsManager().HasQuickActionAccess(action_data.m_Player))
			m_AdjustedQuantityFilledPerSecond *= 100;
	}
}
