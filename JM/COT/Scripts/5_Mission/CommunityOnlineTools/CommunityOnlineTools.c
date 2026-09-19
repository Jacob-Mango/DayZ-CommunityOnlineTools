//! Enforce compiles GameLib/Game/World/Mission as SEPARATE modules - a
//! #define in a 3_Game or 4_World file is invisible here. Kept in exact
//! sync with StaticFunctions.c's block; all three must change together.
#define COT_DEBUGLOGS

class CommunityOnlineTools: CommunityOnlineToolsBase
{
	//! Floor on how often one admin's activation toggle may reach the webhook.
	static const int ACTIVE_MIN_INTERVAL_MS = 2000;

	void CommunityOnlineTools()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
		GetPermissionsManager().RegisterPermission("Actions.QuickActions");
	}

	void ~CommunityOnlineTools()
	{
	}
	
	override void OnStart()
	{
		super.OnStart();

		#ifdef JM_COT_AUTOTEST
		JMAutoTest_TryRun();
		#endif
	}

	override void OnFinish()
	{
		super.OnFinish();

		GetPermissionsManager().ResetMission();
	}

	override void OnLoaded()
	{
		#ifdef DIAG
		if (g_Game.IsMultiplayer())
			GetPermissionsManager().CreateFakePlayers();
		#endif

		super.OnLoaded();
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		super.OnRPC( sender, target, rpc_type, ctx );

		if ( rpc_type > JMClientRPC.INVALID && rpc_type < JMClientRPC.COUNT )
		{
			switch ( rpc_type )
			{
			case JMClientRPC.RefreshClients:
				RPC_RefreshClients( ctx, sender, target );
				break;
			case JMClientRPC.RefreshClientPositions:
				RPC_RefreshClientPositions( ctx, sender, target );
				break;
			case JMClientRPC.RemoveClient:
				RPC_RemoveClient( ctx, sender, target );
				break;
			case JMClientRPC.UpdateClient:
				RPC_UpdateClient( ctx, sender, target );
				break;
			case JMClientRPC.UpdateClientPosition:
				RPC_UpdateClientPosition( ctx, sender, target );
				break;
			case JMClientRPC.SetClient:
				RPC_SetClient( ctx, sender, target );
				break;
			case JMClientRPC.UpdateClientBatch:
				RPC_UpdateClientBatch( ctx, sender, target );
				break;
			case JMClientRPC.UpdateClientPositionBatch:
				RPC_UpdateClientPositionBatch( ctx, sender, target );
				break;
			}
		} else if ( rpc_type > JMRoleRPC.INVALID && rpc_type < JMRoleRPC.COUNT )
		{
			switch ( rpc_type )
			{
			case JMRoleRPC.UpdateRole:
				RPC_UpdateRole( ctx, sender, target );
				break;
			}
		} else if ( rpc_type > JMPermissionRPC.INVALID && rpc_type < JMPermissionRPC.COUNT )
		{
			switch ( rpc_type )
			{
			}
		} else if ( rpc_type > JMCOTRPC.INVALID && rpc_type < JMCOTRPC.COUNT )
		{
			switch ( rpc_type )
			{
			case JMCOTRPC.Active:
				RPC_Active( ctx, sender, target );
				break;
			}
		}
	}

	override void OnCOTActiveChanged( bool active )
	{
		if ( active )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_TOGGLE", "STR_COT_GENERIC_ON" ) );
		} else
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_TOGGLE", "STR_COT_GENERIC_OFF" ) );
		}

		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( active );
			rpc.Send( NULL, JMCOTRPC.Active, true, NULL );
		}
	}

	private void RPC_Active( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_Active").Add(senderRPC);
		#endif

		bool active;
		if ( !ctx.Read( active ) )
			return;

		if ( !senderRPC )
			return;

		//! Same permission the sidebar needs to open at all.
		//!
		//! Without it this handler is one webhook post and one log line per
		//! packet, from any player on the server, announcing an admin
		//! activation that never happened - forged audit entries and a flooded
		//! Discord endpoint in the same call.
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermissionRPC( "COT.View", senderRPC, instance ) )
			return;

		if ( !instance )
			return;

		//! Nothing changed, so nothing to announce. A toggle held down, or a
		//! client repeating itself, is not another activation.
		if ( m_ActiveGUIDs.Contains( senderRPC.GetId() ) && m_ActiveGUIDs.Get( senderRPC.GetId() ) == active )
			return;

		//! Even a permitted admin cannot spend the webhook faster than this.
		if ( !JMRPCThrottle.Allow( senderRPC.GetId(), "cot_active", ACTIVE_MIN_INTERVAL_MS ) )
			return;

		auto message = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_WARNING );

		if ( active )
		{
			message.GetEmbed().SetTitle( "Admin Activated COT" );
			message.GetEmbed().SetDescription( instance.FormatSteamWebhook() + " has activated Community Online Tools." );
			GetCommunityOnlineToolsBase().Log( senderRPC, "Activated Community Online Tools [guid=" + instance.GetGUID() + "]" );
		}
		else
		{
			message.GetEmbed().SetTitle( "Admin Deactivated COT" );
			message.GetEmbed().SetDescription( instance.FormatSteamWebhook() + " has de-activated Community Online Tools." );
			GetCommunityOnlineToolsBase().Log( senderRPC, "Deactivated Community Online Tools [guid=" + instance.GetGUID() + "]" );
		}

		m_ActiveGUIDs[senderRPC.GetId()] = active;

		m_Webhook.Post( "AdminActive", message );
	}

	override void RefreshClients()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "RefreshClients");
		#endif

		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMClientRPC.RefreshClients, true, NULL );
		}
	}

	private void RPC_RefreshClients( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_RefreshClients").Add(senderRPC);
		#endif

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermissionRPC( "Admin.Player.Read", senderRPC ) )
			return;

		if ( IsMissionHost() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			//! One array send for the whole roster instead of one unicast RPC
			//! per connected player - this used to fire N separate RPCs every
			//! refresh, and the Players tab refreshes on a 1.5s timer while
			//! it's open (see JMPlayerForm.UpdatePlayerList).
			#ifdef SERVER
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( players.Count() );
			#endif

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].Update();

				#ifdef SERVER
				rpc.Write( players[i].PlayerObject );
				rpc.Write( players[i].GetGUID() );
				players[i].OnSend( rpc, senderRPC.GetId() );
				#endif
			}

			#ifdef SERVER
			rpc.Send( NULL, JMClientRPC.UpdateClientBatch, true, senderRPC );
			#endif
		}
	}

	override void RefreshClientPositions()
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMClientRPC.RefreshClientPositions, true, NULL );
		}
	}

	private void RPC_RefreshClientPositions( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_RefreshClientPositions").Add(senderRPC);
		#endif

		if ( !senderRPC )
			return;

		if ( !GetPermissionsManager().HasPermissionRPC( "Admin.Player.Teleport.Position", senderRPC ) )
			return;

		if ( IsMissionHost() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			//! One array send for the whole roster instead of one unicast RPC
			//! per connected player - same batching as RPC_RefreshClients.
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( players.Count() );

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].Update();

				rpc.Write( players[i].GetGUID() );
				players[i].OnSendPosition( rpc );
			}

			rpc.Send( NULL, JMClientRPC.UpdateClientPositionBatch, true, senderRPC );
		}
	}

	override void RemoveClient( string guid )
	{
		if ( IsMissionHost() )
		{
			m_ActiveGUIDs.Remove(guid);

			//! Their rate limit windows go with them, so a reconnect does not
			//! inherit a cooldown from the session before it.
			JMRPCThrottle.Clear(guid);

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMClientRPC.RemoveClient, true, NULL );
		}
	}

	private void Client_RemoveClient( string guid )
	{
		JMPlayerInstance instance;
		GetPermissionsManager().OnClientDisconnected( guid, instance );

		JM_GetSelected().RemovePlayer( guid );
	}

	private void RPC_RemoveClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_RemoveClient").Add(senderRPC);
		#endif

		if ( IsMissionClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			Client_RemoveClient( guid );
		}
	}

	override void UpdateClient( string guid, PlayerIdentity sendTo )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "UpdateClient").Add(guid);
		#endif

		if ( IsMissionHost() )
		{
			Server_UpdateClient( guid, sendTo );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMClientRPC.UpdateClient, true, sendTo );
		}
	}

	private void Client_UpdateClient( string guid, ParamsReadContext ctx, PlayerBase playerObj )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Client_UpdateClient").Add(guid).Add(playerObj.ToString());
		#endif

		GetPermissionsManager().UpdatePlayer( guid, ctx, playerObj );
	}

	private void Server_UpdateClient( string guid, PlayerIdentity sendTo )
	{
		JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );
		if ( !player )
			return;

		player.Update();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.PlayerObject );
		rpc.Write( player.GetGUID() );
		player.OnSend( rpc, sendTo.GetId() );

		rpc.Send( NULL, JMClientRPC.UpdateClient, true, sendTo );
	}

	private void RPC_UpdateClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdateClient").Add(senderRPC);
		#endif

		if ( g_Game.IsServer() )
		{
			if ( !senderRPC )
				return;

			if ( !GetPermissionsManager().HasPermissionRPC( "Admin.Player.Read", senderRPC ) )
				return;

			string guid;
			if ( !ctx.Read( guid ) )
				return;

			Server_UpdateClient( guid, senderRPC );
		} else if ( g_Game.IsClient() )
		{
			PlayerBase po;
			if ( !ctx.Read( po ) )
				return;

			string pg;
			if ( !ctx.Read( pg ) )
				return;

			Client_UpdateClient( pg, ctx, po );
		}
	}

	private void RPC_UpdateClientPosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdateClientPosition").Add(senderRPC);
		#endif

		if ( g_Game.IsClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;
			
			JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );
			if (!player)
			{
				return;
			}

			player.OnRecievePosition( ctx );
		}
	}

	//! Batched reply to RPC_RefreshClients - one player entry per iteration,
	//! same wire shape RPC_UpdateClient's client branch already reads.
	private void RPC_UpdateClientBatch( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdateClientBatch").Add(senderRPC);
		#endif

		if ( !g_Game.IsClient() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		for ( int i = 0; i < count; i++ )
		{
			PlayerBase po;
			if ( !ctx.Read( po ) )
				return;

			string pg;
			if ( !ctx.Read( pg ) )
				return;

			Client_UpdateClient( pg, ctx, po );
		}
	}

	//! Batched reply to RPC_RefreshClientPositions. A guid this client does
	//! not know yet still has its position bytes read and discarded, so the
	//! stream stays aligned for the remaining entries in the same message.
	private void RPC_UpdateClientPositionBatch( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdateClientPositionBatch").Add(senderRPC);
		#endif

		if ( !g_Game.IsClient() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		for ( int i = 0; i < count; i++ )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );
			if ( player )
			{
				player.OnRecievePosition( ctx );
			} else
			{
				vector discard;
				if ( !ctx.Read( discard ) )
					return;
			}
		}
	}

	override void SetClient( JMPlayerInstance player )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "SetClient").Add(player.GetGUID());
		#endif

		if ( IsMissionOffline() )
		{
			ScriptReadWriteContext rwctx = new ScriptReadWriteContext;
			player.OnSend( rwctx.GetWriteContext() );

			Client_SetClient( player.GetGUID(), rwctx.GetReadContext() );
		} else if ( IsMissionHost() )
		{
			Server_SetClient( player );
		}
	}

	override void SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "SetClient").Add(player.GetGUID());
		#endif

		if ( IsMissionOffline() )
		{
			ScriptReadWriteContext rwctx = new ScriptReadWriteContext;
			player.OnSend( rwctx.GetWriteContext() );

			Client_SetClient( player.GetGUID(), rwctx.GetReadContext() );
		} else if ( IsMissionHost() )
		{
			Server_SetClient( player, identity );
		}
	}

	private void Client_SetClient( string guid, ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Client_SetClient").Add(guid);
		#endif

		GetPermissionsManager().SetClientGUID( guid );

		GetPermissionsManager().UpdatePlayer( guid, ctx, PlayerBase.Cast( g_Game.GetPlayer() ) );

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_SetClient( JMPlayerInstance player )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Server_SetClient").Add(player.GetGUID());
		#endif

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );

		player.OnSend( rpc, player.PlayerObject.GetIdentity().GetId() );

		rpc.Send( NULL, JMClientRPC.SetClient, true, player.PlayerObject.GetIdentity() );
	}

	private void Server_SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );

		player.OnSend( rpc, identity.GetId() );

		rpc.Send( NULL, JMClientRPC.SetClient, true, identity );
	}

	private void RPC_SetClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_SetClient").Add(senderRPC);
		#endif

		if ( IsMissionClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;

			Client_SetClient( guid, ctx );
		}
	}

	override void UpdateRole( JMRole role, PlayerIdentity toSendTo )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "UpdateRole").Add(role.Name).Add(toSendTo.GetId());
		#endif

		if ( IsMissionHost() )
		{
			Server_UpdateRole( role, toSendTo );
		}
	}

	private void Client_UpdateRole( string roleName, ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "Client_UpdateRole").Add(roleName);
		#endif

		JMRole role;
		GetPermissionsManager().LoadRole( roleName, role );
		if ( role )
		{
			role.RootPermission.OnReceive(ctx);
		}

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_UpdateRole( JMRole role, PlayerIdentity toSendTo )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_UpdateRole").Add(role.Name).Add(toSendTo.GetId());
		#endif

		if ( g_Game.IsServer() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( role.Name );
			role.RootPermission.OnSend(rpc);
			rpc.Send( NULL, JMRoleRPC.UpdateRole, true, toSendTo );
		}
	}

	private void RPC_UpdateRole( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "RPC_UpdateRole");
		#endif

		if ( IsMissionClient() )
		{
			string roleName;
			if ( !ctx.Read( roleName ) )
				return;

			Client_UpdateRole( roleName, ctx );
		}
	}

	override void SpawnCompatibleAttachments(EntityAI entity, PlayerBase player, int depth = 3)
	{
		JMObjectSpawnerModule objSpawnerModule;
		if (CF_Modules<JMObjectSpawnerModule>.Get(objSpawnerModule))
			objSpawnerModule.SpawnCompatibleAttachments(entity, player, depth);
	}

	override void SpawnCompatibleAttachmentsWithColor(EntityAI entity, PlayerBase player, int depth = 3, string preferredColor = "")
	{
		JMObjectSpawnerModule objSpawnerModule;
		if (CF_Modules<JMObjectSpawnerModule>.Get(objSpawnerModule))
			objSpawnerModule.SpawnCompatibleAttachmentsWithColor(entity, player, depth, preferredColor);
	}

	override EntityAI RecolorEntityAndAttachments(EntityAI entity, PlayerBase player, string newColor, int depth = 3)
	{
		JMObjectSpawnerModule objSpawnerModule;
		if (CF_Modules<JMObjectSpawnerModule>.Get(objSpawnerModule))
			return objSpawnerModule.RecolorEntityAndAttachments(entity, player, newColor, depth);

		return entity;
	}
}


CommunityOnlineTools GetCommunityOnlineTools()
{
	return CommunityOnlineTools.Cast( g_cotBase );
}