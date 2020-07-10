enum JMPlayerModuleRPC
{
	INVALID = 10320,
	SetHealth,
	SetBlood,
	SetShock,
	SetEnergy,
	SetWater,
	SetStamina,
	SetBloodyHands,
	RepairTransport,
	TeleportTo,
	TeleportSenderTo,
	TeleportToPrevious,
	StartSpectating,
	EndSpectating,
	SetGodMode,
	SetFreeze,
	SetInvisible,
	SetUnlimitedAmmo,
	Heal,
	Strip,
	StopBleeding,
	SetPermissions,
	SetRoles,
	COUNT
};

class JMPlayerModule: JMRenderableModuleBase
{
	void JMPlayerModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Freeze" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Invisibility" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.UnlimitedAmmo" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.StartSpectating" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Invisible" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Strip" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.StopBleeding" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.SenderTo" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Previous" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Roles" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Health" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Shock" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Blood" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Energy" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Water" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Stamina" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.BloodyHands" );

		GetPermissionsManager().RegisterPermission( "Admin.Transport.Repair" );

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
			JM_GetSelected().AddPlayer( guid );
		} else
		{
			JM_GetSelected().RemovePlayer( guid );
		}
	}

	void OnPlayer_Button( string guid, bool check )
	{
		JM_GetSelected().ClearPlayers();

		if ( check )
		{
			JM_GetSelected().AddPlayer( guid );
		}
	}

	override string GetInputToggle()
	{
		return "UACOTTogglePlayer";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/player_form.layout";
	}

	override string GetTitle()
	{
		return "Player Management";
	}
	
	override string GetIconName()
	{
		return "P";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Player Management Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Set" );
		types.Insert( "Vehicle" );
		types.Insert( "Teleport" );
		types.Insert( "Inventory" );
		types.Insert( "PF" );
	}

	override int GetRPCMin()
	{
		return JMPlayerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMPlayerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMPlayerModuleRPC.SetHealth:
			RPC_SetHealth( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetBlood:
			RPC_SetBlood( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetShock:
			RPC_SetShock( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetEnergy:
			RPC_SetEnergy( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetWater:
			RPC_SetWater( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetStamina:
			RPC_SetStamina( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetBloodyHands:
			RPC_SetBloodyHands( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.RepairTransport:
			RPC_RepairTransport( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportTo:
			RPC_TeleportTo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportSenderTo:
			RPC_TeleportSenderTo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.TeleportToPrevious:
			RPC_TeleportToPrevious( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.StartSpectating:
			RPC_StartSpectating( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.EndSpectating:
			RPC_EndSpectating( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetGodMode:
			RPC_SetGodMode( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetFreeze:
			RPC_SetFreeze( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetInvisible:
			RPC_SetInvisible( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetUnlimitedAmmo:
			RPC_SetUnlimitedAmmo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Heal:
			RPC_Heal( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Strip:
			RPC_Strip( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.StopBleeding:
			RPC_StopBleeding( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetPermissions:
			RPC_SetPermissions( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetRoles:
			RPC_SetRoles( ctx, sender, target );
			break;
		}
	}

	void SetHealth( float health, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetHealth( health, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( health );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetHealth, true, NULL );
		}
	}

	private void Exec_SetHealth( float health, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Health", health );

			GetCommunityOnlineToolsBase().Log( ident, "Set Health To " + health + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " health to " + health );

			players[i].Update();
		}
	}

	private void RPC_SetHealth( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float health;
		if ( !ctx.Read( health ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Health", senderRPC, instance ) )
			return;

		Exec_SetHealth( health, guids, senderRPC, instance );
	}

	void SetBlood( float blood, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetBlood( blood, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( blood );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetBlood, true, NULL );
		}
	}

	private void Exec_SetBlood( float blood, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Blood", blood );

			GetCommunityOnlineToolsBase().Log( ident, "Set Blood To " + blood + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " blood to " + blood );

			players[i].Update();
		}
	}

	private void RPC_SetBlood( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float blood;
		if ( !ctx.Read( blood ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Blood", senderRPC, instance ) )
			return;

		Exec_SetBlood( blood, guids, senderRPC, instance );
	}

	void SetShock( float shock, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetShock( shock, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( shock );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetShock, true, NULL );
		}
	}

	private void Exec_SetShock( float shock, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Shock", shock );

			GetCommunityOnlineToolsBase().Log( ident, "Set Shock To " + shock + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " shock to " + shock );

			players[i].Update();
		}
	}

	private void RPC_SetShock( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float shock;
		if ( !ctx.Read( shock ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock", senderRPC, instance ) )
			return;

		Exec_SetShock( shock, guids, senderRPC, instance );
	}

	void SetEnergy( float energy, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetEnergy( energy, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( energy );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetEnergy, true, NULL );
		}
	}

	private void Exec_SetEnergy( float energy, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.GetStatEnergy().Set( energy );

			GetCommunityOnlineToolsBase().Log( ident, "Set Energy To " + energy + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " energy to " + energy );

			players[i].Update();
		}
	}

	private void RPC_SetEnergy( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float energy;
		if ( !ctx.Read( energy ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Energy", senderRPC, instance ) )
			return;

		Exec_SetEnergy( energy, guids, senderRPC, instance );
	}

	void SetWater( float water, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetWater( water, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( water );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetWater, true, NULL );
		}
	}

	private void Exec_SetWater( float water, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.GetStatWater().Set( water );

			GetCommunityOnlineToolsBase().Log( ident, "Set Water To " + water + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " water to " + water );

			players[i].Update();
		}
	}

	private void RPC_SetWater( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float water;
		if ( !ctx.Read( water ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Water", senderRPC, instance ) )
			return;

		Exec_SetWater( water, guids, senderRPC, instance );
	}

	void SetStamina( float stamina, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetStamina( stamina, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( stamina );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetStamina, true, NULL );
		}
	}

	private void Exec_SetStamina( float stamina, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.GetStatStamina().Set( stamina );

			GetCommunityOnlineToolsBase().Log( ident, "Set Stamina To " + stamina + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " stamina to " + stamina );

			players[i].Update();
		}
	}

	private void RPC_SetStamina( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float stamina;
		if ( !ctx.Read( stamina ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Stamina", senderRPC, instance ) )
			return;

		Exec_SetStamina( stamina, guids, senderRPC, instance );
	}

	void SetBloodyHands( bool bloodyhands, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetBloodyHands( bloodyhands, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( bloodyhands );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetBloodyHands, true, NULL );
		}
	}

	private void Exec_SetBloodyHands( bool bloodyhands, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.SetBloodyHands( bloodyhands );

			GetCommunityOnlineToolsBase().Log( ident, "Set BloodyHands To " + bloodyhands + " [guid=" + players[i].GetGUID() + "]" );

			if ( bloodyhands )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " bloody hands" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " bloody hands" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetBloodyHands( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool bloodyhands;
		if ( !ctx.Read( bloodyhands ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.BloodyHands", senderRPC, instance ) )
			return;

		Exec_SetBloodyHands( bloodyhands, guids, senderRPC, instance );
	}

	void RepairTransport( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_RepairTransport( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.RepairTransport, true, NULL );
		}
	}

	private void Exec_RepairTransport( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< Transport > transports = new array< Transport >;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			Transport transport;
			if ( !Class.CastTo( transport, player.GetParent() ) )
				continue;

			if ( transports.Find( transport ) > -1 )
				continue;

			transports.Insert( transport );

			ItemBase radiator;
			if ( Class.CastTo( radiator, transport.FindAttachmentBySlotName( "CarRadiator" ) ) )
			{
				radiator.SetHealth( "", "", 100 );
			}

			transport.SetHealth( "Engine", "", 100 );
			transport.SetHealth( "FuelTank", "", 100 );

			CarScript car;
			if ( Class.CastTo( car, transport ) )
			{
				car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ) );
				car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ) );
				car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ) );
				car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ) );
			}

			GetCommunityOnlineToolsBase().Log( ident, "Repaired Transport [guid=" + players[i].GetGUID() + "]" );
			SendWebhook( "Vehicle", instance, "Repaired " + players[i].FormatSteamWebhook() + " vehicle" );

			players[i].Update();
		}
	}

	private void RPC_RepairTransport( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target  )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Transport.Repair", senderRPC, instance ) )
			return;

		Exec_RepairTransport( guids, senderRPC, instance );
	}

	void TeleportTo( vector position, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_TeleportTo( position, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportTo, true, NULL );
		}
	}

	private void Exec_TeleportTo( vector position, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL || player.IsInTransport() )
				continue;

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position );

			SendWebhook( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to " + position.ToString() );

			players[i].Update();
		}
	}

	private void RPC_TeleportTo( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector position;
		if ( !ctx.Read( position ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position", senderRPC, instance ) )
			return;

		Exec_TeleportTo( position, guids, senderRPC, instance );
	}

	void TeleportSenderTo( string guid )
	{
		if ( IsMissionHost() )
		{
			Exec_TeleportSenderTo( guid, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportSenderTo, true, NULL );
		}
	}

	private void Exec_TeleportSenderTo( string guid, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		JMPlayerInstance other = GetPermissionsManager().GetPlayer( guid );

		PlayerBase player = other.PlayerObject;
		if ( player == NULL || player.IsInTransport() )
			return;

		vector position = player.GetPosition();

		if ( Class.CastTo( player, GetPlayerObjectByIdentity( ident ) ) )
		{
			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported to " + position + " [guid=" + other.GetGUID() + "]" );
			SendWebhook( "Teleport", instance, "Teleported the admin to " + other.FormatSteamWebhook() + "" );
		}
	}

	private void RPC_TeleportSenderTo( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string guid;
		if ( !ctx.Read( guid ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.SenderTo", senderRPC, instance ) )
			return;

		Exec_TeleportSenderTo( guid, senderRPC, instance );
	}

	void TeleportToPrevious( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_TeleportToPrevious( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.TeleportToPrevious, true, NULL );
		}
	}

	private void Exec_TeleportToPrevious( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL || !player.HasLastPosition() || player.IsInTransport() )
				continue;

			vector position = player.GetLastPosition();

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position + " [previous]" );
			SendWebhook( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to their previous position" );

			players[i].Update();
		}
	}

	private void RPC_TeleportToPrevious( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Previous", senderRPC, instance ) )
			return;

		Exec_TeleportToPrevious( guids, senderRPC, instance );
	}

	void StartSpectating( string guid )
	{
		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMPlayerModuleRPC.StartSpectating, true, NULL );
		}
	}

	private void Server_StartSpectating( string guid, PlayerIdentity ident )
	{
		JMPlayerInstance spectateInstance = GetPermissionsManager().GetPlayer( guid );
		if ( !spectateInstance )
			return;

		PlayerBase spectatePlayer = spectateInstance.PlayerObject;
		if ( !spectatePlayer )
			return;

		bool spectatorObjectExists = false;
		PlayerBase spectatorPlayer = GetPlayerObjectByIdentity( ident );
		if ( spectatorPlayer )
		{
			spectatorObjectExists = true;
		}

		if ( spectatorObjectExists )
		{
			float distance = vector.Distance( spectatorPlayer.GetPosition(), spectatePlayer.GetPosition() );
			if ( distance >= 1000 )
			{
				// notify error using notifications
				return;
			}
		}

		GetGame().SelectSpectator( ident, "JMSpectatorCamera", spectatePlayer.GetPosition() );
		GetGame().SelectPlayer( ident, NULL );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( spectatePlayer, JMPlayerModuleRPC.StartSpectating, true, ident );

		GetCommunityOnlineToolsBase().Log( ident, "Spectating [guid=" + guid + "]" );
	}

	private void Client_StartSpectating( PlayerBase player, PlayerIdentity ident )
	{
		CurrentActiveCamera = JMCameraBase.Cast( Camera.GetCurrentCamera() );
		
		if ( CurrentActiveCamera )
		{
			CurrentActiveCamera.SelectedTarget( player );
			CurrentActiveCamera.SetActive( true );
			
			if ( GetPlayer() )
			{
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}
	}

	private void RPC_StartSpectating( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance instance;
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
				return;

			Server_StartSpectating( guid, senderRPC );
		} else
		{
			PlayerBase player;
			if ( Class.CastTo( player, target ) )
			{
				Client_StartSpectating( player, senderRPC );
			}
		}
	}

	void EndSpectating()
	{
		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( GetGame().GetPlayer(), JMPlayerModuleRPC.EndSpectating, true, NULL );
		}
	}

	private void Server_EndSpectating( PlayerBase player, PlayerIdentity ident )
	{
		GetGame().SelectPlayer( ident, player );

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, ident );
	}

	private void Client_EndSpectating( PlayerIdentity ident )
	{
		if ( CurrentActiveCamera )
		{
			CurrentActiveCamera.SetActive( false );
			CurrentActiveCamera = NULL;
			
			PPEffects.ResetDOFOverride();
		}

		if ( GetGame().GetPlayer() )
		{
			GetGame().GetPlayer().GetInputController().SetDisabled( false );
		}
	}

	private void RPC_EndSpectating( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			PlayerBase player;
			if ( Class.CastTo( player, target ) )
			{
				Server_EndSpectating( player, senderRPC );
			}
		} else
		{
			Client_EndSpectating( senderRPC );
		}
	}

	void SetGodMode( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetGodMode( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetGodMode, true, NULL );
		}
	}

	private void Exec_SetGodMode( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.COTSetGodMode( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set GodMode To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " god mode" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " god mode" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetGodMode( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.GodMode", senderRPC, instance ) )
			return;

		Exec_SetGodMode( value, guids, senderRPC, instance );
	}

	void SetFreeze( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetFreeze( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetFreeze, true, NULL );
		}
	}

	private void Exec_SetFreeze( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.COTSetFreeze( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set Freeze To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " frozen" );
			} else
			{
				SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " unfrozen" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetFreeze( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Freeze", senderRPC, instance ) )
			return;

		Exec_SetFreeze( value, guids, senderRPC, instance );
	}

	void SetInvisible( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetInvisible( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetInvisible, true, NULL );
		}
	}

	private void Exec_SetInvisible( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.COTSetInvisibility( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set Invisibility To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " invisibility" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " invisibility" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetInvisible( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Invisibility", senderRPC, instance ) )
			return;

		Exec_SetInvisible( value, guids, senderRPC, instance );
	}

	void SetUnlimitedAmmo( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetUnlimitedAmmo( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetUnlimitedAmmo, true, NULL );
		}
	}

	private void Exec_SetUnlimitedAmmo( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.COTSetUnlimitedAmmo( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set UnlimitedAmmo To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " unlimited ammo" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " unlimited ammo" );
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_SetUnlimitedAmmo( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.UnlimitedAmmo", senderRPC, instance ) )
			return;

		Exec_SetUnlimitedAmmo( value, guids, senderRPC, instance );
	}

	void Heal( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Heal( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Heal, true, NULL );
		}
	}

	private void Exec_Heal( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			if ( player.GetBleedingManagerServer() )
				player.GetBleedingManagerServer().RemoveAllSources();

			player.SetHealth( "GlobalHealth", "Health", player.GetMaxHealth( "GlobalHealth", "Health" ) );
			player.SetHealth( "GlobalHealth", "Blood", player.GetMaxHealth( "GlobalHealth", "Blood" ) );
			player.SetHealth( "GlobalHealth", "Shock", player.GetMaxHealth( "GlobalHealth", "Shock" ) );

			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );

			GetCommunityOnlineToolsBase().Log( ident, "Healed [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Healed " + players[i].FormatSteamWebhook() );

			players[i].Update();
		}
	}

	private void RPC_Heal( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set", senderRPC, instance ) )
			return;

		Exec_Heal( guids, senderRPC, instance );
	}
	
	void Strip( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Strip( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Strip, true, NULL );
		}
	}

	private void Exec_Strip( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.RemoveAllItems();

			GetCommunityOnlineToolsBase().Log( ident, "Stripped [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Inventory", instance, "Stripped " + players[i].FormatSteamWebhook() );

			players[i].Update();
		}
	}

	private void RPC_Strip( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Strip", senderRPC, instance ) )
			return;

		Exec_Strip( guids, senderRPC, instance );
	}
	
	void StopBleeding( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_StopBleeding( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.StopBleeding, true, NULL );
		}
	}

	private void Exec_StopBleeding( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players[i].PlayerObject;
			if ( player == NULL )
				continue;

			player.GetBleedingManagerServer().RemoveAllSources();

			GetCommunityOnlineToolsBase().Log( ident, "Bleeding stopped [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Stopped " + players[i].FormatSteamWebhook() + " bleeding." );

			players[i].Update();
		}
	}

	private void RPC_StopBleeding( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.StopBleeding", senderRPC, instance ) )
			return;

		Exec_StopBleeding( guids, senderRPC, instance );
	}

	void SetPermissions( array< string > permissions, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetPermissions( permissions, guids, NULL );
		} else
		{
			#ifdef JM_COT_LOGGING
			Print( "IsMissionHost() false" );
			#endif
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( permissions );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetPermissions, true, NULL );
		}
	}

	private void Exec_SetPermissions( array< string > permissions, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{		
			players[i].LoadPermissions( permissions );

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );

			GetCommunityOnlineToolsBase().Log( ident, "Updated permissions [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "PF", instance, "Updated permissions for " + players[i].FormatSteamWebhook() );
		}
	}

	private void RPC_SetPermissions( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > permissions;
		if ( !ctx.Read( permissions ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Permissions", senderRPC, instance ) )
			return;

		Exec_SetPermissions( permissions, guids, senderRPC, instance );
	}

	void SetRoles( array< string > roles, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetRoles( roles, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( roles );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetRoles, true, NULL );
		}
	}

	private void Exec_SetRoles( array< string > roles, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{		
			players[i].LoadRoles( roles );

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );

			GetCommunityOnlineToolsBase().Log( ident, "Updated roles [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "PF", instance, "Updated roles for " + players[i].FormatSteamWebhook() );
		}
	}

	private void RPC_SetRoles( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > roles;
		if ( !ctx.Read( roles ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Roles", senderRPC, instance ) )
			return;

		Exec_SetRoles( roles, guids, senderRPC, instance );
	}
}