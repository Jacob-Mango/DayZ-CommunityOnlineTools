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
		if ( IsMissionClient() )
		{
			Client_RefreshClients();
		}

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
			case JMClientRPC.RemoveClient:
				RPC_RemoveClient( ctx, sender, target );
				break;
			case JMClientRPC.UpdateClient:
				RPC_UpdateClient( ctx, sender, target );
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
		}
	}

	override void RefreshClients()
	{
		if ( IsMissionClient() )
		{
			Client_RefreshClients();
		}
	}

	private void Client_RefreshClients()
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMClientRPC.RefreshClients, true, NULL );
	}

	private void Server_RefreshClients()
	{

	}

	private void RPC_RefreshClients( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		ScriptRPC rpc = new ScriptRPC();

		if ( IsMissionHost() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].UpdatePlayerData();
				players[i].Serialize();

				rpc.Write( players[i].Data );
				rpc.Write( players[i].PlayerObject );

				rpc.Send( NULL, JMClientRPC.UpdateClient, true, senderRPC );

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

		RemoveSelectedPlayer( guid );
	}

	private void Server_RemoveClient( string guid )
	{
	}

	private void RPC_RemoveClient( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
			{
				return;
			} 

			Client_RemoveClient( guid );
		}
	}

	override void UpdateClient( string guid, PlayerIdentity sendTo )
	{
		if ( IsMissionHost() )
		{
			Server_UpdateClient( guid, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMClientRPC.UpdateClient, true, sendTo );
		}
	}

	private void Client_UpdateClient( JMPlayerInformation playerInfo, PlayerBase playerObj )
	{
		GetPermissionsManager().UpdatePlayer( playerInfo, playerObj );
	}

	private void Server_UpdateClient( string guid, PlayerIdentity sendTo )
	{
		JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );
		if ( !player )
			return;

		player.UpdatePlayerData();
		player.Serialize();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.Data );
		rpc.Write( player.PlayerObject );
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
			{
				return;
			}

			Server_UpdateClient( guid, senderRPC );
		}

		if ( GetGame().IsClient() )
		{
			JMPlayerInformation cd;
			if ( !ctx.Read( cd ) )
			{
				return;
			}

			PlayerBase po;
			if ( !ctx.Read( po ) )
			{
				return;
			}

			Client_UpdateClient( cd, po );
		}
	}

	override void SetClient( JMPlayerInstance player )
	{
		if ( IsMissionOffline() )
		{
			Client_SetClient( player.GetGUID(), player.Data );
		} else if ( IsMissionHost() )
		{
			Server_SetClient( player );
		}
	}

	override void SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
		if ( IsMissionOffline() )
		{
			Client_SetClient( player.GetGUID(), player.Data );
		} else if ( IsMissionHost() )
		{
			Server_SetClient( player, identity );
		}
	}

	private void Client_SetClient( string guid, JMPlayerInformation playerInfo )
	{
		GetPermissionsManager().UpdatePlayer( playerInfo, PlayerBase.Cast( GetGame().GetPlayer() ) );

		GetPermissionsManager().SetClientGUID( guid );

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_SetClient( JMPlayerInstance player )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );
		rpc.Write( player.Data );
		rpc.Send( NULL, JMClientRPC.SetClient, true, player.GetPlayerIdentity() );
	}

	private void Server_SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( player.GetGUID() );
		rpc.Write( player.Data );
		rpc.Send( NULL, JMClientRPC.SetClient, true, identity );
	}

	private void RPC_SetClient( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionClient() )
		{
			string guid;
			if ( !ctx.Read( guid ) )
			{
				return;
			}

			JMPlayerInformation cd;
			if ( !ctx.Read( cd ) )
			{
				return;
			}

			Client_SetClient( guid, cd );
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
			{
				return;
			}

			array< string > permissions;
			if ( !ctx.Read( permissions ) )
			{
				return;
			}

			Client_UpdateRole( roleName, permissions );
		}
	}
}

CommunityOnlineTools GetCommunityOnlineTools()
{
	return CommunityOnlineTools.Cast( g_cotBase );
}