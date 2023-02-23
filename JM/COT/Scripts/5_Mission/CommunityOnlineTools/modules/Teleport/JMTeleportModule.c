enum JMTeleportModuleRPC
{
	INVALID = 10240,
	Load,
	Position,
	PositionRaycast,
	Location,
	COUNT
};

class JMTeleportModule: JMRenderableModuleBase
{
	#ifdef COT_TPMENU2MAPMENU
	JMMapModule m_MapModule;
	JMMapForm m_MapMenu;
	#endif
	
	private ref JMTeleportSerialize m_Settings;
	
	void JMTeleportModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor.NoLog" );
	
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.View" );
		
		#ifdef COT_TPMENU2MAPMENU
		if (Class.CastTo(m_MapModule, GetModuleManager().GetModule(JMMapModule)))
			Class.CastTo(m_MapMenu, m_MapModule.GetForm());
		#endif
	}

	override void EnableUpdate()
	{
	}

	void OnSelectLocation(vector pos)
	{
		#ifdef COT_TPMENU2MAPMENU
		if( !m_MapMenu )
		{
			if (!Class.CastTo(m_MapModule, GetModuleManager().GetModule(JMMapModule)))
				return;

			Print("m_MapModule.GetForm() = "+m_MapModule.GetForm());
			Print("m_MapModule = "+m_MapModule);

			if (!Class.CastTo(m_MapMenu, m_MapModule.GetForm()) )
				return;
		}
		m_MapMenu.UpdateMapPos(pos);
		#endif
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
		return "#STR_COT_TELEPORT_MODULE_NAME";
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

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Vector" );
		types.Insert( "Location" );
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
		
		Bind( new JMModuleBinding( "Input_Cursor_RaycastOnServer",				"UATeleportModuleTeleportCursor",		true 	) );
	}

	array< ref JMTeleportLocation > GetLocations()
	{
		return m_Settings.Locations;
	}

	void Input_Cursor( UAInput input )
	{
		if ( !(input.LocalPress()) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		vector currentPosition = "0 0 0";
		vector hitPos = GetCursorPos( Object.Cast( GetGame().GetPlayer().GetParent() ) );

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
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_TELEPORT_MODULE_NOTIFICATION_TOO_FAR" ) );
		}
	}

	void Input_Cursor_RaycastOnServer( UAInput input )
	{
		if ( !(input.LocalPress()) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		vector rayStart;
		vector direction;
		Camera camera = Camera.GetCurrentCamera();
		if ( camera && camera.IsActive() )
		{
			Print("Input_Cursor_RaycastOnServer - Using current active camera: " + camera);
			rayStart = CurrentActiveCamera.GetPosition();
			direction = CurrentActiveCamera.GetDirection();
		} else 
		{
			Print("Input_Cursor_RaycastOnServer - Using player camera");
			rayStart = GetGame().GetCurrentCameraPosition();
			direction = GetGame().GetCurrentCameraDirection();
		}

		PositionRaycast( rayStart, direction );
	}

	private void SetPlayerPosition( PlayerBase player, vector position )
	{
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
		case JMTeleportModuleRPC.PositionRaycast:
			RPC_PositionRaycast( ctx, sender, target );
			break;
		case JMTeleportModuleRPC.Location:
			RPC_Location( ctx, sender, target );
			break;
		}
	}

	void Load()
	{
		if ( GetGame().IsClient() )
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
		m_Settings.Write( rpc );
		rpc.Send( NULL, JMTeleportModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetGame().IsDedicatedServer() )
		{
			Server_Load( senderRPC );
		}
		else
		{
			if ( !m_Settings )
				m_Settings = JMTeleportSerialize.Create();

			if ( m_Settings.Read( ctx ) )
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
		
			SendWebhook( "Vector", instance, "Teleported to position " + position.ToString() );
		}
	}

	private void RPC_Position( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
				return;

			bool isCursor;
			if ( !ctx.Read( isCursor ) )
				return;

			PlayerBase player = senderRPC.GetPlayer();
			if ( !player )
				return;

			Server_Position( pos, isCursor, player );
		}
	}

	void PositionRaycast( vector rayStart, vector direction )
	{
		if ( IsMissionOffline() )
		{
			Server_PositionRaycast( rayStart, direction, PlayerBase.Cast( GetGame().GetPlayer() ) );
		} else if ( IsMissionClient() )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( rayStart );
			rpc.Write( direction );
			rpc.Send( NULL, JMTeleportModuleRPC.PositionRaycast, true, NULL );
		}
	}

	private void Server_PositionRaycast( vector rayStart, vector direction, PlayerBase player )
	{
		auto trace = CF_Trace_0(this, "Server_PositionRaycast");

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor", player.GetIdentity(), instance ) )
			return;

		Object ignore;
		if (!Class.CastTo(ignore, player.GetParent()))
			ignore = player;

		float distance = GetGame().ServerConfigGetInt("defaultVisibility");
		if (distance < 1375)
			distance = 1375;

		bool hit;
		vector position = COT_PerformRayCast( rayStart, rayStart + direction.Normalized() * distance, ignore, hit );

		if ( !hit )
		{
			COTCreateNotification( player.GetIdentity(), new StringLocaliser( "STR_COT_TELEPORT_MODULE_NOTIFICATION_TOO_FAR" ) );
			return;
		}

		SetPlayerPosition( player, position );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor.NoLog", player.GetIdentity() ) )
		{
			GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
		
			SendWebhook( "Vector", instance, "Teleported to position " + position.ToString() );
		}
	}

	private void RPC_PositionRaycast( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
				return;

			vector dir;
			if ( !ctx.Read( dir ) )
				return;

			PlayerBase player = senderRPC.GetPlayer();
			if ( !player )
				return;

			Server_PositionRaycast( pos, dir, player );
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
			PlayerBase player = PlayerBase.Cast( players[j].PlayerObject );

			if ( player == NULL )
				continue;

			vector tempPos = "0 0 0";
			tempPos[0] = position[0] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );
			tempPos[2] = position[2] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );

			tempPos[1] = GetGame().SurfaceY( tempPos[0], tempPos[2] );

			SetPlayerPosition( player, tempPos );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported " + players[j].GetGUID() + " to (" + location.Name + ", " + tempPos.ToString() + ")" );
			SendWebhook( "Location", instance, "Teleported " + players[j].FormatSteamWebhook() + " to " + location.Name );

			players[i].Update();
		}
	}

	private void RPC_Location( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	void Command_Position(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		PlayerBase player = GetPlayerObjectByIdentity(sender);
		if (!player) return;

		vector position;
		if (params.HasNext())
		{
			float x, y, z;

			if (!params.Next(x)) return;
			if (!params.Next(y)) return;
			if (!params.Next(z)) return;

			position = Vector(x, y, z);
		} else return;

		SetPlayerPosition( player, position );

		GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
		SendWebhook( "Vector", instance, "Teleported to position " + position.ToString() );
	}

	void Command_Get(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		PlayerBase player = GetPlayerObjectByIdentity(sender);
		if (!player) return;

		Message(player, "Your position is: " + player.GetPosition());
	}

	override void GetSubCommands(inout array<ref JMCommand> commands)
	{
		AddSubCommand(commands, "position", "Command_Position", "Admin.Player.Teleport.Position");
		AddSubCommand(commands, "pos", "Command_Position", "Admin.Player.Teleport.Position");
		
		AddSubCommand(commands, "get", "Command_Get", "Admin.Player.Teleport.Position");
	}

	override array<string> GetCommandNames()
	{
		auto names = new array<string>();
		names.Insert("teleport");
		names.Insert("tp");
		return names;
	}
}