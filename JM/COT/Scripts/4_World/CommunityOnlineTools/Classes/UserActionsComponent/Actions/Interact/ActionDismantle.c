class COT_QuickActionDismantle: ActionBuildPart // PLACEHOLDER
{
	void COT_QuickActionDismantle()
	{
		m_CommandUID	= DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask	= DayZPlayerConstants.STANCEMASK_ALL;
		m_FullBody		= false;
	}
	
	override void CreateConditionComponents()  
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}
	
	override bool UseMainItem()
	{
		return false;
	}
	
	override bool HasProgress()
	{
		return false;
	}
	
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
        m_Text = "[COT QuickAction]" + m_Text;
	}
	
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (player.IsPlacingLocal() || player.IsPlacingServer())
			return false;
		
		if (player.GetItemInHands())
			return false;
        
		BaseBuildingBase base_building = BaseBuildingBase.Cast(target.GetObject());
        if (!base_building)
			return false;

		if (!GetPermissionsManager().HasQuickActionAccess(player))
			return false;
	
		Construction construction = base_building.GetConstruction();
		if (!construction)
			return false;
		
		ConstructionActionData construction_action_data = player.GetConstructionActionData();
		construction_action_data.SetTarget( target.GetObject() );

		// Currently constrution_part is NULL, fixing it is probably the last Step
		ConstructionPart constrution_part = construction_action_data.GetBuildPartAtIndex(m_VariantID);
		if ( !constrution_part )
			return false;

		string partName = construction_action_data.GetBuildPartAtIndex(m_VariantID).GetPartName();

		if (!construction.COT_CanDismantlePart(partName))
			return false;

		return true;
	}

	override bool ActionConditionContinue(ActionData action_data)
	{
		return true;
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		BaseBuildingBase base_building = BaseBuildingBase.Cast(action_data.m_Target.GetObject());
		Construction construction = base_building.GetConstruction();
		
		ConstructionActionData construction_action_data = action_data.m_Player.GetConstructionActionData();
		ConstructionPart constrution_part = construction_action_data.GetBuildPartAtIndex(m_VariantID);

		construction.COT_DismantleRequiredParts( constrution_part.GetPartName(), action_data.m_Player );	
	}
};
