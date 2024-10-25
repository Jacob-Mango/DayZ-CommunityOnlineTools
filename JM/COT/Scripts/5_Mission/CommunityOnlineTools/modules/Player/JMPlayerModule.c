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
		GetPermissionsManager().RegisterPermission( "Admin.Player.Dry" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.StopBleeding" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.BrokenLegs" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.ReceiveDamageDealt" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.Message" );
		GetPermissionsManager().RegisterPermission( "Admin.Player.CannotBeTargetedByAI" );
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
		case JMPlayerModuleRPC.SetHeatBuffer:
			RPC_SetHeatBuffer( ctx, sender, target );
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
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		string NotifText;
		if (!ctx.Read(NotifText))
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Notif", senderRPC, instance ) )
			return;

		Exec_Notif( guids, senderRPC, instance, NotifText );
	}
	
	private void SendNotif(PlayerIdentity identity, string NotifText)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(NotifText);
		rpc.Send(NULL, JMPlayerModuleRPC.Notif, true, identity);
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Health", health );

			GetCommunityOnlineToolsBase().Log( ident, "Set Health To " + health + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " health to " + health );

			players[i].Update();
		}
	}

	private void RPC_SetHealth( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Blood", blood );

			GetCommunityOnlineToolsBase().Log( ident, "Set Blood To " + blood + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " blood to " + blood );

			players[i].Update();
		}
	}

	private void RPC_SetBlood( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetHealth( "GlobalHealth", "Shock", shock );

			GetCommunityOnlineToolsBase().Log( ident, "Set Shock To " + shock + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " shock to " + shock );

			players[i].Update();
		}
	}

	private void RPC_SetShock( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetStatEnergy().Set( energy );

			GetCommunityOnlineToolsBase().Log( ident, "Set Energy To " + energy + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " energy to " + energy );

			players[i].Update();
		}
	}

	private void RPC_SetEnergy( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetStatWater().Set( water );

			GetCommunityOnlineToolsBase().Log( ident, "Set Water To " + water + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " water to " + water );

			players[i].Update();
		}
	}

	private void RPC_SetWater( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetStatStamina().Set( stamina );

			GetCommunityOnlineToolsBase().Log( ident, "Set Stamina To " + stamina + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " stamina to " + stamina );

			players[i].Update();
		}
	}

	private void RPC_SetStamina( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	void SetHeatBuffer( float HeatBuffer, array< string > guids )
	{
		if ( IsMissionHost() )
		{
			Exec_SetHeatBuffer( HeatBuffer, guids, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( HeatBuffer );
			rpc.Write( guids );
			rpc.Send( NULL, JMPlayerModuleRPC.SetHeatBuffer, true, NULL );
		}
	}

	private void Exec_SetHeatBuffer( float HeatBuffer, array< string > guids, PlayerIdentity ident, JMPlayerInstance instance = NULL  )
	{
		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.GetStatHeatBuffer().Set( HeatBuffer );

			GetCommunityOnlineToolsBase().Log( ident, "Set HeatBuffer To " + HeatBuffer + " [guid=" + players[i].GetGUID() + "]" );

			SendWebhook( "Set", instance, "Set " + players[i].FormatSteamWebhook() + " HeatBuffer to " + HeatBuffer );

			players[i].Update();
		}
	}

	private void RPC_SetHeatBuffer( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float HeatBuffer;
		if ( !ctx.Read( HeatBuffer ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.HeatBuffer", senderRPC, instance ) )
			return;

		Exec_SetHeatBuffer( HeatBuffer, guids, senderRPC, instance );
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
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

	private void RPC_SetBloodyHands( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			SendWebhook( "Vehicle", instance, "Repaired " + players[i].FormatSteamWebhook() + " vehicle" );

			players[i].Update();
		}
	}

	private void RPC_RepairTransport( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target  )
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.SetLastPosition();

			player.SetWorldPosition( position );

			GetCommunityOnlineToolsBase().Log( ident, "Teleported [guid=" + players[i].GetGUID() + "] to " + position );

			SendWebhook( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to " + position.ToString() );

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
			SendWebhook( "Teleport", instance, "Teleported the admin to " + other.FormatSteamWebhook() + "" );
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
			SendWebhook( "Teleport", instance, "Teleported " + players[i].FormatSteamWebhook() + " to their previous position" );

			players[i].Update();
		}
	}

	private void RPC_TeleportToPrevious( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "StartSpectating").Add(guid);
#endif

		if (GetPlayer().GetCommand_Vehicle())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Cannot spectate while in a vehicle. Please leave the vehicle first."));
			return;
		}

		GetPlayer().COT_TempDisableOnSelectPlayer();

		GetPlayer().COT_RememberVehicle();

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			m_SpectatorClient = GetPlayer();
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMPlayerModuleRPC.StartSpectating, true, NULL );
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

		PlayerBase spectatePlayer = PlayerBase.Cast( spectateInstance.PlayerObject );
		if ( !spectatePlayer )
			return;

#ifdef JM_COT_DIAG_LOGGING
		Print(spectatePlayer);
#endif

		PlayerBase playerSpectator = GetPlayerObjectByIdentity( ident );
		if ( !playerSpectator )
			return;

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		if ( playerSpectator == spectatePlayer )
		{
			COTCreateNotification(ident, new StringLocaliser("You can't spectate yourself"));
			return;
		}

		playerSpectator.COT_RememberVehicle();

		playerSpectator.SetLastPosition();

		playerSpectator.m_JM_SpectatedPlayer = spectatePlayer;
		playerSpectator.m_JM_CameraPosition = vector.Zero;

		m_Spectators[ident.GetId()] = playerSpectator;

		playerSpectator.COT_TempDisableOnSelectPlayer();

		GetGame().SelectPlayer( ident, NULL );

		vector position = spectatePlayer.GetBonePositionWS( spectatePlayer.GetBoneIndexByName( "Head" ) );
		GetGame().SelectSpectator( ident, "JMSpectatorCamera", position );

		playerSpectator.COTSetGodMode( true, false );  //! Enable godmode and remember previous state of GetAllowDamage
		playerSpectator.COTUpdateSpectatorPosition();

		ScriptRPC rpc = new ScriptRPC();
		int networkLow, networkHigh;
		spectatePlayer.GetNetworkID(networkLow, networkHigh);
		rpc.Write(networkLow);
		rpc.Write(networkHigh);
		rpc.Send( NULL, JMPlayerModuleRPC.StartSpectating, true, ident );

		GetCommunityOnlineToolsBase().Log( ident, "Spectating [guid=" + guid + "]" );
	}

	private void Client_StartSpectating( PlayerBase player )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Client_StartSpectating").Add(player.ToString());
#endif

		Print("Starting spectate, timestamp " + GetGame().GetTickTime());
		
		if (COT_PreviousActiveCamera)
			COT_PreviousActiveCamera.SetActive( false );

		if ( CurrentActiveCamera )
		{
			CurrentActiveCamera.SelectedTarget( player );
			CurrentActiveCamera.SetActive( true );
			m_SpectatorCamera = CurrentActiveCamera;
			
#ifdef JM_COT_DIAG_LOGGING
			Print(GetGame().GetPlayer());
#endif
			if ( GetPlayer() )
			{
#ifdef JM_COT_DIAG_LOGGING
				Print("Disabling input controller");
#endif
				GetPlayer().GetInputController().SetDisabled( true );
			}
		}
	}

	private void RPC_StartSpectating( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_StartSpectating").Add(senderRPC).Add(target.ToString());
#endif

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
			int networkLow, networkHigh;
			if ( !ctx.Read( networkLow ) )
				return;
			if ( !ctx.Read( networkHigh ) )
				return;

			COT_PreviousActiveCamera = CurrentActiveCamera;

			Print("Starting spectate, waiting for player object, timestamp " + GetGame().GetTickTime());
			Client_Check_StartSpectating(networkLow, networkHigh);
		}
	}

	void Client_Check_StartSpectating(int networkLow, int networkHigh)
	{
		PlayerBase player;
		if (!Class.CastTo(CurrentActiveCamera, Camera.GetCurrentCamera()) || !Class.CastTo(player, GetGame().GetObjectByNetworkId(networkLow, networkHigh)))
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Client_Check_StartSpectating, 34, false, networkLow, networkHigh);
		else if (CurrentActiveCamera.IsInherited(JMSpectatorCamera))
			Client_StartSpectating(player);
	}

	void UpdateSpectatorPositions()
	{
		if ( !m_Spectators.Count() )
			return;

		foreach ( PlayerBase playerSpectator: m_Spectators )
		{
			if ( playerSpectator && playerSpectator.m_JM_SpectatedPlayer)
				playerSpectator.COTUpdateSpectatorPosition();
		}
	}

	void EndSpectating()
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "EndSpectating");
#endif

		if ( IsMissionHost() )
		{
			if ( IsMissionOffline() )
			{
				Message( GetPlayer(), "Spectating a player is not possible in offline mode!" );
			}
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMPlayerModuleRPC.EndSpectating, true, NULL );
		}
	}

	private void Server_EndSpectating( PlayerIdentity ident )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_EndSpectating").Add(ident);
#endif

		PlayerBase playerSpectator = m_Spectators[ident.GetId()];
Print("JMPlayerModule::Server_EndSpectating - spectator " + playerSpectator);
		if (!playerSpectator)
			return;

		playerSpectator.m_JM_SpectatedPlayer = null;
		m_Spectators.Remove( ident.GetId() );

#ifdef JM_COT_DIAG_LOGGING
		Print(playerSpectator);
#endif

		GetCommunityOnlineToolsBase().Log( ident, "Stopped spectating" );

		bool switchToPreviousCamera = true;
		int waitForPlayerIdleTimeout;
Print("JMPlayerModule::Server_EndSpectating - freecam position " + playerSpectator.m_JM_CameraPosition);
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
				GetGame().SelectPlayer(ident, playerSpectator);
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
Print("JMPlayerModule::Client_EndSpectating - switch to prev cam requested? " + switchToPreviousCamera);
Print("JMPlayerModule::Client_EndSpectating - current cam " + CurrentActiveCamera);
Print("JMPlayerModule::Client_EndSpectating - prev cam " + COT_PreviousActiveCamera);
		m_SpectatorCamera.SelectedTarget( NULL );

		if ( CurrentActiveCamera == m_SpectatorCamera )
		{
			CurrentActiveCamera.SetActive( false );
			CurrentActiveCamera = NULL;

			if (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMCinematicCamera) && switchToPreviousCamera)
			{
Print("JMPlayerModule::Client_EndSpectating - switching to prev cam " + COT_PreviousActiveCamera);
				CurrentActiveCamera = COT_PreviousActiveCamera;
				CurrentActiveCamera.SetActive(true);
				waitForPlayerIdleTimeout = 0;
			}
			else
			{
Print("JMPlayerModule::Client_EndSpectating - leaving current cam " + CurrentActiveCamera);
				PPEffects.ResetDOFOverride();

Print("JMPlayerModule::Client_EndSpectating - player " + m_SpectatorClient);
Print("JMPlayerModule::Client_EndSpectating - player is game player? " + (m_SpectatorClient == GetGame().GetPlayer()));
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

		COT_PreviousActiveCamera = NULL;

		m_SpectatorCamera = NULL;

		if (waitForPlayerIdleTimeout)
		{
Print("JMPlayerModule::Client_EndSpectating - waiting for player to be idle, timestamp " + GetGame().GetTickTime());
			m_SpectatorClient.COT_EnableBonePositionUpdate(true);
			Client_Check_EndSpectating(m_SpectatorClient, waitForPlayerIdleTimeout);
			if (waitForPlayerIdleTimeout > 1000)
				COTCreateLocalAdminNotification(new StringLocaliser("Stopping spectating..."));
		}
Print("JMPlayerModule::Client_EndSpectating - stopped spectating");
	}

	void Client_Check_EndSpectating(PlayerBase playerSpectator, int waitForPlayerIdleTimeout)
	{
		if (!playerSpectator.COT_IsAnimationIdle() && waitForPlayerIdleTimeout > 0)
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Client_Check_EndSpectating, 250, false, playerSpectator, waitForPlayerIdleTimeout - 250 );
		}
		else
		{
Print("JMPlayerModule::Client_Check_EndSpectating - player idle, timestamp " + GetGame().GetTickTime());
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
Print("JMPlayerModule::RPC_EndSpectating - timestamp " + GetGame().GetTickTime());
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
		Print("JMPlayerModule::RPC_EndSpectating_Finish - timestamp " + GetGame().GetTickTime());
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", senderRPC ) )
			return;

		GetGame().SelectPlayer(senderRPC, senderRPC.GetPlayer());
	}

	void ToggleGodMode()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		bool value = !player.COTHasGodMode();
		array< string > guids = JM_GetSelected().GetPlayersOrSelf();
		if (guids.Count() == 0)
			guids.Insert(player.GetIdentity().GetId());

		SetGodMode(value, guids);
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

		int godModePlayers;

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetGodMode( value );

			godModePlayers++;

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

		//! TODO localization
		string message;
		if ( godModePlayers > 0 )
		{
			if ( value )
				message = "Enabled Godmode";
			else
				message = "Disabled Godmode";
		}
		else
		{
			message = "Failed to toggle godmode";
		}

		if ( players.Count() > 1 )
			message += " for " + players.Count() + " players";
		else if ( ident && ident.GetId() != guids[0] )
			message += " for player " + players[0].GetName();
		else
			message += " for yourself";

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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
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

	private void RPC_SetFreeze( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetReceiveDamageDealt( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set Receive Damage Dealt To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " receive damage dealt" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " receive damage dealt" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetReceiveDamageDealt( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.ReceiveDamageDealt", senderRPC, instance ) )
			return;

		Exec_SetReceiveDamageDealt( value, guids, senderRPC, instance );
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetCannotBeTargetedByAI( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set cannot be targeted by AI to " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " cannot be targeted by AI" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " cannot be targeted by AI" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetCannotBeTargetedByAI( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.CannotBeTargetedByAI", senderRPC, instance ) )
			return;

		Exec_SetCannotBeTargetedByAI( value, guids, senderRPC, instance );
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
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

	private void RPC_SetInvisible( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Invisibility", senderRPC, instance ) )
			return;

		Exec_SetInvisible( value, guids, senderRPC, instance );
	}

	private void RPC_VONStartedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!GetGame().IsServer())
			return;

		CF_Log.Info("%1::RPC_VONStartedTransmitting target %2", ToString(), target.ToString());
		PlayerBase player;
		if (!Class.CastTo(player, target) || !player.COTIsInvisible(JMInvisibilityType.DisableSimulation))
			return;

		player.COTSetInvisibilityOnly(JMInvisibilityType.Interactive);
	}

	private void RPC_VONStoppedTransmitting( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if (!GetGame().IsServer())
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetRemoveCollision( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set remove collision to " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " remove collision" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " remove collision" );
			}

			players[i].Update();
		}
	}

	private void RPC_SetRemoveCollision( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.RemoveCollision", senderRPC, instance ) )
			return;

		Exec_SetRemoveCollision( value, guids, senderRPC, instance );
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
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
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

	private void RPC_SetUnlimitedAmmo( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.UnlimitedAmmo", senderRPC, instance ) )
			return;

		Exec_SetUnlimitedAmmo( value, guids, senderRPC, instance );
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetAdminNVG( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set AdminNVG To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " admin NVG" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " admin NVG" );
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_SetAdminNVG( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.AdminNVG", senderRPC, instance ) )
			return;

		Exec_SetAdminNVG( value, guids, senderRPC, instance );
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

		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = PlayerBase.Cast( players[i].PlayerObject );
			if ( player == NULL )
				continue;

			player.COTSetUnlimitedStamina( value );

			GetCommunityOnlineToolsBase().Log( ident, "Set UnlimitedStamina To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " unlimited stamina" );
			} else
			{
				SendWebhook( "Set", instance, "Removed " + players[i].FormatSteamWebhook() + " unlimited stamina" );
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_SetUnlimitedStamina( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
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

		array< string > guids;
		if ( !ctx.Read( guids ) )
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

			GetCommunityOnlineToolsBase().Log( ident, "Set Broken Legs To " + value + " [guid=" + players[i].GetGUID() + "]" );

			if ( value )
			{
				SendWebhook( "Set", instance, "Gave " + players[i].FormatSteamWebhook() + " broken legs" );
			} else
			{
				SendWebhook( "Set", instance, "Fixed " + players[i].FormatSteamWebhook() + " broken legs" );
			}

			players[i].Update();

			GetCommunityOnlineTools().SetClient( players[i] );
		}
	}

	private void RPC_SetBrokenLegs( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool value;
		if ( !ctx.Read( value ) )
			return;

		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.BrokenLegs", senderRPC, instance ) )
			return;

		Exec_SetBrokenLegs( value, guids, senderRPC, instance );
	}

	void InputHeal( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( GetCommunityOnlineToolsBase().IsActive() )
		{
			array< string > guids = JM_GetSelected().GetPlayersOrSelf();
			if (guids.Count() == 0)
				guids.Insert(GetGame().GetPlayer().GetIdentity().GetId());

			Heal(guids);
		}
	}

	void InputToggleGodMode( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( GetCommunityOnlineToolsBase().IsActive() )
			ToggleGodMode();
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

			SendWebhook( "Set", instance, "Healed " + players[i].FormatSteamWebhook() );

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
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Ban_Single, 500, false, player, ident, instance);
		}

		if (cantBanAdmin)
			COTCreateNotification(ident, new StringLocaliser("You cant ban admins"));

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	private void Exec_Ban_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL)
	{
		if (!GetGame() || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(GetGame().GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		GetGame().SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Banned [guid=" + player.GetGUID() + "]" );

		SendWebhook( "Ban", instance, "Banned " + player.FormatSteamWebhook() );
	}

	private void RPC_Ban( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		string messageText;
		if (!ctx.Read(messageText))
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", senderRPC, instance ) )
			return;

		Exec_Ban( guids, senderRPC, instance, messageText );
	}
	
	private void SendBanMessage(PlayerIdentity identity, string messageText, int duration = -1)
	{
		if (duration > 0)
		{
			CF_Date nowUTC = CF_Date.Now(true);
			duration = nowUTC.GetTimestamp() + duration;
		}

		JMPlayerBan banData = new JMPlayerBan;
		banData.Message = messageText;
		banData.BanDuration = duration;
		banData.Save(banData, identity.GetId());

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
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_Kick_Single, 500, false, player, ident, instance);
		}

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SyncEvents.SendPlayerList, 1500);
	}

	private void Exec_Kick_Single(JMPlayerInstance player, PlayerIdentity ident, JMPlayerInstance instance = NULL)
	{
		if (!GetGame() || !player.PlayerObject)
			return;

		auto missionServer = MissionServer.Cast(GetGame().GetMission());

		if (!missionServer)
			return;

		player.PlayerObject.COTSetIsBeingKicked(true);

		GetGame().SendLogoutTime(player.PlayerObject, 0);

		missionServer.PlayerDisconnected(player.PlayerObject, player.PlayerObject.GetIdentity(), player.PlayerObject.GetIdentity().GetId());

		GetCommunityOnlineToolsBase().Log( ident, "Kicked [guid=" + player.GetGUID() + "]" );

		SendWebhook( "Kick", instance, "Kicked " + player.FormatSteamWebhook() );
	}

	private void RPC_Kick( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
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

		if (GetGame().IsDedicatedServer())
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

		if (GetGame().IsDedicatedServer())
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

			SendWebhook( "Inventory", instance, "Stripped " + players[i].FormatSteamWebhook() );

			players[i].Update();
		}
	}

	private void RPC_Strip( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Strip", senderRPC, instance ) )
			return;

		Exec_Strip( guids, senderRPC, instance );
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

			SendWebhook( "Inventory", instance, "Dried " + players[i].FormatSteamWebhook() );

			players[i].Update();
		}
	}

	private void RPC_Dry( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
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

			SendWebhook( "Set", instance, "Stopped " + players[i].FormatSteamWebhook() + " bleeding." );

			players[i].Update();
		}
	}

	private void RPC_StopBleeding( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		array< string > guids;
		if ( !ctx.Read( guids ) )
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

			SendWebhook( "PF", instance, "Updated permissions for " + players[i].FormatSteamWebhook() );
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

			SendWebhook( "PF", instance, "Updated roles for " + players[i].FormatSteamWebhook() );
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

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Roles", senderRPC, instance ) )
			return;

		Exec_SetRoles( roles, guids, senderRPC, instance );
	}
};

