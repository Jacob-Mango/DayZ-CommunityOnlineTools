class PlayerModule: EditorModule
{
	void PlayerModule()
	{
		GetRPCManager().AddRPC( "COT_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "SetRoles", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "GodMode", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "SpectatePlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "ToggleFreecam", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Invisibility", this, SingeplayerExecutionType.Server);

		GetRPCManager().AddRPC( "COT_Admin", "Player_SetHealth", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetBlood", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetEnergy", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetWater", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetShock", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetHeatComfort", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetWet", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetTremor", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetStamina", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetLifeSpanState", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_SetBloodyHands", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_Admin", "Player_KickTransport", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_RepairTransport", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_TeleportToMe", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Player_TeleportMeTo", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "Admin.Permissions" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles" );
		
		GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Spectate" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Invisible" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Health" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Shock" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Blood" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Energy" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Water" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.HeatComfort" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Wet" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Tremor" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Stamina" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.LastShaved" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.BloodyHands" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Transport.Kick" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Transport.Repair" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.ToMe" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.MeTo" );
	}

	void ~PlayerModule()
	{
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/player/PlayerMenu.layout";
	}

	override void OnMissionLoaded()
	{
	}

	void Player_SetHealth( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Health", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Health", data.param1 );

				COTLog( sender, "Set health to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your health has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Health has been set to " + data.param1 );
			}
		}
	}

	void Player_SetBlood( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Blood", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Blood", data.param1 );

				COTLog( sender, "Set blood to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your blood has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Blood has been set to " + data.param1 );
			}
		}
	}

	void Player_SetEnergy( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Energy", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatEnergy().Set( data.param1 );

				COTLog( sender, "Set energy to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your energy has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Energy has been set to " + data.param1 );
			}
		}
	}

	void Player_SetWater( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Water", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatWater().Set( data.param1 );

				COTLog( sender, "Set water to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your water has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Water has been set to " + data.param1 );
			}
		}
	}

	void Player_SetShock( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Shock", data.param1 );

				COTLog( sender, "Set shock to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your shock has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Shock has been set to " + data.param1 );
			}
		}
	}

	void Player_SetHeatComfort( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.HeatComfort", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatHeatComfort().Set( data.param1 );

				COTLog( sender, "Set heat comfort to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your heat comfort has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Heat comfort has been set to " + data.param1 );
			}
		}
	}

	void Player_SetWet( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Wet", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatWet().Set( data.param1 );

				COTLog( sender, "Set wetness to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your wetness has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Wetness has been set to " + data.param1 );
			}
		}
	}

	void Player_SetTremor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Tremor", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatTremor().Set( data.param1 );

				COTLog( sender, "Set tremor to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your tremor has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Tremor has been set to " + data.param1 );
			}
		}
	}

	void Player_SetStamina( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Stamina", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatStamina().Set( data.param1 );

				COTLog( sender, "Set stamina to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your stamina has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Stamina has been set to " + data.param1 );
			}
		}
	}

	void Player_SetLifeSpanState( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< int, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.LastShaved", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				if ( data.param1 >= LifeSpanState.BEARD_NONE && data.param1 < LifeSpanState.COUNT )
				{
					player.SetLifeSpanStateVisible( data.param1 );
				}

				COTLog( sender, "Set beard state to " + data.param1 + " for " + players[i].Data.SGUID );

				//SendAdminNotification( sender, player.GetIdentity(), "Your beard state has been set to " + data.param1 );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Beard state has been set to " + data.param1 );
			}
		}
	}

	void Player_SetBloodyHands( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< bool, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.BloodyHands", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetBloodyHands( data.param1 );

				COTLog( sender, "Set bloody hands to " + data.param1 + " for " + players[i].Data.SGUID );

				if ( data.param1 )
				{
					//SendAdminNotification( sender, player.GetIdentity(), "You now have bloody hands." );
				} else 
				{
					//SendAdminNotification( sender, player.GetIdentity(), "You no longer have bloody hands." );
				}

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Bloody hands has been set to " + data.param1 );
			}
		}
	}

	void Player_KickTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Kick", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

				if ( vehCommand == NULL ) continue;

				Transport trans = vehCommand.GetTransport();
				
				if ( trans )
				{
					Car car;
					if ( Class.CastTo(car, trans) )
					{
						float speed = car.GetSpeedometer();
						if ( speed <= 8 )
						{
							vehCommand.GetOutVehicle();
						} else
						{
							vehCommand.JumpOutVehicle();
						}

						COTLog( sender, "Kicked " + players[i].Data.SGUID + " out of transport" );

						//SendAdminNotification( sender, player.GetIdentity(), "You have been kicked out of your vehicle." );

						//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
						//	SendAdminNotification( player.GetIdentity(), sender, "Kicked out of vehicle." );
					}
				}
			}
		}
	}

	void Player_RepairTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Repair", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< Transport > completedTransports = new array< Transport >;

			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL) continue;

				HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

				if ( vehCommand == NULL ) continue;

				Transport transport = vehCommand.GetTransport();

				if ( transport == NULL ) continue;

				if ( completedTransports.Find( transport ) > -1 )
				{
					ItemBase radiator;

					Class.CastTo( radiator, transport.FindAttachmentBySlotName("CarRadiator") );

					if ( radiator )
					{
						radiator.SetHealth( "", "", 100 );
					}

					transport.SetHealth( "Engine", "", 100 );
					transport.SetHealth( "FuelTank", "", 100 );

					CarScript car = CarScript.Cast( transport );

					if ( car )
					{
						car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ) );
						car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ) );
						car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ) );
						car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ) );
					}

					completedTransports.Insert( transport );

					COTLog( sender, "Repaired transport for " + players[i].Data.SGUID );

					//SendAdminNotification( sender, player.GetIdentity(), "Your vehicle has been repaired." );

					//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
					//	SendAdminNotification( player.GetIdentity(), sender, "Vehicle has been repaired." );
				}
			}
		}
	}

	void Player_TeleportToMe( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< vector, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.ToMe", sender ) ) return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();
				
				if ( vehCommand )
				{
					Transport transport = vehCommand.GetTransport();

					if ( transport )
					{
						transport.SetOrigin( data.param1 );
						transport.SetPosition( data.param1 );
						transport.Update();
					}
				}

				player.SetPosition( data.param1 );

				COTLog( sender, "Teleported " + players[i].Data.SGUID + " to self" );

				//SendAdminNotification( sender, player.GetIdentity(), "You have been teleported to " + VectorToString( data.param1, 1 ) );

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "Teleported to " + VectorToString( data.param1, 1 ) );
			}
		}
	}

	void Player_TeleportMeTo( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.MeTo", sender ) ) return;

		PlayerBase senderPlayer = GetPlayerObjectByIdentity( sender );

		if ( senderPlayer == NULL ) return;


		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			if ( players.Count() == 0 )
				return;

			PlayerBase player = players[0].PlayerObject;

			if ( player == NULL )
				return;
			
			HumanCommandVehicle vehCommand = senderPlayer.GetCommand_Vehicle();

			if ( vehCommand )
			{
				Transport transport = vehCommand.GetTransport();

				if ( transport == NULL )
					return;

				transport.SetOrigin( player.GetPosition() );
				transport.SetPosition( player.GetPosition() );
				transport.Update();
			} else
			{
				senderPlayer.SetPosition( player.GetPosition() );
			}

			COTLog( sender, "Teleported self to " + players[0].Data.SGUID );

			//SendAdminNotification( sender, player.GetIdentity(), "Teleported themself to you." );

			//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
			//	SendAdminNotification( player.GetIdentity(), sender, "You've teleported to player." );
		}
	}

	void SpectatePlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", sender ) )
			return;

		if ( type == CallType.Server )
		{
			Param2< bool, ref array< string > > data;
			if ( !ctx.Read( data ) )
				return;

			array< string > guids = new array< string >;
			guids.Copy( data.param2 );

			PlayerBase player;
			
			if ( !data.param1 )
			{
				player = GetPlayerObjectByIdentity( sender );
				if ( !player ) return;

				GetGame().SelectPlayer( sender, player );
				GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param, true, sender );

				COTLog( sender, "Left spectating/free camera" );
				return;
			}

			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			if ( players.Count() != 1 ) return;

			player = players[0].PlayerObject;

			if ( player == NULL ) return;

			GetGame().SelectSpectator( sender, "SpectatorCamera", player.GetPosition() );

			GetGame().SelectPlayer( sender, NULL );

			GetRPCManager().SendRPC( "COT_Admin", "SpectatePlayer", new Param, true, sender, player );

			COTLog( sender, "Spectating " + players[0].Data.SGUID );
		}	

		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				CurrentActiveCamera = COTCamera.Cast( Camera.GetCurrentCamera() );
			}
			
			if ( CurrentActiveCamera )
			{
				CurrentActiveCamera.SelectedTarget( target );
				CurrentActiveCamera.SetActive( true );
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}
	}

	void GodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param2< bool, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode", sender ) )
			return;

		if ( type == CallType.Server )
		{
			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetGodMode( data.param1 );
 
				COTLog( sender, "Set god mode to " + data.param1 + " for " + players[i].Data.SGUID );

				if ( data.param1 )
				{
				//	SendAdminNotification( sender, player.GetIdentity(), "You now have god mode." );
				} else 
				{
				//	SendAdminNotification( sender, player.GetIdentity(), "You no longer have god mode." );
				}

				//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
				//	SendAdminNotification( player.GetIdentity(), sender, "God mode has been set to " + data.param1 );
			}
		}
	}

	void Invisibility(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type == CallType.Server)
		{
			Param2<bool, ref array<string>> data;
			if (!ctx.Read(data))
				return;
	
			array<string> guids = new array<string>;
			guids.Copy(data.param2);

			if (!GetPermissionsManager().HasPermission( "Admin.Player.Invisible", sender))
				return;


			array<ref AuthPlayer> players = GetPermissionsManager().GetPlayersFromArray(guids);
			for (int i = 0; i < players.Count(); i++)
			{
				PlayerBase player = players[i].PlayerObject;

				if (player == NULL) continue;

				player.SetInvisibility(data.param1);

				//if (GetGame().IsMultiplayer())
				//	GetRPCManager().SendRPC("COT_Admin", "Invisibility", new Param1<bool> (data.param1), false, NULL, player);

				COTLog(sender, "Set invisibility to " + data.param1 + " for " + players[i].Data.SGUID);
			}
		}

		if (type == CallType.Client)
		{
			Param1<bool> cdata;
			if (!ctx.Read(cdata))
				return;

			PlayerBase cplayer = PlayerBase.Cast(target);

			if (cplayer)
				cplayer.SetInvisibility(cdata.param1);
		}
	}

	void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Permissions", sender ) )
			return;
   
		if ( type == CallType.Server )
		{
			Param2< ref array< string >, ref array< string > > data;
			if ( !ctx.Read( data ) ) return;

			ref array< string > perms = new ref array< string >;
			perms.Copy( data.param1 );

			ref array< string > guids = new ref array< string >;
			guids.Copy( data.param2 );

			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( data.param2 );

				for ( int k = 0; k < players.Count(); k++ )
				{
					AuthPlayer player = players[k];
					
					//if ( guids[i] == player.Data.SSteam64ID )
					//{
						player.ClearPermissions();

						for ( int j = 0; j < perms.Count(); j++ )
						{
							player.AddPermission( perms[j] );
						}

						GetRPCManager().SendRPC( "PermissionsFramework", "SetClientPlayer", new Param1< ref PlayerData >( player.Data ), true, player.IdentityPlayer );

						player.Save();

						COTLog( sender, "Updates permissions for " + players[j].Data.SSteam64ID );

						//SendAdminNotification( sender, player.IdentityPlayer, "Your permissions have been updated." );

						//if ( sender.GetPlainId() != player.IdentityPlayer.GetPlainId() )
						//	SendAdminNotification( player.IdentityPlayer, sender, "Updated permissions." );
					//}
				}
		}
	}

	void SetRoles( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Roles", sender ) )
			return;
   
		if ( type == CallType.Server )
		{
			Param2< ref array< string >, ref array< string > > data;
			if ( !ctx.Read( data ) ) return;

			ref array< string > roles = new ref array< string >;
			roles.Copy( data.param1 );

			ref array< string > guids = new ref array< string >;
			guids.Copy( data.param2 );

			array< ref AuthPlayer > players = GetPermissionsManager().GetPlayersFromArray( data.param2 );

				for ( int k = 0; k < players.Count(); k++ )
				{
					AuthPlayer player = players[k];
					
					//if ( guids[i] == player.Data.SSteam64ID )
					//{
						player.ClearRoles();

						for ( int j = 0; j < roles.Count(); j++ )
						{
							player.AddStringRole( roles[j] );
						}

						GetRPCManager().SendRPC( "PermissionsFramework", "SetClientPlayer", new Param1< ref PlayerData >( player.Data ), false, player.IdentityPlayer );

						player.Save();

						COTLog( sender, "Updates roles for " + players[j].Data.SSteam64ID );

						//SendAdminNotification( sender, player.IdentityPlayer, "Your roles have been updated." );

						//if ( sender.GetPlainId() != player.IdentityPlayer.GetPlainId() )
						//	SendAdminNotification( player.IdentityPlayer, sender, "Updated roles." );
					//}
				}
		}
	}

	void KickPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Kick", sender ) )
			return;

		if ( type == CallType.Server )
		{
			ref Param1< ref array< string > > data;
			if ( !ctx.Read( data ) )
				return;

			array< string > guids = new array< string >;
			guids.Copy( data.param1 );

			array< ref AuthPlayer > auPlayers = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < auPlayers.Count(); i++ )
			{
				auPlayers[i].Kick();
				COTLog( sender, "Kicked " + auPlayers[i].Data.SGUID );

				// won't ever send
				//SendAdminNotification( sender, auPlayers[i].IdentityPlayer, "You have been kicked." );

				//if ( sender.GetPlainId() != auPlayers[i].IdentityPlayer.GetPlainId() )
				//	SendAdminNotification( auPlayers[i].IdentityPlayer, sender, "Kicked player." );
			}
		}
	}

	void BanPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", sender ) )
			return;

		if ( type == CallType.Server )
		{
			ref Param1< ref array< string > > data;
			if ( !ctx.Read( data ) )
				return;

			array< string > guids = new array< string >;
			guids.Copy( data.param1 );

			array< ref AuthPlayer > auPlayers = GetPermissionsManager().GetPlayersFromArray( guids );

			for ( int i = 0; i < auPlayers.Count(); i++ )
			{
				auPlayers[i].Ban();
				COTLog( sender, "Banned " + auPlayers[i].Data.SGUID );

				// won't ever send
				//SendAdminNotification( sender, auPlayers[i].IdentityPlayer, "You have been banned." );

				//if ( sender.GetPlainId() != auPlayers[i].IdentityPlayer.GetPlainId() )
				//	SendAdminNotification( auPlayers[i].IdentityPlayer, sender, "Banned player." );
			}
		}
	}
}