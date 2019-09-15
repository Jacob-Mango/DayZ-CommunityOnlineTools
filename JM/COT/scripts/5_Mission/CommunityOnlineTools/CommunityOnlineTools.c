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
	}

	override void OnLoaded()
	{
		if ( IsMissionClient() )
		{
			Client_RefreshClients();
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

		if ( rpc_type > JMClientRPC.INVALID && rpc_type < JMClientRPC.MAX )
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
			return;
		}

		if ( rpc_type > JMRoleRPC.INVALID && rpc_type < JMRoleRPC.MAX )
		{
			switch ( rpc_type )
			{
			case JMRoleRPC.UpdateRole:
				RPC_UpdateRole( ctx, sender, target );
				break;
			}
			return;
		}
    }

	void RefreshClients()
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

	private void RPC_RefreshClients( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
				rpc.Write( players[i].IdentityPlayer );
				rpc.Write( players[i].PlayerObject );

				rpc.Send( NULL, JMClientRPC.UpdateClient, true, senderRPC );

				rpc.Reset();
			}
		}
	}

	void RemoveClient( string guid )
	{
		if ( IsMissionHost() )
		{
			Server_RemoveClient( guid );
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
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( guid );
		rpc.Send( NULL, JMClientRPC.RefreshClients, true, NULL );
	}

	private void RPC_RemoveClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	void UpdateClient( string guid )
	{
		if ( IsMissionHost() )
		{
			Server_UpdateClient( guid, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Send( NULL, JMClientRPC.UpdateClient, true, NULL );
		}
	}

	private void Client_UpdateClient( JMPlayerInformation playerInfo, PlayerIdentity playerIdent, PlayerBase playerObj )
	{
		GetPermissionsManager().UpdatePlayer( playerInfo, playerIdent, playerObj );
	}

	private void Server_UpdateClient( string guid, PlayerIdentity sendTo )
	{
		JMPlayerInstance player = GetPermissionsManager().GetPlayer( guid );
		if ( !player )
			return;

		player.UpdatePlayerData();
		player.Serialize();

		if ( IsMissionClient() )
		{
			Client_UpdateClient( player.Data, player.IdentityPlayer, player.PlayerObject );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( player.Data );
			rpc.Write( player.IdentityPlayer );
			rpc.Write( player.PlayerObject );
			rpc.Send( NULL, JMClientRPC.UpdateClient, true, sendTo );
		}
	}

	private void RPC_UpdateClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			PlayerIdentity pi;
			if ( !ctx.Read( pi ) )
			{
				return;
			}

			PlayerBase po;
			if ( !ctx.Read( po ) )
			{
				return;
			}

			Client_UpdateClient( cd, pi, po );
		}
	}

	void SetClient( string guid, JMPlayerInformation cd, PlayerIdentity pi, PlayerBase po )
	{
		if ( IsMissionHost() )
		{
			Server_SetClient( guid, cd, pi, po );
		}
	}

	private void Client_SetClient( string guid, JMPlayerInformation playerInfo, PlayerIdentity playerIdent, PlayerBase playerObj )
	{
		GetPermissionsManager().UpdatePlayer( playerInfo, playerIdent, playerObj );

		GetPermissionsManager().SetClientGUID( guid );

		GetModuleManager().OnClientPermissionsUpdated();
	}

	private void Server_SetClient( string guid, JMPlayerInformation pInfo, PlayerIdentity pIdent, PlayerBase pObj )
	{
		if ( IsMissionClient() )
		{
			Client_SetClient( guid, pInfo, pIdent, pObj );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( guid );
			rpc.Write( pInfo );
			rpc.Write( pIdent );
			rpc.Write( pObj );
			rpc.Send( NULL, JMClientRPC.SetClient, true, pIdent );
		}
	}

	private void RPC_SetClient( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			PlayerIdentity pi;
			if ( !ctx.Read( pi ) )
			{
				return;
			}

			PlayerBase po;
			if ( !ctx.Read( po ) )
			{
				return;
			}

			Client_SetClient( guid, cd, pi, po );
		}
	}

	void UpdateRole( JMRole role, PlayerIdentity toSendTo = NULL )
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

	private void RPC_UpdateRole( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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