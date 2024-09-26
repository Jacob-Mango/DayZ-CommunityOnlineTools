modded class ActionActivateTrap
{
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (!super.ActionCondition(player, target, item))
			return false;

		if (Type() == ActionActivateTrap && GetPermissionsManager().HasQuickActionAccess(player))
			return false;

		return true;
	}
}

class COT_QuickActionActivateTrap: ActionActivateTrap
{
	void COT_QuickActionActivateTrap()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_CRAFTING;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_FullBody = false;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (!super.ActionCondition(player, target, item))
			return false;

		if (!GetPermissionsManager().HasQuickActionAccess(player))
			return false;

		return true;
	}
}

class COT_ActionTriggerBearTrap: ActionClapBearTrapWithThisItem
{
	void COT_ActionTriggerBearTrap()
	{
		m_CommandUID	= DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask	= DayZPlayerConstants.STANCEMASK_ALL;
		m_FullBody		= false;
		m_Text			= "[COT QuickAction] #trigger_bear_trap";
	}
	
	override void CreateConditionComponents()
	{
		m_ConditionItem		= new CCINone;
		m_ConditionTarget	= new CCTCursor;
	}

	override bool UseMainItem()
	{
		return false;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (!super.ActionCondition(player, target, item))
			return false;

		if (!GetPermissionsManager().HasQuickActionAccess(player))
			return false;

		return true;
	}
}
