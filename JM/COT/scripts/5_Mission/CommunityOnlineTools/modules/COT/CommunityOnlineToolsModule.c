class CommunityOnlineToolsModule: JMRenderableModuleBase
{
	static ref ScriptInvoker DATA_UPDATED = new ScriptInvoker();

	protected ref CommunityOnlineToolsData m_Data;

	void CommunityOnlineToolsModule()
	{
		GetRPCManager().AddRPC( "COT_Module", "LoadData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "COT.View" );
		GetPermissionsManager().RegisterPermission( "COT.UpdateData" );

		m_Data = new CommunityOnlineToolsData;
	}

	void ~CommunityOnlineToolsModule()
	{
	}

	CommunityOnlineToolsData GetData()
	{
		return m_Data;
	}

	override void RegisterKeyMouseBindings() 
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "COT.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/cot_form.layout";
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
	}

	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "COT.UpdateData", sender ) )
				return;
				
			Param1< ref CommunityOnlineToolsData > data;
			if ( !ctx.Read( data ) ) return;

			m_Data = data.param1;

			if ( m_Data.TestSort )
			{
				SendAdminNotification( sender, NULL, "You have enabled Player Sorting." );
			} else
			{
				SendAdminNotification( sender, NULL, "You have disabled Player Sorting." );
			}
		
			GetRPCManager().SendRPC( "COT_ServerInformation", "LoadData", new Param1< ref CommunityOnlineToolsData >( m_Data ), false, sender );
		}

		if ( type == CallType.Client )
		{
			Param1< ref CommunityOnlineToolsData > cdata;
			if ( !ctx.Read( cdata ) ) return;

			m_Data = cdata.param1;

			CommunityOnlineToolsModule.DATA_UPDATED.Invoke( m_Data );
		}
	}
}

static bool COT_IsUsingTestSort()
{
	CommunityOnlineToolsModule module = CommunityOnlineToolsModule.Cast( GetModuleManager().GetModule( CommunityOnlineToolsModule ) );
	if ( module && module.GetData() )
	{
		return module.GetData().TestSort;
	}
	return false;
}