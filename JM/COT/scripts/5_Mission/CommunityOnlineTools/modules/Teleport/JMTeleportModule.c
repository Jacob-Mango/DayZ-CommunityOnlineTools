enum JMTeleportModuleRPC
{
	INVALID = 10240,
	Load,
	Position,
	Location,
	COUNT
};

class JMTeleportModule: JMRenderableModuleBase
{
	private ref JMTeleportSerialize m_Settings;
	
	void JMTeleportModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor.NoLog" );
	
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.Player.Teleport.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleTeleport";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/teleport_form.layout";
	}

	override string GetTitle()
	{
		return "Teleport";
	}
	
	override string GetIconName()
	{
		return "T";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Teleport Module";
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
	}
	
	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( m_Settings )
		{
			if ( !m_Settings.Locations )
				return;

			for ( int i = 0; i < m_Settings.Locations.Count(); i++ )
			{
				JMTeleportLocation location = m_Settings.Locations[i];

				string permission = location.Permission;
				permission.Replace( " ", "." );
				GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location." + permission );
			}
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() && m_Settings )
			m_Settings.Save();
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		RegisterBinding( new JMModuleBinding( "Input_Cursor",				"UATeleportModuleTeleportCursor",		true 	) );
	}

	array< ref JMTeleportLocation > GetLocations()
	{
		return m_Settings.Locations;
	}

	void Input_Cursor( UAInput input )
	{
		if ( !(input.LocalPress() || input.LocalHold()) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
			return;
		}

		vector currentPosition = "0 0 0";
		vector hitPos = GetCursorPos();

		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
		{
			currentPosition = CurrentActiveCamera.GetPosition();
		} else 
		{
			currentPosition = GetPlayer().GetPosition();
		}

		float distance = vector.Distance( currentPosition, hitPos );


		if ( distance <= 1000 )
		{
			Position( hitPos, true );
		} else
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TELEPORT_TOO_FAR" ) );
		}
	}

	private void SetPlayerPosition( PlayerBase player, vector position )
	{
		if ( player.IsInTransport() )
			return;

		player.SetLastPosition();

		player.SetWorldPosition( position );
	}

	override int GetRPCMin()
	{
		return JMTeleportModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMTeleportModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMTeleportModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMTeleportModuleRPC.Position:
			RPC_Position( ctx, sender, target );
			break;
		case JMTeleportModuleRPC.Location:
			RPC_Location( ctx, sender, target );
			break;
		}
	}

	void Load()
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMTeleportModuleRPC.Load, true, NULL );
		} else
		{
			m_Settings = JMTeleportSerialize.Load();

			OnSettingsUpdated();
		}
	}

	private void Server_Load( PlayerIdentity ident )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( m_Settings );
		rpc.Send( NULL, JMTeleportModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			Server_Load( senderRPC );
		}

		if ( IsMissionClient() )
		{
			if ( ctx.Read( m_Settings ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void Position( vector position, bool isCursor = false )
	{
		if ( IsMissionOffline() )
		{
			Server_Position( position, isCursor, PlayerBase.Cast( GetGame().GetPlayer() ) );
		} else if ( IsMissionClient() )
		{
			if ( !isCursor && !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Write( isCursor );
			rpc.Send( NULL, JMTeleportModuleRPC.Position, true, NULL );
		}
	}

	private void Server_Position( vector position, bool isCursor, PlayerBase player )
	{
		JMPlayerInstance instance;
		bool shouldLog = true;
		if ( isCursor )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor", player.GetIdentity(), instance ) )
				return;

			shouldLog = !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor.NoLog", player.GetIdentity() );
		} else
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position", player.GetIdentity(), instance ) )
				return;
		}

		SetPlayerPosition( player, position );

		// COTCreateNotification( player.GetIdentity(), new StringLocaliser( "Teleported to position " + position.ToString() ) );

		if ( shouldLog )
		{
			GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
		
			SendWebhook( instance, "Teleported to position " + position.ToString() );
		}
	}

	private void RPC_Position( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
				return;

			bool isCursor;
			if ( !ctx.Read( isCursor ) )
				return;

			PlayerBase player = GetPlayerObjectByIdentity( senderRPC );
			if ( !player )
				return;

			Server_Position( pos, isCursor, player );
		}
	}

	void Location( JMTeleportLocation location, array< string > guids )
	{
		if ( IsMissionOffline() )
		{
			Server_Location( location.Permission, guids, NULL );
		} else if ( IsMissionClient() )
		{
			if ( location == NULL )
				return;

			if ( guids.Count() == 0 )
				return;

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location." + location.Permission ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( location.Permission );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Location, true, NULL );
		}
	}

	private void Server_Location( string locName, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location." + locName, ident, instance ) )
			return;

		JMTeleportLocation location = NULL;

		for ( int i = 0; i < GetLocations().Count(); i++ )
		{
			if ( GetLocations()[i].Permission == locName )
			{
				location = GetLocations()[i];
				break;
			}
		}

		if ( location == NULL )
		{
			return;
		}

		vector position = SnapToGround( location.Position );

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		
		for ( int j = 0; j < players.Count(); j++ )
		{
			PlayerBase player = players[j].PlayerObject;

			if ( player == NULL )
				continue;

			vector tempPos = "0 0 0";
			tempPos[0] = position[0] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );
			tempPos[2] = position[2] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );

			tempPos[1] = GetGame().SurfaceY( tempPos[0], tempPos[2] );

			SetPlayerPosition( player, tempPos );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported " + players[j].GetGUID() + " to (" + location.Name + ", " + tempPos.ToString() + ")" );
			SendWebhook( instance, "Teleported " + players[j].FormatSteamWebhook() + " to " + location.Name );

			players[i].Update();
		}
	}

	private void RPC_Location( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string loc;
			if ( !ctx.Read( loc ) )
			{
				return;
			}

			array< string > guids;
			if ( !ctx.Read( guids ) )
			{
				return;
			}

			Server_Location( loc, guids, senderRPC );
		}
	}
}