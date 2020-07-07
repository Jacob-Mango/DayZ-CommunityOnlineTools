class CommunityOnlineTools: CommunityOnlineToolsBase
{
	void CommunityOnlineTools()
	{
		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
	}

	void ~CommunityOnlineTools()
	{
	}
	
	override void OnStart()
	{
		super.OnStart();
	}

	override void OnFinish()
	{
		super.OnFinish();

		GetPermissionsManager().ResetMission();
	}

	override void OnLoaded()
	{
		if ( IsMissionOffline() )
		{
			// GetPermissionsManager().CreateFakePlayers();
		}

		super.OnLoaded();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
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
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLE", "STR_COT_ON" ) );
		} else
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLE", "STR_COT_OFF" ) );
		}

		if ( GetGame().IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( active );
			rpc.Send( NULL, JMCOTRPC.Active, true, NULL );
		}
	}

	private void RPC_Active( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		bool active;
		if ( !ctx.Read( active ) )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( senderRPC.GetId() );

        auto message = m_Webhook.CreateDiscordMessage();
        
		if ( active )
        	message.GetEmbed().AddField( "Admin Activity", "" + instance.FormatSteamWebhook() + " has activated Community Online Tools" );
		else
        	message.GetEmbed().AddField( "Admin Activity", "" + instance.FormatSteamWebhook() + " has de-activated Community Online Tools" );

        m_Webhook.Post( "AdminActive", message );
	}

	override void RefreshClients()
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMClientRPC.RefreshClients, true, NULL );
		}
	}

	private void RPC_RefreshClients( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Read", senderRPC ) )
			return;

		ScriptRPC rpc = new ScriptRPC();

		if ( IsMissionHost() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].Update();
				
				rpc.Write( players[i].PlayerObject );
				rpc.Write( players[i].GetGUID() );
				players[i].OnSend( rpc );

				rpc.Send( NULL, JMClientRPC.UpdateClient, true, senderRPC );

				rpc.Reset();
			}
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

	private void RPC_RefreshClientPositions( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Position", senderRPC ) )
			return;

		ScriptRPC rpc = new ScriptRPC();

		if ( IsMissionHost() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].Update();

				rpc.Write( players[i].GetGUID() );
				players[i].OnSendPosition( rpc );

				rpc.Send( NULL, JMClientRPC.UpdateClientPosition, true, senderRPC );

				rpc.Reset();
			}
		}
	}

	override void RemoveClient( string guid )
	{
		if ( IsMissionHost() )
		{
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

	private void RPC_RemoveClient( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
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

	private void Client_UpdateClient( string guid, ref ParamsReadContext ctx, PlayerBase playerObj )
	{
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
		player.OnSend( rpc );

		rpc.Send( NULL, JMClientRPC.UpdateClient, true, sendTo );
	}

	private void RPC_UpdateClient( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetGame().IsServer() )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Read", senderRPC ) )
				return;

			string guid;
			if ( !ctx.Read( guid ) )
				return;

			Server_UpdateClient( guid, senderRPC );
		} else if ( GetGame().IsClient() )
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

	private void RPC_UpdateClientPosition( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetGame().IsClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
				return;
			
			JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );

			player.OnRecievePosition( ctx );
		}
	}

	override void SetClient( JMPlayerInstance player )
	{
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

	private void Client_SetClient( string guid, ref ParamsReadContext ctx )
	{
		GetPermissionsManager().SetClientGUID( guid );

		GetPermissionsManager().UpdatePlayer( guid, ctx, PlayerBase.Cast( GetGame().GetPlayer() ) );

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_SetClient( JMPlayerInstance player )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );

		player.OnSend( rpc );

		rpc.Send( NULL, JMClientRPC.SetClient, true, player.PlayerObject.GetIdentity() );
	}

	private void Server_SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );

		player.OnSend( rpc );

		rpc.Send( NULL, JMClientRPC.SetClient, true, identity );
	}

	private void RPC_SetClient( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
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
		if ( IsMissionHost() )
		{
			Server_UpdateRole( role, toSendTo );
		}
	}

	private void Client_UpdateRole( string roleName, array< string > perms )
	{
		JMRole role;
		GetPermissionsManager().LoadRole( roleName, role );
		if ( role )
		{
			role.SerializedData.Clear();
			role.SerializedData.Copy( perms );
			role.Deserialize();
		}

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_UpdateRole( JMRole role, PlayerIdentity toSendTo )
	{
		if ( GetGame().IsServer() )
		{
			role.Serialize();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( role.Name );
			rpc.Write( role.SerializedData );
			rpc.Send( NULL, JMRoleRPC.UpdateRole, true, toSendTo );
		}
	}

	private void RPC_UpdateRole( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionClient() )
		{
			string roleName;
			if ( !ctx.Read( roleName ) )
				return;

			array< string > permissions;
			if ( !ctx.Read( permissions ) )
				return;

			Client_UpdateRole( roleName, permissions );
		}
	}
}

CommunityOnlineTools GetCommunityOnlineTools()
{
	return CommunityOnlineTools.Cast( g_cotBase );
}