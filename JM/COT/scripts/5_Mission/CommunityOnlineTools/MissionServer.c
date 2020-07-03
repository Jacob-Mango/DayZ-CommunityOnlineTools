modded class MissionServer
{
	void MissionServer()
	{
		m_bLoaded = false;
		
		if ( !g_cotBase )
		{
			g_cotBase = new CommunityOnlineTools;
		}
	}

	void ~MissionServer()
	{
		delete g_cotBase;
	}

	override void OnInit()
	{
		super.OnInit();
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		g_cotBase.OnStart();
	}

	override void OnMissionFinish()
	{
		g_cotBase.OnFinish();

		super.OnMissionFinish();
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		array< string > data = new array< string >;
		GetPermissionsManager().RootPermission.Serialize( data );

		if ( !GetPermissionsManager().RoleExists( "everyone" ) )
		{
			GetPermissionsManager().CreateRole( "everyone", data );
		}

		if ( !GetPermissionsManager().RoleExists( "admin" ) )
		{
			for ( int i = 0; i < data.Count(); i++ )
			{
				string s = data[i];
				s.Replace( "0", "2" );
				data.Remove( i );
				data.InsertAt( s, i );
			}

			GetPermissionsManager().CreateRole( "admin", data );
		}
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		g_cotBase.OnUpdate( timeslice );
	}
}
