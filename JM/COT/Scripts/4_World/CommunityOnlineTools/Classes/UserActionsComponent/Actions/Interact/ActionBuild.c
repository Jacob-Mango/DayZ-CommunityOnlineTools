class COT_QuickActionBuild: ActionInteractBase
{
	void COT_QuickActionBuild()
	{
		m_CommandUID	= DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask	= DayZPlayerConstants.STANCEMASK_ALL;
		m_FullBody		= false;
		m_Text = "[COT QuickAction] #build";
	}
	
	override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
	{
		BaseBuildingBase base_building = BaseBuildingBase.Cast(target.GetObject());
		Construction construction = base_building.GetConstruction();
		string partname = construction.COT_GetFirstBuildablePart();

		if (partname != string.Empty)
		{
			m_Text = "[COT QuickAction] #build " + construction.GetConstructionPart( partname ).GetName();
		}
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

	#ifdef SERVER
		PlayerIdentity identity = player.GetIdentity();
		if (!GetCommunityOnlineToolsBase().IsActive(identity) || !GetPermissionsManager().HasPermission("Actions.QuickActions", identity))
			return false;
	#else
		if (!GetCommunityOnlineToolsBase().IsActive() || !GetPermissionsManager().HasPermission("Actions.QuickActions"))
			return false;
	#endif
	
		Construction construction = base_building.GetConstruction();
		
		string partname = construction.COT_GetFirstBuildablePart();
		if (partname == string.Empty)
			return false;

		return true;
	}
	
	override void OnStartServer( ActionData action_data )
	{
		super.OnStartServer(action_data);

		BaseBuildingBase base_building = BaseBuildingBase.Cast(action_data.m_Target.GetObject());
		Construction construction = base_building.GetConstruction();
		string partname = construction.COT_GetFirstBuildablePart();

		construction.COT_BuildRequiredParts( partname, action_data.m_Player, false );		
	}
};
