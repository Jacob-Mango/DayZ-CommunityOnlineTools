class JMPlayerModule: JMRenderableModuleBase
{
	PlayerBase m_SpectatorClient;
	ref map<string, PlayerBase> m_Spectators = new map<string, PlayerBase>();
	JMCameraBase m_SpectatorCamera;

	void JMPlayerModule()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Heal" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Heal.Attachments" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Heal.Cargo" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Freeze" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Invisibility" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.UnlimitedAmmo" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.UnlimitedStamina" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Spectate" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Strip" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.ClearCargo" );		
		GetPermissionsManager().RegisterPermission( "Admin.Player.Dry" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.StopBleeding" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.BrokenLegs" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.ReceiveDamageDealt" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Message" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Notif" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.CannotBeTargetedByAI" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.AccessInventory" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.RemoveCollision" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.AdminNVG" );

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
		GetPermissionsManager().RegisterPermission( "Admin.Player.Set.HeatBuffer" );
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

	override void EnableUpdate()
	{
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		Bind( new JMModuleBinding( "InputHeal",			"UAPlayerModuleHeal",		true 	) );
		Bind( new JMModuleBinding( "InputToggleGodMode",	"UAPlayerModuleGodMode",	true 	) );
		Bind( new JMModuleBinding( "InputToggleInvisibility",	"UAPlayerModuleInvisibility",	true 	) );
		Bind( new JMModuleBinding( "InputToggleCannotBeTargetedByAI",	"UAPlayerModuleCannotBeTargetedByAI",	true 	) );
		Bind( new JMModuleBinding( "InputToggleUnlimitedStamina",	"UAPlayerModuleUnlimitedStamina",	true 	) );
		Bind( new JMModuleBinding( "InputToggleUnlimitedAmmo",	"UAPlayerModuleUnlimitedAmmo",	true 	) );
		Bind( new JMModuleBinding( "InputToggleAdminNV",	"UAPlayerModuleAdminNV",	true 	) );
		Bind( new JMModuleBinding( "InputFreezePlayer",		"UAPlayerModuleFreezePlayer",		true 	) );
		Bind( new JMModuleBinding( "EndSpectating",		"UAPlayerModuleStopSpectating",		true 	) );
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		if ( checked )
			JM_GetSelected().AddPlayer( guid );
		else
			JM_GetSelected().RemovePlayer( guid );
	}

	void OnPlayer_Button( string guid, bool check )
	{
		JM_GetSelected().ClearPlayers();

		if ( check )
			JM_GetSelected().AddPlayer( guid );
	}

	override string GetInputToggle()
	{
		return "UACOTTogglePlayer";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/player_form.layout";
	}

	override string GetCategory()
	{
		return "Players";
	}

	override string GetTitle()
	{
		return "#STR_COT_PLAYER_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\Player.paa";
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
		return "Player Management Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Set" );
		types.Insert( "Vehicle" );
		types.Insert( "Teleport" );
		types.Insert( "Inventory" );
		types.Insert( "PF" );
		types.Insert( "Kick" );
		types.Insert( "Message" );
		types.Insert( "Ban" );
	}

	override int GetRPCMin()
	{
		return JMPlayerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMPlayerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMPlayerModuleRPC.SetStat:
			RPC_SetStat( ctx, sender, target );
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
		case JMPlayerModuleRPC.EndSpectating_Finish:
			RPC_EndSpectating_Finish( ctx, sender, target );
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
		case JMPlayerModuleRPC.SetRemoveCollision:
			RPC_SetRemoveCollision( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetCannotBeTargetedByAI:
			RPC_SetCannotBeTargetedByAI( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetUnlimitedAmmo:
			RPC_SetUnlimitedAmmo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetUnlimitedStamina:
			RPC_SetUnlimitedStamina( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetBrokenLegs:
			RPC_SetBrokenLegs( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Heal:
			RPC_Heal( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Strip:
			RPC_Strip( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.ClearCargo:
			RPC_ClearCargo( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Dry:
			RPC_Dry( ctx, sender, target );
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
		case JMPlayerModuleRPC.SetReceiveDamageDealt:
			RPC_SetReceiveDamageDealt( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Kick:
			RPC_Kick( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.KickMessage:
			RPC_KickMessage( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Ban:
			RPC_Ban( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.BanMessage:
			RPC_BanMessage( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Message:
			RPC_Message( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Notif:
			RPC_Notif( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.VONStartedTransmitting:
			RPC_VONStartedTransmitting( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.VONStoppedTransmitting:
			RPC_VONStoppedTransmitting( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetAdminNVG:
			RPC_SetAdminNVG( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.Vomit:
			RPC_Vomit( ctx, sender, target );
			break;
		case JMPlayerModuleRPC.SetScale:
			RPC_SetScale( ctx, sender, target );
			break;
		}
	}

	override void OnClientDisconnect( PlayerBase player, PlayerIdentity identity, string uid )
	{
		if ( m_Spectators.Contains( uid ) )
			m_Spectators.Remove( uid );
	}

	void DoMessage( array< string > guids, string messageText )
	{
		if ( IsMissionHost() )
		{
			Exec_Message( guids, NULL, NULL, messageText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(messageText);
			rpc.Send( NULL, JMPlayerModuleRPC.Message, true, NULL );
		}
	}

	private void Exec_Message( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			Message( player.PlayerObject, messageText );
		}
	}

	private void RPC_Message( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Message", senderRPC, instance ) )
			return;

		Exec_Message( guids, senderRPC, instance, messageText );
	}
	
	private void SendMessage(PlayerIdentity identity, string messageText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.Message, true, identity);
	}

	void DoNotif( array< string > guids, string NotifText )
	{
		if ( IsMissionHost() )
		{
			Exec_Notif( guids, NULL, NULL, NotifText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(NotifText);
			rpc.Send( NULL, JMPlayerModuleRPC.Notif, true, NULL );
		}
	}

	private void Exec_Notif( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string NotifText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			NotificationSystem.Create( new StringLocaliser( "#STR_COT_NOTIFICATION_MESSAGE_FROM_ADMIN" ), new StringLocaliser( NotifText ), "JM//COT//gui//textures//cot_icon.edds", COLOR_RED, 10, player.PlayerObject.GetIdentity() );
		}
	}

	private void RPC_Notif( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Notif", senderRPC, instance ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string NotifText;
		if (!ctx.Read(NotifText))
			return;

		Exec_Notif( guids, senderRPC, instance, NotifText );
	}
	
	private void SendNotif(PlayerIdentity identity, string NotifText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(NotifText);
		rpc.Send(NULL, JMPlayerModuleRPC.Notif, true, identity);
	}

	// -----------------------------------------------------------------------
	// Batched stat setter — single RPC replaces 7 individual SetX methods.
	// Public entry points below are kept as thin wrappers so call sites and
	// form code don't need to change.
	// -----------------------------------------------------------------------

	private string StatPermission( JMStatType type )
	{
		switch ( type )
		{
		case JMStatType.Health:      return "Admin.Player.Set.Health";
		case JMStatType.Blood:       return "Admin.Player.Set.Blood";
		case JMStatType.Shock:       return "Admin.Player.Set.Shock";
		case JMStatType.Energy:      return "Admin.Player.Set.Energy";
		case JMStatType.Water:       return "Admin.Player.Set.Water";
		case JMStatType.Stamina:     return "Admin.Player.Set.Stamina";
		case JMStatType.HeatBuffer:  return "Admin.Player.Set.HeatBuffer";
		}
		return "";
	}

	private string StatName( JMStatType type )
	{
		switch ( type )
		{
		case JMStatType.Health:      return "Health";
		case JMStatType.Blood:       return "Blood";
		case JMStatType.Shock:       return "Shock";
		case JMStatType.Energy:      return "Energy";
		case JMStatType.Water:       return "Water";
		case JMStatType.Stamina:     return "Stamina";
		case JMStatType.HeatBuffer:  return "HeatBuffer";
		}
		return "Unknown";
	}

	private float ClampStat( JMStatType type, float value )
	{
		switch ( type )
		{
		case JMStatType.Health:      return Math.Clamp( value, JMConstants.STAT_HEALTH_MIN,     JMConstants.STAT_HEALTH_MAX );
		case JMStatType.Blood:       return Math.Clamp( value, JMConstants.STAT_BLOOD_MIN,      JMConstants.STAT_BLOOD_MAX );
		case JMStatType.Shock:       return Math.Clamp( value, JMConstants.STAT_SHOCK_MIN,      JMConstants.STAT_SHOCK_MAX );
		case JMStatType.Energy:      return Math.Clamp( value, JMConstants.STAT_ENERGY_MIN,     JMConstants.STAT_ENERGY_MAX );
		case JMStatType.Water:       return Math.Clamp( value, JMConstants.STAT_WATER_MIN,      JMConstants.STAT_WATER_MAX );
		case JMStatType.Stamina:     return Math.Clamp( value, JMConstants.STAT_STAMINA_MIN,    JMConstants.STAT_STAMINA_MAX );
		case JMStatType.HeatBuffer:  return Math.Clamp( value, JMConstants.STAT_HEATBUFFER_MIN, JMConstants.STAT_HEATBUFFER_MAX );
		}
		return value;
	}

	private void ApplyStatToPlayer( PlayerBase player, JMStatType type, float value )
	{
		switch ( type )
		{
		case JMStatType.Health:     player.SetHealth( "GlobalHealth", "Health", value ); break;
		case JMStatType.Blood:      player.SetHealth( "GlobalHealth", "Blood",  value ); break;
		case JMStatType.Shock:      player.SetHealth( "GlobalHealth", "Shock",  value ); break;
		case JMStatType.Energy:     player.GetStatEnergy().Set( value );                 break;
		case JMStatType.Water:      player.GetStatWater().Set( value );                  break;
		case JMStatType.Stamina:    player.GetStatStamina().Set( value );                break;
		case JMStatType.HeatBuffer: player.GetStatHeatBuffer().Set( value );             break;
		}
	}

	void SetStat( JMStatType type, float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetStat( type, value, guids, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( type );
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetStat, true, NULL );
		}
	}

	private void Exec_SetStat( JMStatType type, float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		value = ClampStat( type, value );
		string statName = StatName( type );

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			ApplyStatToPlayer( player, type, value );

			GetCommunityOnlineToolsBase().Log( ident, "Set " + statName + " To " + value + " [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " " + statName + " to " + value, JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	private void RPC_SetStat( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int typeInt;
		if ( !ctx.Read( typeInt ) )
			return;

		if ( typeInt < 0 || typeInt >= JMStatType.COUNT )
			return;

		float value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMStatType type = typeInt;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( StatPermission( type ), senderRPC, instance ) )
			return;

		Exec_SetStat( type, value, guids, senderRPC, instance );
	}

	// Public wrappers — keep existing call sites working without changes
	void SetHealth(     float v, array< string > guids ) { SetStat( JMStatType.Health,     v, guids ); }
	void SetBlood(      float v, array< string > guids ) { SetStat( JMStatType.Blood,      v, guids ); }
	void SetShock(      float v, array< string > guids ) { SetStat( JMStatType.Shock,      v, guids ); }
	void SetEnergy(     float v, array< string > guids ) { SetStat( JMStatType.Energy,     v, guids ); }
	void SetWater(      float v, array< string > guids ) { SetStat( JMStatType.Water,      v, guids ); }
	void SetStamina(    float v, array< string > guids ) { SetStat( JMStatType.Stamina,    v, guids ); }
	void SetHeatBuffer( float v, array< string > guids ) { SetStat( JMStatType.HeatBuffer, v, guids ); }

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

	private void Exec_SetBloodyHands( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetBloodyHands( value );

			ProcessToggle("bloody hands", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("bloody hands", value, affectedPlayers, ident);
	}

	private void RPC_SetBloodyHands( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool bloodyhands;
		if ( !ctx.Read( bloodyhands ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
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
		array< EntityAI > vehicles = {};

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			EntityAI vehicle;
			if ( !Class.CastTo( vehicle, player.GetParent() ) )
				continue;

			if ( vehicles.Find( vehicle ) > -1 )
				continue;

			vehicles.Insert( vehicle );

			CommunityOnlineToolsBase.HealEntityRecursive(vehicle);
			CommunityOnlineToolsBase.Refuel(vehicle);
			
			JMObjectSpawnerModule objSpawnerModule;
			if (CF_Modules<JMObjectSpawnerModule>.Get(objSpawnerModule))
				objSpawnerModule.SpawnCompatibleAttachments(vehicle, null, 0);

			GetCommunityOnlineToolsBase().Log( ident, "Repaired Transport [guid=" + players[i].GetGUID() + "]" );
			SendWebhookColored( "Vehicle", instance, "Repaired " + players[i].FormatSteamWebhook() + " vehicle", JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}
	}

	private void RPC_RepairTransport( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target  )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
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
		}
		else
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position );

			SendWebhookColored( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to " + position.ToString(), JMConstants.WEBHOOK_COLOR_TELEPORT );

			players[i].Update();
		}
	}

	private void RPC_TeleportTo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector position;
		if ( !ctx.Read( position ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
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

		PlayerBase player = PlayerBase.Cast( other.PlayerObject );
		if ( player == NULL )
			return;

		vector position = player.GetPosition();

		if ( Class.CastTo( player, GetPlayerObjectByIdentity( ident ) ) )
		{
			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported to " + position + " [guid=" + other.GetGUID() + "]" );
			SendWebhookColored( "Teleport", instance, "Teleported the admin to " + other.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_TELEPORT );
		}
	}

	private void RPC_TeleportSenderTo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL || !player.HasLastPosition() )
				continue;

			vector position = player.GetLastPosition();

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position + " [previous]" );
			SendWebhookColored( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to their previous position", JMConstants.WEBHOOK_COLOR_TELEPORT );

			players[i].Update();
		}
	}

	private void RPC_TeleportToPrevious( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Previous", senderRPC, instance ) )
			return;

		Exec_TeleportToPrevious( guids, senderRPC, instance );
	}

	//! Client
	void Click_Spectate(UIActionBase action, Object target, string guid = "")
	{
		bool shouldSpectate;

		if (target || guid != "")
		{
			shouldSpectate = true;

			if (CurrentActiveCamera)
			{
				if (CurrentActiveCamera.IsInherited(JMSpectatorCamera))
					shouldSpectate = !target || CurrentActiveCamera.SelectedTarget != target;
				else if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera))
					shouldSpectate = !target || COT_PreviousActiveCamera.SelectedTarget != target;
			}
		}

		action.Disable();

		if (shouldSpectate)
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 1000);

			if (!target)
				StartSpectating(guid);
			else
				StartSpectating(target);

			action.SetButton("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_SPECTATE");
		}
		else
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 3000);

			EndSpectating();

			action.SetButton("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SPECTATE");
		}
	}

	//! @note this allows to start spectating players that are not in netbubble
	void StartSpectating( string guid )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "StartSpectating").Add(guid);
#endif

		if (GetPlayer().GetCommand_Vehicle())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Cannot spectate while in a vehicle. Please leave the vehicle first."));
			return;
		}

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			if (!GetPermissionsManager().HasPermission("Admin.Player.Spectate"))
				return;

			m_SpectatorClient = GetPlayer();

			m_SpectatorClient.COT_TempDisableOnSelectPlayer();
			m_SpectatorClient.COT_RememberVehicle();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( null, JMPlayerModuleRPC.StartSpectating, true, NULL );
		}
	}

	void StartSpectating(Object spectateObject)
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "StartSpectating").Add(spectateObject);
#endif

		if (GetPlayer().GetCommand_Vehicle())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Cannot spectate while in a vehicle. Please leave the vehicle first."));
			return;
		}

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			if (!GetPermissionsManager().HasPermission("Admin.Player.Spectate"))
				return;

			m_SpectatorClient = GetPlayer();

			m_SpectatorClient.COT_TempDisableOnSelectPlayer();
			m_SpectatorClient.COT_RememberVehicle();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( spectateObject, JMPlayerModuleRPC.StartSpectating, true, NULL );
		}
	}

	private void Server_StartSpectating( string guid, PlayerIdentity ident )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_StartSpectating").Add(guid).Add(ident);
#endif

		JMPlayerInstance spectateInstance = GetPermissionsManager().GetPlayer( guid );
		if ( !spectateInstance )
			return;

		PlayerBase spectatePlayer = spectateInstance.PlayerObject;
		if ( !spectatePlayer )
			return;

		Server_StartSpectating(spectatePlayer, ident);
	}

	private void Server_StartSpectating(Object spectateObject, PlayerIdentity ident)
	{
#ifdef JM_COT_DIAG_LOGGING
		Print(spectateObject);
#endif
		PlayerBase playerSpectator = GetPlayerObjectByIdentity( ident );
		if ( !playerSpectator )
			return;

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		if ( playerSpectator == spectateObject )
		{
			COTCreateNotification(ident, new StringLocaliser("You can't spectate yourself"));
			return;
		}

		if (m_Spectators[ident.GetId()] != playerSpectator)
		{
			playerSpectator.COT_RememberVehicle();
			playerSpectator.SetLastPosition();

			m_Spectators[ident.GetId()] = playerSpectator;
		}

		playerSpectator.m_JM_SpectatedObject = spectateObject;
		playerSpectator.m_JM_CameraPosition = vector.Zero;

		playerSpectator.COT_TempDisableOnSelectPlayer();

		g_Game.SelectPlayer( ident, NULL );

		vector transform[4];
		GetCommunityOnlineToolsBase().GetHeadTransform(spectateObject, transform);
		vector position = transform[3];

		g_Game.SelectSpectator( ident, "JMSpectatorCamera", position );

		playerSpectator.COTSetGodMode( true, false );  //! Enable godmode and remember previous state of GetAllowDamage
		playerSpectator.COTUpdateSpectatorPosition();

		Server_OnStartSpectating(spectateObject, ident);

		ScriptRPC rpc = new ScriptRPC();
		int networkLow, networkHigh;
		spectateObject.GetNetworkID(networkLow, networkHigh);
		rpc.Write(networkLow);
		rpc.Write(networkHigh);
		rpc.Send( NULL, JMPlayerModuleRPC.StartSpectating, true, ident );

		PlayerBase spectatePlayer;
		if (Class.CastTo(spectatePlayer, spectateObject) && spectatePlayer.GetAuthenticatedPlayer())
			GetCommunityOnlineToolsBase().Log( ident, "Spectating [guid=" + spectatePlayer.GetAuthenticatedPlayer().GetGUID() + "]" );
		else
			GetCommunityOnlineToolsBase().Log( ident, "Spectating " + spectateObject );
	}

	void Server_OnStartSpectating(Object spectateObject, PlayerIdentity ident)
	{
	}

	private void Client_StartSpectating(Object spectateObject)
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Client_StartSpectating").Add(spectateObject.ToString());
#endif

		CF_Log.Debug("JMPlayerModule::Client_StartSpectating " + spectateObject);
		
		if (COT_PreviousActiveCamera)
		{
			COT_PreviousActiveCamera.SetActive( false );

		#ifdef DIAG_DEVELOPER
			ErrorEx("g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera);

			COT_PreviousActiveCamera = null;
		}

		if ( CurrentActiveCamera )
		{
			CurrentActiveCamera.SelectedTarget( spectateObject );
			CurrentActiveCamera.SetActive( true );
			m_SpectatorCamera = CurrentActiveCamera;
			
#ifdef JM_COT_DIAG_LOGGING
			Print(g_Game.GetPlayer());
#endif
			if ( GetPlayer() )
			{
#ifdef JM_COT_DIAG_LOGGING
				Print("Disabling input controller");
#endif
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}

		Client_OnStartSpectating(spectateObject);
	}

	void Client_OnStartSpectating(Object spectateObject)
	{
	}

	private void RPC_StartSpectating( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_StartSpectating").Add(senderRPC).Add(target.ToString());
#endif

		if ( IsMissionHost() )
		{
			if (!target)
			{
				string guid;
				if (!ctx.Read(guid))
					return;
			}

			JMPlayerInstance instance;
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
				return;

			if (!target)
				Server_StartSpectating(guid, senderRPC);
			else
				Server_StartSpectating(target, senderRPC);
		} else
		{
			int networkLow, networkHigh;
			if ( !ctx.Read( networkLow ) )
				return;
				
			if ( !ctx.Read( networkHigh ) )
				return;

			COT_PreviousActiveCamera = CurrentActiveCamera;

			CF_Log.Debug("Starting spectate, waiting for spectate object");
			Client_Check_StartSpectating(networkLow, networkHigh);
		}
	}

	void Client_Check_StartSpectating(int networkLow, int networkHigh)
	{
		Object spectateObject = g_Game.GetObjectByNetworkId(networkLow, networkHigh);
		if (!Class.CastTo(CurrentActiveCamera, Camera.GetCurrentCamera()) || !spectateObject)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Client_Check_StartSpectating, 34, false, networkLow, networkHigh);
		else if (CurrentActiveCamera.IsInherited(JMSpectatorCamera))
			Client_StartSpectating(spectateObject);
	}

	void UpdateSpectatorPositions()
	{
		if ( !m_Spectators.Count() )
			return;

		foreach ( PlayerBase playerSpectator: m_Spectators )
		{
			if ( playerSpectator && playerSpectator.m_JM_SpectatedObject)
				playerSpectator.COTUpdateSpectatorPosition();
		}
	}

	void EndSpectating()
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "EndSpectating");
#endif

		if (!g_Game.IsMultiplayer())
			return;

		if (!GetPermissionsManager().HasPermission("Admin.Player.Spectate"))
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, NULL );
	}

	private void Server_EndSpectating( PlayerIdentity ident )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_EndSpectating").Add(ident);
#endif

		PlayerBase playerSpectator = m_Spectators[ident.GetId()];
		CF_Log.Debug("JMPlayerModule::Server_EndSpectating - spectator " + playerSpectator);
		if (!playerSpectator)
			return;

		playerSpectator.m_JM_SpectatedObject = null;
		m_Spectators.Remove( ident.GetId() );

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		GetCommunityOnlineToolsBase().Log( ident, "Stopped spectating" );

		bool switchToPreviousCamera = true;
		int waitForPlayerIdleTimeout;
		CF_Log.Debug("JMPlayerModule::Server_EndSpectating - freecam position " + playerSpectator.m_JM_CameraPosition);
		if (playerSpectator.m_JM_CameraPosition == vector.Zero)
		{
			switchToPreviousCamera = false;

			vector spectatorPosition = playerSpectator.GetPosition();
			playerSpectator.COTResetSpectator();

			if ( playerSpectator.HasLastPosition() )
			{
				if (COT_SurfaceIsWater(playerSpectator.GetLastPosition()))
					waitForPlayerIdleTimeout = 250;
				else
					waitForPlayerIdleTimeout = 5000;
			}

			if (!waitForPlayerIdleTimeout)
				g_Game.SelectPlayer(ident, playerSpectator);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(switchToPreviousCamera);
		rpc.Write(waitForPlayerIdleTimeout);
		rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, ident );
	}

	private void Client_EndSpectating(bool switchToPreviousCamera, int waitForPlayerIdleTimeout )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Client_EndSpectating");
#endif
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - switch to prev cam requested? " + switchToPreviousCamera);
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - current cam " + CurrentActiveCamera);
		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - prev cam " + COT_PreviousActiveCamera);
		m_SpectatorCamera.SelectedTarget( NULL );

		if ( CurrentActiveCamera == m_SpectatorCamera )
		{
			CurrentActiveCamera.SetActive( false );

		#ifdef DIAG_DEVELOPER
			ErrorEx("g_Game.ObjectDeleteOnClient(CurrentActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(CurrentActiveCamera);

			CurrentActiveCamera = NULL;

			if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMCinematicCamera) && switchToPreviousCamera)
			{
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - switching to prev cam " + COT_PreviousActiveCamera);
				CurrentActiveCamera = COT_PreviousActiveCamera;
				CurrentActiveCamera.SetActive(true);
				waitForPlayerIdleTimeout = 0;
			}
			else
			{
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - leaving current cam " + CurrentActiveCamera);
				PPEffects.ResetDOFOverride();

				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - player " + m_SpectatorClient);
				CF_Log.Debug("JMPlayerModule::Client_EndSpectating - player is game player? " + (m_SpectatorClient == g_Game.GetPlayer()));
				if ( m_SpectatorClient )
				{
					m_SpectatorClient.GetInputController().SetDisabled( false );
				}
			}
		}
		else if (CurrentActiveCamera)
		{
			waitForPlayerIdleTimeout = 0;
		}

		if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera))
		{
		#ifdef DIAG_DEVELOPER
			ErrorEx("g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(COT_PreviousActiveCamera);
		}

		COT_PreviousActiveCamera = NULL;

		m_SpectatorCamera = NULL;

		if (waitForPlayerIdleTimeout)
		{
			CF_Log.Debug("JMPlayerModule::Client_EndSpectating - waiting for player to be idle");
			m_SpectatorClient.COT_EnableBonePositionUpdate(true);
			Client_Check_EndSpectating(m_SpectatorClient, waitForPlayerIdleTimeout);
		}

		if (waitForPlayerIdleTimeout > 1000)
			COTCreateLocalAdminNotification(new StringLocaliser("Stopping spectating..."));
		else
			COTCreateLocalAdminNotification(new StringLocaliser("Stopped spectating"));

		CF_Log.Debug("JMPlayerModule::Client_EndSpectating - stopped spectating");
	}

	void Client_Check_EndSpectating(PlayerBase playerSpectator, int waitForPlayerIdleTimeout)
	{
		if (!playerSpectator.COT_IsAnimationIdle() && waitForPlayerIdleTimeout > 0)
		{
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Client_Check_EndSpectating, 250, false, playerSpectator, waitForPlayerIdleTimeout - 250 );
		}
		else
		{
			CF_Log.Debug("JMPlayerModule::Client_Check_EndSpectating - player idle");
			playerSpectator.COT_EnableBonePositionUpdate(false);
			COTCreateLocalAdminNotification(new StringLocaliser("Stopped spectating. In case your 3rd person camera or collision is broken, use the “Sit Crossed” emote to fix it."), "set:ccgui_enforce image:HudBuild", 5);

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, JMPlayerModuleRPC.EndSpectating_Finish, true, NULL);
		}
	}

	private void RPC_EndSpectating( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_EndSpectating").Add(senderRPC).Add(target);
#endif
		CF_Log.Debug("JMPlayerModule::RPC_EndSpectating");
		if ( IsMissionHost() )
		{
			JMPlayerInstance instance;
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
				return;

			Server_EndSpectating( senderRPC );
		} else
		{
			bool switchToPreviousCamera;
			if ( !ctx.Read( switchToPreviousCamera ) )
				return;

			int waitForPlayerIdleTimeout;
			if ( !ctx.Read( waitForPlayerIdleTimeout ) )
				return;

			Client_EndSpectating(switchToPreviousCamera, waitForPlayerIdleTimeout);
		}
	}

	private void RPC_EndSpectating_Finish( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_EndSpectating_Finish").Add(senderRPC).Add(target);
#endif
		CF_Log.Debug("JMPlayerModule::RPC_EndSpectating_Finish");
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
			return;

		g_Game.SelectPlayer(senderRPC, senderRPC.GetPlayer());
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

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetGodMode( value );

			ProcessToggle("god mode", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("god mode", value, affectedPlayers, ident);
	}

	void ProcessToggle(string toggle, bool value, JMPlayerInstance player, array<JMPlayerInstance> affectedPlayers, PlayerIdentity ident, JMPlayerInstance instance)
	{
		affectedPlayers.Insert(player);

		GetCommunityOnlineToolsBase().Log( ident, "Set " + toggle + " to " + value + " [guid=" + player.GetGUID() + "]" );

		if ( value )
			SendWebhookColored( "Set", instance, "Gave " + player.FormatSteamWebhook() + " " + toggle, JMConstants.WEBHOOK_COLOR_WARNING );
		else
			SendWebhookColored( "Set", instance, "Removed " + player.FormatSteamWebhook() + " " + toggle, JMConstants.WEBHOOK_COLOR_WARNING );

		player.Update();
	}

	void ShowToggleNotification(string toggle, bool value, array<JMPlayerInstance> players, PlayerIdentity ident)
	{
		//! TODO localization
		string message;
		if ( players.Count() > 0 )
		{
			if ( value )
				message = "Enabled " + toggle;
			else
				message = "Disabled " + toggle;

			if ( players.Count() > 1 )
				message += " for " + players.Count() + " players";
			else if ( ident && ident.GetId() != players[0].GetGUID() )
				message += " for player " + players[0].GetName();
			else
				message += " for yourself";
		}
		else
		{
			message = "Failed to toggle " + toggle + " - no player(s) affected";
		}

		COTCreateNotification( ident, new StringLocaliser( message ) );
	}

	private void RPC_SetGodMode( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.GodMode", senderRPC, instance ) )
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

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetFreeze( value );

			ProcessToggle("freeze", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("freeze", value, affectedPlayers, ident);
	}

	private void RPC_SetFreeze( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Freeze", senderRPC, instance ) )
			return;

		Exec_SetFreeze( value, guids, senderRPC, instance );
	}

	void SetReceiveDamageDealt( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetReceiveDamageDealt( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetReceiveDamageDealt, true, NULL );
		}
	}

	private void Exec_SetReceiveDamageDealt( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetReceiveDamageDealt( value );

			ProcessToggle("receive damage dealt", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("receive damage dealt", value, affectedPlayers, ident);
	}

	private void RPC_SetReceiveDamageDealt( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.ReceiveDamageDealt", senderRPC, instance ) )
			return;

		Exec_SetReceiveDamageDealt( value, guids, senderRPC, instance );
	}

	void InputToggleCannotBeTargetedByAI( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.CannotBeTargetedByAI", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_IGNORED_BY_AI");
			return;
		}

		bool value = !instance.PlayerObject.COTGetCannotBeTargetedByAI();
		array< string > guids = {instance.GetGUID()};

		SetCannotBeTargetedByAI(value, guids);
	}

	void SetCannotBeTargetedByAI( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetCannotBeTargetedByAI( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetCannotBeTargetedByAI, true, NULL );
		}
	}

	private void Exec_SetCannotBeTargetedByAI( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetCannotBeTargetedByAI( value );

			ProcessToggle("cannot be targeted by AI", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("cannot be targeted by AI", value, affectedPlayers, ident);
	}

	private void RPC_SetCannotBeTargetedByAI( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.CannotBeTargetedByAI", senderRPC, instance ) )
			return;

		Exec_SetCannotBeTargetedByAI( value, guids, senderRPC, instance );
	}

	void InputToggleInvisibility( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.Invisibility", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_INVISIBILITY_SELF");
			return;
		}

		bool value = !instance.PlayerObject.COTIsInvisible();
		array< string > guids = {instance.GetGUID()};
		SetInvisible(value, guids);
	}

	void SetInvisible( int value, array< string > guids )
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

	private void Exec_SetInvisible( int value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetInvisibility( value );

			ProcessToggle("invisibility", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("invisibility", value, affectedPlayers, ident);
	}

	private void RPC_SetInvisible( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Invisibility", senderRPC, instance ) )
			return;

		Exec_SetInvisible( value, guids, senderRPC, instance );
	}

	private void RPC_VONStartedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!g_Game.IsServer())
			return;

		CF_Log.Info("%1::RPC_VONStartedTransmitting target %2", ToString(), target.ToString());
		PlayerBase player;
		if (!Class.CastTo(player, target) || !player.COTIsInvisible(JMInvisibilityType.DisableSimulation))
			return;

		player.COTSetInvisibilityOnly(JMInvisibilityType.Interactive);
	}

	private void RPC_VONStoppedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!g_Game.IsServer())
			return;

		CF_Log.Info("%1::RPC_VONStoppedTransmitting target %2", ToString(), target.ToString());
		PlayerBase player;
		if (!Class.CastTo(player, target) || !player.COTIsInvisible(JMInvisibilityType.Interactive))
			return;

		player.COTSetInvisibilityOnly(JMInvisibilityType.DisableSimulation);
	}

	void SetRemoveCollision( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetRemoveCollision( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetRemoveCollision, true, NULL );
		}
	}

	private void Exec_SetRemoveCollision( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetRemoveCollision( value );

			ProcessToggle("remove collision", value, players[i], affectedPlayers, ident, instance);
		}

		ShowToggleNotification("remove collision", value, affectedPlayers, ident);
	}

	private void RPC_SetRemoveCollision( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.RemoveCollision", senderRPC, instance ) )
			return;

		Exec_SetRemoveCollision( value, guids, senderRPC, instance );
	}

	void InputToggleUnlimitedAmmo( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.UnlimitedAmmo", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_AMMO");
			return;
		}

		bool value = !instance.PlayerObject.COTHasUnlimitedAmmo();
		array< string > guids = {instance.GetGUID()};

		SetUnlimitedAmmo(value, guids);
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

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetUnlimitedAmmo( value );

			ProcessToggle("unlimited ammo", value, players[i], affectedPlayers, ident, instance);

			GetCommunityOnlineTools().SetClient( players[i] );
		}

		ShowToggleNotification("unlimited ammo", value, affectedPlayers, ident);
	}

	private void RPC_SetUnlimitedAmmo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.UnlimitedAmmo", senderRPC, instance ) )
			return;

		Exec_SetUnlimitedAmmo( value, guids, senderRPC, instance );
	}

	void InputToggleAdminNV( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.AdminNVG", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_NVG");
			return;
		}

		bool value = !instance.PlayerObject.COTHasAdminNVG();
		array< string > guids = {instance.GetGUID()};

		SetAdminNVG(value, guids);
	}

	void SetAdminNVG( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetAdminNVG( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetAdminNVG, true, NULL );
		}
	}

	private void Exec_SetAdminNVG( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetAdminNVG( value );

			ProcessToggle("admin night vision", value, players[i], affectedPlayers, ident, instance);

			GetCommunityOnlineTools().SetClient( players[i] );
		}

		ShowToggleNotification("admin night vision", value, affectedPlayers, ident);
	}

	private void RPC_SetAdminNVG( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.AdminNVG", senderRPC, instance ) )
			return;

		Exec_SetAdminNVG( value, guids, senderRPC, instance );
	}

	void InputToggleUnlimitedStamina( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.UnlimitedStamina", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_STAMINA");
			return;
		}

		bool value = !instance.PlayerObject.COTHasUnlimitedStamina();
		array< string > guids = {instance.GetGUID()};

		SetUnlimitedStamina(value, guids);
	}

	void SetUnlimitedStamina( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetUnlimitedStamina( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetUnlimitedStamina, true, NULL );
		}
	}

	private void Exec_SetUnlimitedStamina( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetUnlimitedStamina( value );

			ProcessToggle("unlimited stamina", value, players[i], affectedPlayers, ident, instance);

			GetCommunityOnlineTools().SetClient( players[i] );
		}

		ShowToggleNotification("unlimited stamina", value, affectedPlayers, ident);
	}

	private void RPC_SetUnlimitedStamina( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.UnlimitedStamina", senderRPC, instance ) )
			return;

		Exec_SetUnlimitedStamina( value, guids, senderRPC, instance );
	}

	void Vomit(float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Vomit(value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Vomit, true, NULL );
		}
	}

	private void Exec_Vomit(float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;
			
			if(!player.GetCommand_Vehicle())
			{
				SymptomBase vomitSymptom = player.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT);
				if(vomitSymptom != NULL)
					vomitSymptom.SetDuration(value);
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_Vomit( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Vomit", senderRPC, instance ) )
			return;

		Exec_Vomit(value, guids, senderRPC, instance );
	}

	void SetScale( float value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetScale( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetScale, true, NULL );
		}
	}

	private void Exec_SetScale( float value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetScale(value);

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_SetScale( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float value;
		if ( !ctx.Read( value ) )
			return;

		value = Math.Clamp( value, 0.1, 10.0 );

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Scale", senderRPC, instance ) )
			return;

		Exec_SetScale( value, guids, senderRPC, instance );
	}

	void SetBrokenLegs( bool value, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetBrokenLegs( value, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( value );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetBrokenLegs, true, NULL );
		}
	}

	private void Exec_SetBrokenLegs( bool value, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		array<JMPlayerInstance> affectedPlayers = {};

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			if (value)
			{
				if ( player.GetModifiersManager().IsModifierActive( eModifiers.MDF_BROKEN_LEGS ) )//effectively resets the modifier
				{
					player.GetModifiersManager().DeactivateModifier( eModifiers.MDF_BROKEN_LEGS );
				}
				player.GetModifiersManager().ActivateModifier( eModifiers.MDF_BROKEN_LEGS );
			}
			else
			{
				player.SetHealth("RightLeg", "Health", 100);
				player.SetHealth("LeftLeg", "Health", 100);
				player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
			}

			ProcessToggle("broken legs", value, players[i], affectedPlayers, ident, instance);

			GetCommunityOnlineTools().SetClient( players[i] );
		}

		ShowToggleNotification("broken legs", value, affectedPlayers, ident);
	}

	private void RPC_SetBrokenLegs( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.BrokenLegs", senderRPC, instance ) )
			return;

		Exec_SetBrokenLegs( value, guids, senderRPC, instance );
	}

	void InputFreezePlayer( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.Freeze", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_FREEZE_PLAYER");
			return;
		}

		bool value = !instance.PlayerObject.COTIsFrozen();
		array< string > guids = {instance.GetGUID()};

		SetFreeze(value, guids);
	}

	void InputHeal( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.Heal", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_HEAL_SELF");
			return;
		}

		array< string > guids = {instance.GetGUID()};

		Heal(guids);
	}

	void InputToggleGodMode( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission("Admin.Player.GodMode", instance))
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_GODMODE_SELF");
			return;
		}

		bool value = !instance.PlayerObject.COTHasGodMode();
		array< string > guids = {instance.GetGUID()};
		SetGodMode(value, guids);
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

		bool includeAttachments = GetPermissionsManager().HasPermission( "Admin.Player.Heal.Attachments", ident );
		bool includeCargo = GetPermissionsManager().HasPermission( "Admin.Player.Heal.Cargo", ident );
		
		int healedPlayers;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			bool allowDamage = player.GetAllowDamage();
			if (!allowDamage)
				player.SetAllowDamage(true);

			if ( player.GetBleedingManagerServer() )
				player.GetBleedingManagerServer().RemoveAllSources();

			CommunityOnlineToolsBase.HealEntityRecursive(player, includeAttachments, includeCargo);
			player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
			player.COTRemoveAllDiseases();

			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );

			healedPlayers++;

			GetCommunityOnlineToolsBase().Log( ident, "Healed [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Set", instance, "Healed " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SUCCESS );

			if (!allowDamage)
				player.SetAllowDamage(false);

			players[i].Update();
		}

		//! TODO localization
		string message;
		if ( healedPlayers > 0 )
			message = "You healed";
		else
			message = "Failed to heal";

		if ( players.Count() > 1 )
			message += " " + players.Count() + " players";
		else if ( ident && ident.GetId() != guids[0] )
			message += " player " + players[0].GetName();
		else
			message += " yourself";

		COTCreateNotification( ident, new StringLocaliser( message ) );
	}

	private void RPC_Heal( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Heal", senderRPC, instance ) )
			return;

		Exec_Heal( guids, senderRPC, instance );
	}

	void Ban( array< string > guids, string messageText = "", int duration = -1 )
	{
		if ( IsMissionHost() )
		{
			Exec_Ban( guids, NULL, NULL, messageText, duration );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(guids);
			rpc.Write(messageText);
			rpc.Write(duration);
			rpc.Send( NULL, JMPlayerModuleRPC.Ban, true, NULL );
		}
	}

	private void Exec_Ban( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = "", int duration = -1 )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		bool cantBanAdmin;
		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;
			
			if (player.PlayerObject.GetIdentity() == ident)
				continue;
			
			// Because we dont want to ban other staff members
			if ( GetPermissionsManager().HasPermission( "COT", player.PlayerObject.GetIdentity(), player ) )
			{
				cantBanAdmin = true;
				continue;
			}

			SendBanMessage(player.PlayerObject.GetIdentity(), messageText, duration);

			//! Kick and Ban player after delay so client can still receive kickmessage RPC
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Ban_Single, 500, false, player, ident, instance, messageText, duration);
		}

		if (cantBanAdmin)
			COTCreateNotification(ident, new StringLocaliser("You can't ban admins"));

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	private void Exec_Ban_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = "", int duration = -1)
	{
		if (!g_Game || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(g_Game.GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		g_Game.SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Banned [guid=" + player.GetGUID() + "]" );

		SendWebhookColored( "Ban", instance, "Banned " + player.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_DANGER );
	}

	private void RPC_Ban( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		int duration;
		if ( !ctx.Read( duration ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", senderRPC, instance ) )
			return;

		Exec_Ban( guids, senderRPC, instance, messageText, duration );
	}
	
	private void SendBanMessage(PlayerIdentity identity, string messageText, int duration = -1)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.BanMessage, true, identity);
	}

	void Kick( array< string > guids, string messageText )
	{
		if ( IsMissionHost() )
		{
			Exec_Kick( guids, NULL, NULL, messageText );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Write(messageText);
			rpc.Send( NULL, JMPlayerModuleRPC.Kick, true, NULL );
		}
	}

	private void Exec_Kick( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL, string messageText = ""  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		foreach (JMPlayerInstance player: players)
		{
			if (!player.PlayerObject)
				continue;

			SendKickMessage(player.PlayerObject.GetIdentity(), messageText);

			//! Kick player after delay so client can still receive kickmessage RPC
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Kick_Single, 500, false, player, ident, instance);
		}

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	private void Exec_Kick_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL)
	{
		if (!g_Game || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(g_Game.GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		g_Game.SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Kicked [guid=" + player.GetGUID() + "]" );

		SendWebhookColored( "Kick", instance, "Kicked " + player.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
	}

	private void RPC_Kick( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Kick", senderRPC, instance ) )
			return;

		Exec_Kick( guids, senderRPC, instance, messageText );
	}
	
	private void SendKickMessage(PlayerIdentity identity, string messageText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(messageText);
		rpc.Send(NULL, JMPlayerModuleRPC.KickMessage, true, identity);
	}

	//! Because there is no way to check on client if the sender had permissions to send a kick message,
	//! we queue the message for deferred display on the main menu screen.
	//! That way, if the RPC is abused (hacking etc), it won't affect the player during gameplay.
	private void RPC_KickMessage(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		auto trace = CF_Trace_0(this, "RPC_KickMessage");

		if (g_Game.IsDedicatedServer())
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		if (messageText)
			messageText = "#STR_COT_NOTIFICATION_KICKED_BY_ADMIN: " + messageText;
		else
			messageText = "#STR_COT_NOTIFICATION_KICKED_BY_ADMIN";
		
		JMDeferredMessage.QueuedMessages.Clear();
		JMDeferredMessage.Queue("#STR_COT_NOTIFICATION_TITLE_ADMIN", messageText);
	}

	private void RPC_BanMessage(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		auto trace = CF_Trace_0(this, "RPC_BanMessage");

		if (g_Game.IsDedicatedServer())
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		if (messageText)
			messageText = "#STR_COT_NOTIFICATION_BANNED_BY_ADMIN: " + messageText;
		else
			messageText = "#STR_COT_NOTIFICATION_BANNED_BY_ADMIN";
		
		JMDeferredMessage.QueuedMessages.Clear();
		JMDeferredMessage.Queue("#STR_COT_NOTIFICATION_TITLE_ADMIN", messageText);
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COT_RemoveAllItems();

			GetCommunityOnlineToolsBase().Log( ident, "Stripped [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Stripped " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	private void RPC_Strip( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Strip", senderRPC, instance ) )
			return;

		Exec_Strip( guids, senderRPC, instance );
	}

	void ClearCargo( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_ClearCargo( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.ClearCargo, true, NULL );
		}
	}

	private void Exec_ClearCargo( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COT_ClearCargo();

			GetCommunityOnlineToolsBase().Log( ident, "Cleared Cargo [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Cleared Cargo " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_WARNING );

			players[i].Update();
		}
	}

	private void RPC_ClearCargo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.ClearCargo", senderRPC, instance ) )
			return;

		Exec_ClearCargo( guids, senderRPC, instance );
	}
	
	void Dry( array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_Dry( guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.Dry, true, NULL );
		}
	}

	private void Exec_Dry( array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTResetItemWetness();

			GetCommunityOnlineToolsBase().Log( ident, "Dried [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Inventory", instance, "Dried " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_INFO );

			players[i].Update();
		}
	}

	private void RPC_Dry( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Dry", senderRPC, instance ) )
			return;

		Exec_Dry( guids, senderRPC, instance );
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetBleedingManagerServer().RemoveAllSources();

			GetCommunityOnlineToolsBase().Log( ident, "Bleeding stopped [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "Set", instance, "Stopped " + players[i].FormatSteamWebhook() + " bleeding.", JMConstants.WEBHOOK_COLOR_SUCCESS );

			players[i].Update();
		}
	}

	private void RPC_StopBleeding( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.StopBleeding", senderRPC, instance ) )
			return;

		Exec_StopBleeding( guids, senderRPC, instance );
	}

	void SetPermissions( JMPermission permission, array< string > guids )
	{
		auto trace = CF_Trace_0(this, "SetPermissions");

		if ( IsMissionHost() )
		{
			Exec_SetPermissions( permission, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			permission.OnSend(rpc);
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetPermissions, true, NULL );
		}
	}

	private void Exec_SetPermissions( JMPermission permission, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		auto trace = CF_Trace_0(this, "Exec_SetPermissions");

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			players[i].CopyPermissions( permission );
			players[i].Save();

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );

			GetCommunityOnlineToolsBase().Log( ident, "Updated permissions [guid=" + players[i].GetGUID() + "]" );

			SendWebhookColored( "PF", instance, "Updated permissions for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
		}
	}

	private void RPC_SetPermissions( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		auto trace = CF_Trace_1(this, "RPC_SetPermissions").Add(senderRPC.GetId());

		JMPermission permission = new JMPermission( JMConstants.PERM_ROOT );
		permission.CopyPermissions(GetPermissionsManager().RootPermission);
		if ( !permission.OnReceive( ctx ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Permissions", senderRPC, instance ) )
			return;

		Exec_SetPermissions( permission, guids, senderRPC, instance );
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

			SendWebhookColored( "PF", instance, "Updated roles for " + players[i].FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_MODERATION );
		}
	}

	private void RPC_SetRoles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > roles;
		if ( !ctx.Read( roles ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Roles", senderRPC, instance ) )
			return;

		Exec_SetRoles( roles, guids, senderRPC, instance );
	}
}