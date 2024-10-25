class JMTeleportModule: JMRenderableModuleBase
{
	JMMapModule m_MapModule;
	JMMapForm m_MapMenu;
	
	private ref JMTeleportSerialize m_Settings;
	
	void JMTeleportModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location.Add" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location.Refresh" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Location.Remove" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Cursor.NoLog" );
	
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.View" );
		
		if (Class.CastTo(m_MapModule, GetModuleManager().GetModule(JMMapModule)))
			Class.CastTo(m_MapMenu, m_MapModule.GetForm());
	}

	override void EnableUpdate()
	{
	}

	void OnSelectLocation(vector pos, bool playerpos = false)
	{
		if( !m_MapMenu )
		{
			if (!Class.CastTo(m_MapModule, GetModuleManager().GetModule(JMMapModule)))
				return;

			if (!Class.CastTo(m_MapMenu, m_MapModule.GetForm()) )
				return;
		}

		m_MapMenu.UpdateMapPosition(playerpos, pos);
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
		return "JM\\COT\\GUI\\textures\\modules\\Teleport.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
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

		if (GetGame().IsServer())
			Load();
	}
	
	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();
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

	TStringArray GetLocationTypes()
	{
		return m_Settings.Types;
	}

	void Input_Cursor( UAInput input )
	{
#ifdef DEVELOPER
		return;
#endif

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
		} else {
			currentPosition = GetPlayer().GetPosition();
		}

		float distance = vector.Distance( currentPosition, hitPos );

		if ( distance <= 1000 )
		{
			Position( hitPos, {GetGame().GetPlayer().GetIdentity().GetId()}, true);
		} else
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_TELEPORT_MODULE_NOTIFICATION_TOO_FAR" ) );
		}
	}

	void Input_Cursor_RaycastOnServer( UAInput input )
	{
#ifdef DEVELOPER
		return;
#endif

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
		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
		{
			rayStart = CurrentActiveCamera.GetPosition();
			direction = CurrentActiveCamera.GetDirection();
		} else 
		{
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

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
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
		case JMTeleportModuleRPC.AddLocation:
			RPC_AddLocation( ctx, sender, target );
			break;
		case JMTeleportModuleRPC.RemoveLocation:
			RPC_RemoveLocation( ctx, sender, target );
			break;
		}
	}

	void Reload()
	{
		m_Settings = null;
		Load();
	}

	void Load()
	{
		if ( GetGame().IsClient() )
		{
			if (m_Settings)
				return;

			m_Settings = JMTeleportSerialize.Create();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMTeleportModuleRPC.Load, true, NULL );
		} else
		{
			m_Settings = JMTeleportSerialize.Load();

			OnSettingsUpdated();
		}
	}

	bool IsLoaded()
	{
		return m_Settings != null;
	}

	private void Server_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location", ident, instance ) )
			return;

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
			if ( m_Settings.Read( ctx ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void Position( vector position, array< string > guids = NULL, bool isCursor = false )
	{
		if ( IsMissionOffline() )
		{
			Server_Position( position, isCursor, guids, NULL );
		} else if ( IsMissionClient() )
		{
			if ( !isCursor && !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Write( isCursor );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Position, true, NULL );
		}
	}

	private void Server_Position( vector position, bool isCursor, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		bool shouldLog = true;
		if ( isCursor )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor", ident, instance ) )
				return;

			shouldLog = !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Cursor.NoLog", ident );
		} else
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position", ident, instance ) )
				return;
		}

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		int count;
		bool isSelfOnly = true;
		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( !player )
				continue;
			
			if (isSelfOnly && GetGame().IsMultiplayer() && players[i].GetGUID() != ident.GetId())
				isSelfOnly = false;

			count++;
			
			SetPlayerPosition( player, position );

			if ( shouldLog )
			{
				GetCommunityOnlineToolsBase().Log( ident, "Teleported "+ players[i].GetSteam64ID() +" to position " + position.ToString() );
				SendWebhook( "Vector", instance, "Teleported "+players[i].GetSteam64ID()+" to position " + position.ToString() );
			}
		}

		if (!isSelfOnly && count > 0)
			COTCreateNotification( ident, new StringLocaliser( "Teleported "+count.ToString()+" Player(s)" ) );
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

			array< string > guids
			if ( !ctx.Read( guids ) )
				return;

			Server_Position( pos, isCursor, guids, senderRPC );
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

			PlayerBase player;
			if ( !Class.CastTo(player, senderRPC.GetPlayer()) )
				return;

			Server_PositionRaycast( pos, dir, player );
		}
	}

	void Location( JMTeleportLocation location, array< string > guids )
	{
		if ( IsMissionOffline() )
		{
			Server_Location( location.Name, guids, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( location == NULL )
				return;

			if ( guids.Count() == 0 )
				return;

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( location.Name );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Location, true, NULL );
		}
	}

	private void Server_Location( string locName, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location", ident, instance ) )
			return;

		JMTeleportLocation location = NULL;

		for ( int i = 0; i < GetLocations().Count(); i++ )
		{
			if ( GetLocations()[i].Name == locName )
			{
				location = GetLocations()[i];
				break;
			}
		}

		if ( location == NULL )
			return;

		vector position = location.Position;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		
		for ( int j = 0; j < players.Count(); j++ )
		{
			PlayerBase player = PlayerBase.Cast( players[j].PlayerObject );

			if ( player == NULL )
				continue;

			vector tempPos = position;
			tempPos[0] = position[0] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );
			tempPos[2] = position[2] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );

			tempPos = SnapToGround(tempPos);

			SetPlayerPosition( player, tempPos );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported " + players[j].GetGUID() + " to (" + location.Name + ", " + tempPos.ToString() + ")" );
			SendWebhook( "Location", instance, "Teleported " + players[j].FormatSteamWebhook() + " to " + location.Name );

			players[j].Update();
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

	void AddLocation( string locName, string catName, vector playerpos )
	{
		if ( IsMissionOffline() )
		{
			Server_AddLocation( locName, catName, playerpos, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Add" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( locName );
			rpc.Write( catName );
			rpc.Write( playerpos );
			rpc.Send( NULL, JMTeleportModuleRPC.AddLocation, true, NULL );
		}
	}

	private void RPC_AddLocation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string locName;
			if ( !ctx.Read( locName ) )
				return;

			string catName;
			if ( !ctx.Read( catName ) )
				return;

			vector playerpos;
			if ( !ctx.Read( playerpos ) )
				return;

			Server_AddLocation( locName, catName, playerpos, senderRPC );
		}
	}

	private void Server_AddLocation( string locName, string catName, vector playerpos, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Add", ident, instance ) )
			return;
		
		if (m_Settings.Types.Find(catName) == -1)
			m_Settings.Types.Insert(catName);

		m_Settings.AddLocation(catName, locName, playerpos);

		m_Settings.Save();

		OnSettingsUpdated();

		GetCommunityOnlineToolsBase().Log( ident, "Added TP Location " + locName + " (" + catName + ": "+ playerpos +" )" );
		SendWebhook( "Location", instance, "Added TP " + locName + " (" + catName + ": "+ playerpos +" )" );
	}

	private void RPC_RemoveLocation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			JMTeleportLocation locName;
			if ( !ctx.Read( locName ) )
				return;

			Server_RemoveLocation( locName, senderRPC );
		}
	}

	void RemoveLocation( JMTeleportLocation locName )
	{
		if ( IsMissionOffline() )
		{
			Server_RemoveLocation( locName, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Remove" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( locName );
			rpc.Send( NULL, JMTeleportModuleRPC.RemoveLocation, true, NULL );
		}
	}

	private void Server_RemoveLocation( JMTeleportLocation locName, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Remove", ident, instance ) )
			return;

		int id;
		bool hasType;
		for(int i=0; i < m_Settings.Locations.Count(); i++)
		{
			if (m_Settings.Locations[i].Type == locName.Type)
			{
				if (m_Settings.Locations[i].Name == locName.Name)
					id = i;
				else
					hasType = true;

				if (id > -1)
					break;
			}
		}
		
		if (id > -1)
			m_Settings.Locations.Remove(id);

		if (!hasType)
		{
			for(i=0; i < m_Settings.Types.Count(); i++)
			{
				if (m_Settings.Types[i] == locName.Type)
				{
					m_Settings.Types.Remove(i);
					break;
				}
			}
		}

		m_Settings.Save();

		OnSettingsUpdated();

		GetCommunityOnlineToolsBase().Log( ident, "Removed TP Location " + locName.Name + " (" + locName.Type + ")" );
		SendWebhook( "Location", instance, "Removed TP " + locName.Name + " (" + locName.Type + ")" );
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
};
