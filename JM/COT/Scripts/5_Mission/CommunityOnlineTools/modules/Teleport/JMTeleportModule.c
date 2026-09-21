class JMTeleportModule: JMRenderableModuleBase
{
	JMMapModule m_MapModule;
	JMMapForm m_MapMenu;
	int m_LastTeleportCursorTime;
	protected ref JMTeleportSerialize m_Settings;

	void JMTeleportModule()
	{
		if (Class.CastTo(m_MapModule, GetModuleManager().GetModule(JMMapModule)))
			Class.CastTo(m_MapMenu, m_MapModule.GetForm());
	}

	array< ref JMTeleportLocation > GetLocations()
	{
		return m_Settings.Locations;
	}

	TStringArray GetLocationTypes()
	{
		return m_Settings.Types;
	}

	bool IsLoaded()
	{
		return m_Settings != null;
	}

	protected void SetPlayerPosition( PlayerBase player, vector position )
	{
		player.SetLastPosition();

		player.SetWorldPosition( position );
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_TELEPORT_MODULE_NAME";
		info.WebhookTitle = "Teleport Module";
		info.Icon = "map-pin";
		info.Layout = "JM/COT/GUI/layouts/teleport_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_WORLD;
		info.ViewPermission = JMConstants.PERM_PLAYER_TELEPORT_POSITION_VIEW;
		info.InputToggle = "UACOTToggleTeleport";
		info.SetRPCRange( JMTeleportModuleRPC.INVALID, JMTeleportModuleRPC.COUNT );

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_POSITION );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_LOCATION );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_REMOVE );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR_NOLOG );

		info.AddWebhookType( "Vector" );
		info.AddWebhookType( "Location" );
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

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
			Load();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();

		Bind( new JMModuleBinding( "Input_Cursor_RaycastOnServer", "UATeleportModuleTeleportCursor", true ) );
		Bind( new JMModuleBinding( "Input_TeleportBack",           "UATeleportModuleTeleportBack",   true ) );
	}

	void Input_TeleportBack( UAInput input )
	{
		if ( !input.LocalPress() ) return;
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) ) return;

		JMPlayerModule playerModule;
		if ( !Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) ) return;

		JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();
		if ( !self ) return;

		playerModule.TeleportToPrevious( {self.GetGUID()} );
	}

	void Input_Cursor( UAInput input )
	{
#ifdef DEVELOPER
		return;
#endif

		if ( !input.LocalPress() && !input.LocalHoldBegin() && !input.LocalHold() )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			if (input.LocalPress())
				ShowInactiveNotification("STR_COT_INPUT_TELEPORT_CROSSHAIR");
			return;
		}

		vector currentPosition = "0 0 0";
		vector hitPos = GetCursorPos( Object.Cast( g_Game.GetPlayer().GetParent() ) );

		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
			currentPosition = CurrentActiveCamera.GetPosition();
		else
			currentPosition = GetPlayer().GetPosition();

		float distance = vector.Distance( currentPosition, hitPos );

		if ( distance <= 1000 )
			Position( hitPos, {g_Game.GetPlayer().GetIdentity().GetId()}, true);
		else
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_TELEPORT_MODULE_NOTIFICATION_TOO_FAR" ) );
	}

	void Input_Cursor_RaycastOnServer( UAInput input )
	{
#ifdef DEVELOPER
		return;
#endif

		if ( !input.LocalPress() && !input.LocalHoldBegin() && !input.LocalHold() )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			if (input.LocalPress())
				ShowInactiveNotification("STR_COT_INPUT_TELEPORT_CROSSHAIR");
			return;
		}

		int time = g_Game.GetTime();

		//! Rate limit to prevent *severe* server FPS drop when holding key
		if (time - m_LastTeleportCursorTime < 50)
			return;

		m_LastTeleportCursorTime = time;

		vector rayStart;
		vector direction;
		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
		{
			rayStart = CurrentActiveCamera.GetPosition();
			direction = CurrentActiveCamera.GetDirection();
		}
		else 
		{
			rayStart = g_Game.GetCurrentCameraPosition();
			direction = g_Game.GetCurrentCameraDirection();
		}

		PositionRaycast( rayStart, direction );
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
		case JMTeleportModuleRPC.EditLocation:
			RPC_EditLocation( ctx, sender, target );
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
		if ( g_Game.IsClient() )
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

	protected void Exec_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION, ident, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		m_Settings.Write( rpc );
		rpc.Send( NULL, JMTeleportModuleRPC.Load, true, ident );
	}

	protected void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			if ( !senderRPC )
				return;
			Exec_Load( senderRPC );
		}
		else if ( m_Settings.Read( ctx ) )
				OnSettingsUpdated();
	}

	void Position( vector position, array< string > guids = NULL, bool isCursor = false )
	{
		//! Saved locations, the map, the crosshair teleport and the ESP menu all
		//! land here, so recording once covers every one of them.
		JMTeleportHistory.PushPlayers( guids );

		if ( IsMissionOffline() )
		{
			Exec_Position( position, isCursor, guids, NULL );
		} else if ( IsMissionClient() )
		{
			if ( !isCursor && !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Write( isCursor );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Position, true, NULL );
		}
	}

	protected void Exec_Position( vector position, bool isCursor, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		bool shouldLog = true;
		if ( isCursor )
		{
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR, ident, instance ) )
				return;

			shouldLog = !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR_NOLOG, ident );
		} else
		{
			if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_POSITION, ident, instance ) )
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
			
			if (isSelfOnly && g_Game.IsMultiplayer() && players[i].GetGUID() != ident.GetId())
				isSelfOnly = false;

			count++;

			//! Captured before the move, one entry per targeted player, for
			//! the shared global Ctrl+Z - separate from JMTeleportHistory's
			//! own per-target stack, which the ESP menu's dedicated Undo/Redo
			//! rows still walk unchanged.
			JMActionHistory.Push( new JMTeleportSnapshotEntry( player, player.GetPosition() ), JMActionHistory.OwnerOf( ident ) );

			SetPlayerPosition( player, position );

			if ( shouldLog )
			{
				GetCommunityOnlineToolsBase().Log( ident, "Teleported "+ players[i].GetSteam64ID() +" to position " + position.ToString() );
				SendWebhookColored( "Vector", instance, "Teleported "+players[i].GetSteam64ID()+" to position " + position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );
			}
		}

		if (!isSelfOnly && count > 0)
			COTCreateNotification( ident, new StringLocaliser( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_TELEPORTED_PREFIX" ) + count.ToString() + Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_PLAYERS_SUFFIX" ) ) );
	}

	protected void RPC_Position( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
				return;

			if ( !CommunityOnlineToolsBase.IsValidWorldPosition( pos ) )
				return;

			bool isCursor;
			if ( !ctx.Read( isCursor ) )
				return;

			array< string > guids;
			if ( !ctx.Read( guids ) )
				return;

			if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
				return;

			Exec_Position( pos, isCursor, guids, senderRPC );
		}
	}

	void PositionRaycast( vector rayStart, vector direction )
	{
		//! Where the ray lands is only known on the server, but WHO is being
		//! moved is known here - it is always the admin who pressed the key.
		JMTeleportHistory.PushSelf();

		if ( IsMissionOffline() )
		{
			Exec_PositionRaycast( rayStart, direction, PlayerBase.Cast( g_Game.GetPlayer() ) );
		}
		else if ( IsMissionClient() )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( rayStart );
			rpc.Write( direction );
			rpc.Send( NULL, JMTeleportModuleRPC.PositionRaycast, true, NULL );
		}
	}

	protected void Exec_PositionRaycast( vector rayStart, vector direction, PlayerBase player )
	{
	#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Exec_PositionRaycast");
	#endif

		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR, player.GetIdentity(), instance ) )
			return;

		float distance = g_Game.ServerConfigGetInt("defaultVisibility");
		if (distance < 1375)
			distance = 1375;

		bool hit;
		vector position = COT_PerformRayCast( rayStart, rayStart + direction.Normalized() * distance, player, hit );

		if ( !hit )
		{
			COTCreateNotification( player.GetIdentity(), new StringLocaliser( "STR_COT_TELEPORT_MODULE_NOTIFICATION_TOO_FAR" ) );
			return;
		}

		SetPlayerPosition( player, position );

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION_CURSOR_NOLOG, player.GetIdentity() ) )
		{
			GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
		
			SendWebhookColored( "Vector", instance, "Teleported to position " + position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );
		}
	}

	protected void RPC_PositionRaycast( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
				return;

			vector dir;
			if ( !ctx.Read( dir ) )
				return;

			//! Both come off the wire and both feed a raycast: a NaN start or
			//! direction gives a hit position physics cannot resolve.
			if ( !CommunityOnlineToolsBase.IsValidWorldPosition( pos ) || !CommunityOnlineToolsBase.IsFiniteVector( dir ) )
				return;

			PlayerBase player;
			if ( !Class.CastTo(player, senderRPC.GetPlayer()) )
				return;

			Exec_PositionRaycast( pos, dir, player );
		}
	}

	void Location( JMTeleportLocation location, array< string > guids )
	{
		if ( IsMissionOffline() )
		{
			Exec_Location( location.Name, guids, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( location == NULL )
				return;

			if ( guids.Count() == 0 )
				return;

			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( location.Name );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Location, true, NULL );
		}
	}

	protected void Exec_Location( string locName, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_LOCATION, ident, instance ) )
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
			SendWebhookColored( "Location", instance, "Teleported " + players[j].FormatSteamWebhook() + " to " + location.Name, JMConstants.WEBHOOK_COLOR_TELEPORT );

			players[j].Update();
		}
	}

	protected void RPC_Location( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string loc;
			if ( !ctx.Read( loc ) )
				return;

			array< string > guids;
			if ( !ctx.Read( guids ) )
				return;

			Exec_Location( loc, guids, senderRPC );
		}
	}

	void AddLocation( string locName, string catName, vector playerpos )
	{
		if ( IsMissionOffline() )
		{
			Exec_AddLocation( locName, catName, playerpos, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( locName );
			rpc.Write( catName );
			rpc.Write( playerpos );
			rpc.Send( NULL, JMTeleportModuleRPC.AddLocation, true, NULL );
		}
	}

	protected void RPC_AddLocation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			//! This one is written to disk and handed back out to every admin
			//! afterwards, so a bad value outlives the packet that carried it.
			if ( !CommunityOnlineToolsBase.IsValidWorldPosition( playerpos ) )
				return;

			Exec_AddLocation( locName, catName, playerpos, senderRPC );
		}
	}

	protected void Exec_AddLocation( string locName, string catName, vector playerpos, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE, ident, instance ) )
			return;
		
		if (m_Settings.Types.Find(catName) == -1)
			m_Settings.Types.Insert(catName);

		m_Settings.AddLocation(catName, locName, playerpos);

		m_Settings.Save();

		OnSettingsUpdated();

		GetCommunityOnlineToolsBase().Log( ident, "Added TP Location " + locName + " (" + catName + ": "+ playerpos +" )" );
		SendWebhookColored( "Location", instance, "Added TP " + locName + " (" + catName + ": "+ playerpos +" )", JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	protected void RPC_RemoveLocation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			JMTeleportLocation locName;
			if ( !ctx.Read( locName ) )
				return;

			Exec_RemoveLocation( locName, senderRPC );
		}
	}

	void RemoveLocation( JMTeleportLocation locName )
	{
		if ( IsMissionOffline() )
		{
			Exec_RemoveLocation( locName, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_REMOVE ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( locName );
			rpc.Send( NULL, JMTeleportModuleRPC.RemoveLocation, true, NULL );
		}
	}

	protected void Exec_RemoveLocation( JMTeleportLocation locName, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_REMOVE, ident, instance ) )
			return;

		int id = -1;
		bool hasType = false;
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
		SendWebhookColored( "Location", instance, "Removed TP " + locName.Name + " (" + locName.Type + ")", JMConstants.WEBHOOK_COLOR_WARNING );
	}

	//! Rename and recategorise one saved location IN PLACE.
	//!
	//! A remove followed by an add would do the same job from the client, but
	//! as two independent messages: a failure or a permission change between
	//! them loses the location outright, and every other admin sees it vanish
	//! and reappear. One message cannot half-happen.
	//!
	//! The position is deliberately not editable here. Moving a saved location
	//! is a different intent from renaming one, and the coordinates panel plus
	//! Save already covers it.
	void EditLocation( JMTeleportLocation target, string newName, string newCategory )
	{
		if ( !target )
			return;

		if ( IsMissionOffline() )
		{
			Exec_EditLocation( target, newName, newCategory, NULL );
		}
		else if ( IsMissionClient() )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( target );
			rpc.Write( newName );
			rpc.Write( newCategory );
			rpc.Send( NULL, JMTeleportModuleRPC.EditLocation, true, NULL );
		}
	}

	protected void RPC_EditLocation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMTeleportLocation location;
		if ( !ctx.Read( location ) )
			return;

		string newName;
		if ( !ctx.Read( newName ) )
			return;

		string newCategory;
		if ( !ctx.Read( newCategory ) )
			return;

		Exec_EditLocation( location, newName, newCategory, senderRPC );
	}

	protected void Exec_EditLocation( JMTeleportLocation target, string newName, string newCategory, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT, ident, instance ) )
			return;

		if ( newName == "" )
			return;

		int found = -1;
		for ( int i = 0; i < m_Settings.Locations.Count(); i++ )
		{
			if ( m_Settings.Locations[i].Type != target.Type )
				continue;

			if ( m_Settings.Locations[i].Name != target.Name )
				continue;

			found = i;
			break;
		}

		if ( found < 0 )
			return;

		string oldName = m_Settings.Locations[found].Name;
		string oldType = m_Settings.Locations[found].Type;

		m_Settings.Locations[found].Name = newName;
		m_Settings.Locations[found].Type = newCategory;

		if ( m_Settings.Types.Find( newCategory ) == -1 )
			m_Settings.Types.Insert( newCategory );

		//! The old category may have just lost its last member, in which case it
		//! is not a category any more. Add does the same bookkeeping in reverse.
		if ( oldType != newCategory )
			PruneEmptyType( oldType );

		m_Settings.Save();

		OnSettingsUpdated();

		GetCommunityOnlineToolsBase().Log( ident, "Edited TP Location " + oldName + " (" + oldType + ") -> " + newName + " (" + newCategory + ")" );
		SendWebhookColored( "Location", instance, "Edited TP " + oldName + " (" + oldType + ") -> " + newName + " (" + newCategory + ")", JMConstants.WEBHOOK_COLOR_INFO );
	}

	//! Drop a category no location uses any more.
	protected void PruneEmptyType( string type )
	{
		for ( int i = 0; i < m_Settings.Locations.Count(); i++ )
		{
			if ( m_Settings.Locations[i].Type == type )
				return;
		}

		for ( int t = 0; t < m_Settings.Types.Count(); t++ )
		{
			if ( m_Settings.Types[t] != type )
				continue;

			m_Settings.Types.Remove( t );
			return;
		}
	}

	void Command_Position(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		PlayerBase player = GetPlayerObjectByIdentity(sender);
		if (!player) return;

		vector position;
		if (params.HasNext())
		{
			float x;
			if (!params.Next(x))
				return;

			float y;
			if (!params.Next(y))
				return;

			float z;
			if (!params.Next(z))
				return;

			position = Vector(x, y, z);
		}
		else
			return;

		SetPlayerPosition( player, position );

		GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
		SendWebhookColored( "Vector", instance, "Teleported to position " + position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );
	}

	void Command_Get(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		PlayerBase player = GetPlayerObjectByIdentity(sender);
		if (!player)
			return;

		Message(player, Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_YOUR_POSITION_IS" ) + player.GetPosition());
	}

	override void GetSubCommands(inout array<ref JMCommand> commands)
	{
		AddSubCommand(commands, "position", "Command_Position", JMConstants.PERM_PLAYER_TELEPORT_POSITION);
		AddSubCommand(commands, "pos", "Command_Position", JMConstants.PERM_PLAYER_TELEPORT_POSITION);		
		AddSubCommand(commands, "get", "Command_Get", JMConstants.PERM_PLAYER_TELEPORT_POSITION);
	}

	override array<string> GetCommandNames()
	{
		auto names = new array<string>();
		names.Insert("teleport");
		names.Insert("tp");
		return names;
	}
}
