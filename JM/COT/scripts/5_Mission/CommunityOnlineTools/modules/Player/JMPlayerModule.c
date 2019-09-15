class JMPlayerModule: JMRenderableModuleBase
{
	void JMPlayerModule()
	{
		GetRPCManager().AddRPC( "COT_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "SetRoles", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "GodMode", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "SpectatePlayer", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "ToggleFreecam", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Admin", "Invisibility", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC( "COT_Admin", "HealPlayer", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC( "COT_Admin", "StripPlayer", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC( "COT_Admin", "StopBleeding", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC( "COT_Admin", "TPLastPosition", this, SingeplayerExecutionType.Server);

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
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Previous" );

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerModule()
	{
		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		if ( checked )
		{
			AddSelectedPlayer( guid );
		} else
		{
			RemoveSelectedPlayer( guid );
		}
	}

	void OnPlayer_Button( string guid, bool check )
	{
		ClearSelectedPlayers();

		if ( check )
		{
			AddSelectedPlayer( guid );
		}
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/player_form.layout";
	}

	override void OnMissionLoaded()
	{
	}

	void Player_SetHealth( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Health", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Health", data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set health to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetBlood( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Blood", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Blood", data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set blood to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetEnergy( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Energy", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatEnergy().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set energy to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetWater( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Water", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatWater().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set water to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetShock( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetHealth( "GlobalHealth", "Shock", data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set shock to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetHeatComfort( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.HeatComfort", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatHeatComfort().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set heat comfort to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetWet( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Wet", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatWet().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set wetness to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetTremor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Tremor", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatTremor().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set tremor to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetStamina( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< float, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Stamina", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.GetStatStamina().Set( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set stamina to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetLifeSpanState( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< int, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.LastShaved", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				if ( data.param1 >= LifeSpanState.BEARD_NONE && data.param1 < LifeSpanState.COUNT )
				{
					player.SetLifeSpanStateVisible( data.param1 );
				}

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set beard state to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_SetBloodyHands( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< bool, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.BloodyHands", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetBloodyHands( data.param1 );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Set bloody hands to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void Player_KickTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Kick", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

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

						GetCommunityOnlineToolsBase().Log( senderRPC, "Kicked " + players[i].GetGUID() + " out of transport" );
					}
				}
			}
		}
	}

	void Player_RepairTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Repair", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< Transport > completedTransports = new array< Transport >;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

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

					GetCommunityOnlineToolsBase().Log( senderRPC, "Repaired transport for " + players[i].GetGUID() );
				}
			}
		}
	}

	void Player_TeleportToMe( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< vector, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.ToMe", senderRPC ) ) return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				player.SetLastPosition( player.GetPosition() );

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
				} else
				{
					player.SetPosition( data.param1 );
				}

				GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported " + players[i].GetGUID() + " to self" );
			}
		}
	}

	void Player_TeleportMeTo( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param1< ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param1 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.MeTo", senderRPC ) ) return;

		PlayerBase senderPlayer = GetPlayerObjectByIdentity( senderRPC );

		if ( senderPlayer == NULL )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			if ( players.Count() == 0 )
				return;

			PlayerBase player = players[0].PlayerObject;

			if ( player == NULL )
				return;
			
			HumanCommandVehicle vehCommand = senderPlayer.GetCommand_Vehicle();

			senderPlayer.SetLastPosition( senderPlayer.GetPosition() );

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

			GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported self to " + players[0].GetGUID() );
		}
	}

	void SpectatePlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
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
				player = GetPlayerObjectByIdentity( senderRPC );
				if ( !player )
					return;

				GetGame().SelectPlayer( senderRPC, player );
				GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param, true, senderRPC );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Left spectating/free camera" );
				return;
			}

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			if ( players.Count() != 1 ) return;

			player = players[0].PlayerObject;

			if ( player == NULL ) return;

			GetGame().SelectSpectator( senderRPC, "JMSpectatorCamera", player.GetPosition() );

			GetGame().SelectPlayer( senderRPC, NULL );

			GetRPCManager().SendRPC( "COT_Admin", "SpectatePlayer", new Param, true, senderRPC, player );

			GetCommunityOnlineToolsBase().Log( senderRPC, "Spectating " + players[0].GetGUID() );
		}	

		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				CurrentActiveCamera = JMCameraBase.Cast( Camera.GetCurrentCamera() );
			}
			
			if ( CurrentActiveCamera )
			{
				CurrentActiveCamera.SelectedTarget( target );
				CurrentActiveCamera.SetActive( true );
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}
	}

	void GodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		Param2< bool, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		array< string > guids = new array< string >;
		guids.Copy( data.param2 );

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL ) continue;

				player.SetGodMode( data.param1 );
 
				GetCommunityOnlineToolsBase().Log( senderRPC, "Set god mode to " + data.param1 + " for " + players[i].GetGUID() );
			}
		}
	}

	void HealPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( type == CallType.Server )
		{
			Param1<ref array<string>> data;
			if ( !ctx.Read( data ) )
				return;
	
			array<string> guids = new array<string>;
			guids.Copy( data.param1 );

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set", senderRPC ) )
				return;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				player.GetBleedingManagerServer().RemoveAllSources();

				player.SetHealth( "GlobalHealth", "Health", player.GetMaxHealth( "GlobalHealth", "Health" ) );
				player.SetHealth( "GlobalHealth", "Blood", player.GetMaxHealth( "GlobalHealth", "Blood" ) );
				player.SetHealth( "GlobalHealth", "Shock", player.GetMaxHealth( "GlobalHealth", "Shock" ) );

				player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
				player.GetStatWater().Set( player.GetStatWater().GetMax() );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Healing player for " + players[i].GetGUID() );
			}
		}
	}

	void TPLastPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( type == CallType.Server )
		{
			Param1<ref array<string>> data;
			if (!ctx.Read(data))
				return;
	
			array<string> guids = new array<string>;
			guids.Copy( data.param1 );

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Previous", senderRPC ) )
				return;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				if ( player.HasLastPosition() )
				{
					vector newPosition = player.GetLastPosition();

					player.SetLastPosition( player.GetPosition() );

					player.SetPosition( newPosition );

					GetCommunityOnlineToolsBase().Log( senderRPC, "Teleported " + players[i].GetGUID() + " to last position" );
				}
			}
		}
	}

	void StripPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( type == CallType.Server )
		{
			Param1<ref array<string>> data;
			if (!ctx.Read(data))
				return;
	
			array<string> guids = new array<string>;
			guids.Copy( data.param1 );

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Strip", senderRPC ) )
				return;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				player.RemoveAllItems();

				GetCommunityOnlineToolsBase().Log( senderRPC, "Stripping items from " + players[i].GetGUID() );
			}
		}
	}

	void StopBleeding( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( type == CallType.Server )
		{
			Param1<ref array<string>> data;
			if (!ctx.Read(data))
				return;
	
			array<string> guids = new array<string>;
			guids.Copy( data.param1 );

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Bleeding", senderRPC ) )
				return;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				PlayerBase player = players[i].PlayerObject;

				if ( player == NULL )
					continue;

				player.GetBleedingManagerServer().RemoveAllSources();

				GetCommunityOnlineToolsBase().Log( senderRPC, "Bleeding stopped for " + players[i].GetGUID() );
			}
		}
	}

	void Invisibility(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target)
	{
		if (type == CallType.Server)
		{
			Param2<bool, ref array<string>> data;
			if (!ctx.Read(data))
				return;
	
			array<string> guids = new array<string>;
			guids.Copy(data.param2);

			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Invisible", senderRPC ) )
				return;

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
			
			for (int i = 0; i < players.Count(); i++)
			{
				PlayerBase player = players[i].PlayerObject;

				if (player == NULL) continue;

				player.SetInvisibility( data.param1 );

				GetCommunityOnlineToolsBase().Log(senderRPC, "Set invisibility to " + data.param1 + " for " + players[i].GetGUID());
			}
		}
	}

	void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Permissions", senderRPC ) )
			return;
   
		if ( type == CallType.Server )
		{
			Param2< ref array< string >, ref array< string > > data;
			if ( !ctx.Read( data ) ) return;

			ref array< string > perms = new array< string >;
			perms.Copy( data.param1 );

			ref array< string > guids = new array< string >;
			guids.Copy( data.param2 );

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int k = 0; k < players.Count(); k++ )
			{
				players[k].ClearPermissions();

				for ( int j = 0; j < perms.Count(); j++ )
				{
					players[k].AddPermission( perms[j] );
				}

				players[k].Save();

				GetRPCManager().SendRPC( "COT", "SetClientInstance", new Param4< string, ref JMPlayerInformation, PlayerIdentity, PlayerBase >( players[k].GetGUID(), players[k].Data, players[k].IdentityPlayer, players[k].PlayerObject ), false, players[k].IdentityPlayer );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Updated permissions for " + players[k].GetGUID() );
			}
		}
	}

	void SetRoles( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Roles", senderRPC ) )
			return;
   
		if ( type == CallType.Server )
		{
			Param2< ref array< string >, ref array< string > > data;
			if ( !ctx.Read( data ) ) return;

			ref array< string > roles = new array< string >;
			roles.Copy( data.param1 );

			ref array< string > guids = new array< string >;
			guids.Copy( data.param2 );

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int k = 0; k < players.Count(); k++ )
			{
				players[k].ClearRoles();
				
				for ( int j = 0; j < roles.Count(); j++ )
				{
					players[k].AddRole( roles[j] );
				}

				players[k].Save();

				GetRPCManager().SendRPC( "COT", "SetClientInstance", new Param4< string, ref JMPlayerInformation, PlayerIdentity, PlayerBase >( players[k].GetGUID(), players[k].Data, players[k].IdentityPlayer, players[k].PlayerObject ), false, players[k].IdentityPlayer );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Updated roles for " + players[k].GetGUID() );
			}
		}
	}

	void KickPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Kick", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			ref Param1< ref array< string > > data;
			if ( !ctx.Read( data ) )
				return;

			array< string > guids = new array< string >;
			guids.Copy( data.param1 );

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				GetGame().ChatPlayer( "#kick " + players[i].IdentityPlayer.GetPlayerId() );
				GetCommunityOnlineToolsBase().Log( senderRPC, "Kicked " + players[i].GetGUID() );
			}
		}
	}

	void BanPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			ref Param1< ref array< string > > data;
			if ( !ctx.Read( data ) )
				return;

			array< string > guids = new array< string >;
			guids.Copy( data.param1 );

			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

			for ( int i = 0; i < players.Count(); i++ )
			{
				GetGame().ChatPlayer( "#ban " + players[i].IdentityPlayer.GetPlayerId() );
				GetCommunityOnlineToolsBase().Log( senderRPC, "Banned " + players[i].GetGUID() );
			}
		}
	}
}