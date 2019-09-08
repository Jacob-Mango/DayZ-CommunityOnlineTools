class JMTeleportModule: JMRenderableModuleBase
{
	protected ref JMTeleportSerialize settings;
	
	void JMTeleportModule()
	{
		GetRPCManager().AddRPC( "COT_Teleport", "Cursor", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Teleport", "Predefined", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Teleport", "LoadData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "Teleport.Cursor" );
		GetPermissionsManager().RegisterPermission( "Teleport.Predefined" );
	
		GetPermissionsManager().RegisterPermission( "Teleport.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Teleport.View" );
	}
	
	override void ReloadSettings()
	{
		super.ReloadSettings();

		settings = JMTeleportSerialize.Load();
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( GetGame().IsClient() )
			GetRPCManager().SendRPC( "COT_Teleport", "LoadData", new Param, true );
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "TeleportCursor",		"UATeleportModuleTeleportCursor",	true 	) );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/teleport_form.layout";
	}

	ref array< ref JMTeleportLocation > GetLocations()
	{
		return settings.Locations;
	}

	void TeleportCursor( UAInput input )
	{
		if ( !(input.LocalPress() || input.LocalHold()) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Teleport.Cursor" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
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

		if ( distance < 1000 )
		{
			GetRPCManager().SendRPC( "COT_Teleport", "Cursor", new Param1< vector >( hitPos ) );
		}
		else
		{
			CreateLocalAdminNotification( "Distance for teleportation is too far!" );
		}
	}

	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Teleport.Predefined", sender ) )
				return;
		
			GetRPCManager().SendRPC( "COT_Teleport", "LoadData", new Param1< ref JMTeleportSerialize >( settings ), false, sender );
		}

		if ( type == CallType.Client )
		{
			Param1< ref JMTeleportSerialize > data;
			if ( !ctx.Read( data ) ) return;

			settings = data.param1;
		}
	}

	void Cursor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Teleport.Cursor", sender ) )
			return;

		Param1< vector > data;
		if ( !ctx.Read( data ) ) return;

		if ( type == CallType.Server )
		{
			PlayerBase player = GetPlayerObjectByIdentity( sender );

			if ( !player )
				return;

			if ( player.IsInTransport() )
			{
				// player.GetTransport().SetOrigin( data.param1 );
			   HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

				if ( vehCommand )
				{
					Transport transport = vehCommand.GetTransport();

					if ( transport == NULL ) return;

					transport.SetOrigin( data.param1 );
					transport.SetPosition( data.param1 );
					transport.Update();
				}
			} else
			{
				player.SetPosition( data.param1 );
			}

			GetCommunityOnlineToolsBase().Log( sender, "Teleported to cursor " + data.param1 );
			//SendAdminNotification( sender, NULL, "You have teleported to " + VectorToString( data.param1, 1 ) );
		}
	}
	
	void Predefined( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Teleport.Predefined", sender ) )
			return;

		Param2< string, ref array< string > > data;
		if ( !ctx.Read( data ) ) return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( type == CallType.Server )
		{
			vector position = "0 0 0";

			string name = data.param1;

			ref JMTeleportLocation location = NULL;

			for ( int i = 0; i < GetLocations().Count(); i++ )
			{
				if ( GetLocations()[i].Name == name )
				{
					location = GetLocations()[i];
					break;
				}
			}

			if ( location == NULL )
			{
				return;
			}

			position = SnapToGround( location.Position );
			PlayerBase player;
			HumanCommandVehicle vehCommand;
			Transport transport;
			
			if ( !GetGame().IsMultiplayer() )
			{
				player = GetGame().GetPlayer();

				if ( player == NULL )
					return;
				
				if ( player.IsInTransport() )
				{
					vehCommand = player.GetCommand_Vehicle();

					if ( vehCommand )
					{
						transport = vehCommand.GetTransport();

						if ( transport == NULL ) return;

						transport.SetOrigin( position );
						transport.SetPosition( position );
						transport.Update();
					}
				} else 
				{
					player.SetPosition( position );
				}
				
				return;
			}

			array< ref JMPlayerInstance > players = GetPermissionsManager().GetPlayersFromArray( guids );
			
			for ( int j = 0; j < players.Count(); j++ )
			{
				player = players[j].PlayerObject;

				if ( player == NULL )
					continue;

				if ( player.IsInTransport() )
				{
					vehCommand = player.GetCommand_Vehicle();

					if ( vehCommand )
					{
						transport = vehCommand.GetTransport();

						if ( transport == NULL )
							return;

						transport.SetOrigin( position );
						transport.SetPosition( position );
						transport.Update();
					}
				} else 
				{
					player.SetPosition( position );
				}
				
				GetCommunityOnlineToolsBase().Log( sender, "Teleported " + players[j].Data.SSteam64ID + " to " + location.Name );
				
				SendAdminNotification( sender, player.GetIdentity(), "You have been teleported to " + data.param1 );
			}
		}
	}
}