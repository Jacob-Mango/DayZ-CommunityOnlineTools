class COTMapModule: EditorModule
{
	protected ref array< Man > m_ServerPlayers;
	
	void COTMapModule() 
	{
		m_ServerPlayers = new ref array< Man >;

		GetPermissionsManager().RegisterPermission( "Map.ShowPlayers" );
		GetPermissionsManager().RegisterPermission( "Map.Hidden" );
		GetPermissionsManager().RegisterPermission( "Map.View" );
		GetPermissionsManager().RegisterPermission( "Map.Teleport" );
		
		GetRPCManager().AddRPC( "COT_Map", "Request_Map_PlayerPositions", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Map", "Receive_Map_PlayerPositions", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Map", "MapTeleport", this, SingeplayerExecutionType.Server );
	}
	
	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/Map/COTMap.layout";
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Map.View" );
	}
	
	void Request_Map_PlayerPositions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Map.ShowPlayers", sender ) )
			return;

		if ( type == CallType.Server )
		{
			GetGame().GetPlayers( m_ServerPlayers );
			
			ref array<ref PlayerData> playerDataArray = new ref array<ref PlayerData>;
			
			for ( int j = 0; j < m_ServerPlayers.Count(); j++ )
	   		{
				Man man = m_ServerPlayers[j];

				ref PlayerData data = new PlayerData();
				data.SName = man.GetIdentity().GetName();
				data.VPosition = man.GetPosition();
				data.VDirection = man.GetDirection();
				
				playerDataArray.Insert(data);
			}
			
	 	  	m_ServerPlayers.Clear();
			
			GetRPCManager().SendRPC("COT_Map", "Receive_Map_PlayerPositions", new Param1< ref array<ref PlayerData >>( playerDataArray ), false, sender );
		}
	}
	
	void Receive_Map_PlayerPositions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server ) return;
		
		ref Param1< ref array<ref PlayerData> > data;
		
		if ( !ctx.Read( data ) ) return;
		
		if ( !GetPermissionsManager().HasPermission( "Map.ShowPlayers", sender ) )
			return;
		
		COTMapMenu.Cast(form).ShowPlayers( data.param1 );
	}
	
	void MapTeleport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Map.Teleport", sender ) )
			return;

		Param1< vector > data;
		if ( !ctx.Read( data ) ) return;

		if ( type == CallType.Server )
		{
			PlayerBase player = GetPlayerObjectByIdentity( sender );

			if ( !player ) return;

			if ( player.IsInTransport() )
			{
				HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

				if ( vehCommand )
				{
					Transport transport = vehCommand.GetTransport();

					if ( transport == NULL ) return;

					transport.SetPosition( data.param1 );
				}
			} else
			{
				player.SetPosition( data.param1 );
			}

			COTLog( sender, "Teleported to position " + data.param1 + " from map." );

			SendAdminNotification( sender, NULL, "Teleported to " + VectorToString( data.param1, 1 ) );
		}
	}
}