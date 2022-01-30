enum JMTeleportModuleRPC
{
	INVALID = 10240,
	Load,
	Position,
	Location,
	COUNT
};

[CF_RegisterModule(JMTeleportModule)]
class JMTeleportModule: JMRenderableModuleBase
{
	private ref JMTeleportSerialize m_Settings;
	
	COTWebhook WH_Vector;
	COTWebhook WH_Location;

	override void OnInit()
	{
		super.OnInit();

		EnableRPC();
		EnableMissionLoaded();
		EnableMissionFinish();
		
		Bind("Input_Cursor", "UATeleportModuleTeleportCursor", true);
	
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

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		Load();
	}
	
	override void OnSettingsChanged(Class sender, CF_EventArgs args)
	{
		super.OnSettingsChanged(sender, args);

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

	override void OnMissionFinish(Class sender, CF_EventArgs args)
	{
		if ( GetGame().IsServer() && m_Settings )
			m_Settings.Save();
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

	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto rpcArgs = CF_EventRPCArgs.Cast(args);

		switch (rpcArgs.ID)
		{
		case JMTeleportModuleRPC.Load:
			RPC_Load(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		case JMTeleportModuleRPC.Position:
			RPC_Position(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		case JMTeleportModuleRPC.Location:
			RPC_Location(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
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

			OnSettingsChanged(this, new CF_EventArgs);
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
				OnSettingsChanged(this, new CF_EventArgs);
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
		
			WH_Vector.Send(instance, "Teleported to position " + position.ToString() );
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
			PlayerBase player = PlayerBase.Cast( players[j].PlayerObject );

			if ( player == NULL )
				continue;

			vector tempPos = "0 0 0";
			tempPos[0] = position[0] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );
			tempPos[2] = position[2] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * location.Radius );

			tempPos[1] = GetGame().SurfaceY( tempPos[0], tempPos[2] );

			SetPlayerPosition( player, tempPos );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported " + players[j].GetGUID() + " to (" + location.Name + ", " + tempPos.ToString() + ")" );
			WH_Location.Send(instance, "Teleported " + players[j].FormatSteamWebhook() + " to " + location.Name );

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
		WH_Vector.Send(instance, "Teleported to position " + position.ToString() );
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