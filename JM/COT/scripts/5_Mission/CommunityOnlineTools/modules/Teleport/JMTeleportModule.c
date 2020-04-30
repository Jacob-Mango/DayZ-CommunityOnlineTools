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
	private ref JMTeleportSerialize settings;
	
	void JMTeleportModule()
	{
		GetPermissionsManager().RegisterPermission( "Teleport.Position" );
		GetPermissionsManager().RegisterPermission( "Teleport.Location" );
	
		GetPermissionsManager().RegisterPermission( "Teleport.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Teleport.View" );
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

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
	}
	
	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( settings )
		{
			if ( !settings.Locations )
				return;

			for ( int i = 0; i < settings.Locations.Count(); i++ )
			{
				JMTeleportLocation location = settings.Locations[i];

				string permission = location.Permission;
				permission.Replace( " ", "." );
				GetPermissionsManager().RegisterPermission( "Teleport.Location." + permission );
			}
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		RegisterBinding( new JMModuleBinding( "Input_Cursor",				"UATeleportModuleTeleportCursor",	true 	) );
		RegisterBinding( new JMModuleBinding( "Input_WorldToModelLookAt",	"UATeleportModuleWorldToModelLookAt",	true 	) );
	}

	array< ref JMTeleportLocation > GetLocations()
	{
		return settings.Locations;
	}

	void Input_WorldToModelLookAt( UAInput input )
	{
		if ( !(input.LocalPress()) )
			return;

		vector dir = GetGame().GetPointerDirection();

		vector from = GetGame().GetCurrentCameraPosition();

		vector to = from + ( dir * 10000 );

		vector rayStart = from;
		vector rayEnd = to;
		vector hitPos;
		vector hitNormal;
		int hitComponentIndex;

		set< Object > objs = new set< Object >;
		if ( DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, objs, NULL, GetPlayer(), false, false, ObjIntersectGeom) )
		{
			if ( objs.Count() > 0 ) 
			{
				vector position = objs[0].CoordToLocal( hitPos );

				Print( "Model: " + objs[0].GetType() + ", Position: " + position );

				Message( NULL, "(Logged) Model: " + objs[0].GetType() + ", Position: " + position );
			}
		}
	}

	void Input_Cursor( UAInput input )
	{
		if ( !(input.LocalPress() || input.LocalHold()) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Teleport.Cursor" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
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
			Position( hitPos );
		} else
		{
			CreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TELEPORT_TOO_FAR" ) );
		}
	}

	private void SetPlayerPosition( PlayerBase player, vector position )
	{
		if ( player.IsInTransport() )
			return;

		player.SetLastPosition( player.GetPosition() );

		Object parent;
		if ( Class.CastTo( parent, player.GetParent() ) )
		{
			position = parent.WorldToModel( position );
		}

		player.SetPosition( position );
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
			settings = JMTeleportSerialize.Load();

			OnSettingsUpdated();
		}
	}

	private void Server_Load( PlayerIdentity ident )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( settings );
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
			if ( ctx.Read( settings ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void Position( vector position )
	{
		if ( IsMissionOffline() )
		{
			Server_Position( position, PlayerBase.Cast( GetGame().GetPlayer() ) );
		} else if ( IsMissionClient() )
		{
			if ( !GetPermissionsManager().HasPermission( "Teleport.Position" ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Send( NULL, JMTeleportModuleRPC.Position, true, NULL );
		}
	}

	private void Server_Position( vector position, PlayerBase player )
	{
		if ( !GetPermissionsManager().HasPermission( "Teleport.Position", player.GetIdentity() ) )
			return;

		SetPlayerPosition( player, position );

		GetCommunityOnlineToolsBase().Log( player.GetIdentity(), "Teleported to position " + position.ToString() );
	}

	private void RPC_Position( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			vector pos;
			if ( !ctx.Read( pos ) )
			{
				return;
			}

			PlayerBase player = GetPlayerObjectByIdentity( senderRPC );

			if ( !player )
				return;

			Server_Position( pos, player );
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

			if ( !GetPermissionsManager().HasPermission( "Teleport.Location." + location.Permission ) )
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( location.Permission );
			rpc.Write( guids );
			rpc.Send( NULL, JMTeleportModuleRPC.Location, true, NULL );
		}
	}

	private void Server_Location( string locName, array< string > guids, PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().HasPermission( "Teleport.Location." + locName, ident ) )
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