class JMCompensationsModule: JMRenderableModuleBase
{
	private ref array< ref JMCompensationEntry > m_CachedCompensations;

	void JMCompensationsModule()
	{
		GetPermissionsManager().RegisterPermission( "Compensations.View" );
		GetPermissionsManager().RegisterPermission( "Compensations.Spawn" );
		GetPermissionsManager().RegisterPermission( "Compensations.Delete" );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Compensations.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleCompensations";
	}

	override string GetLayoutRoot()
	{
		return "JM\\COT\\GUI\\layouts\\compensations_form.layout";
	}

	override string GetCategory()
	{
		return "Items";
	}

	override string GetTitle()
	{
		return "Compensations";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "hand-coins" );
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
		return "Compensations Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Spawn" );
		types.Insert( "Delete" );
	}

	override int GetRPCMin()
	{
		return JMCompensationsModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMCompensationsModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMCompensationsModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMCompensationsModuleRPC.SpawnCursor:
			RPC_SpawnCursor( ctx, sender, target );
			break;
		case JMCompensationsModuleRPC.SpawnTarget:
			RPC_SpawnTarget( ctx, sender, target );
			break;
		case JMCompensationsModuleRPC.SpawnPlayers:
			RPC_SpawnPlayers( ctx, sender, target );
			break;
		case JMCompensationsModuleRPC.Delete:
			RPC_Delete( ctx, sender, target );
			break;
		}
	}

	array< ref JMCompensationEntry > GetCompensations()
	{
		// On client, return cached data from server
		if ( IsMissionClient() && m_CachedCompensations )
			return m_CachedCompensations;

		// On server, read from filesystem
		array< ref JMCompensationEntry > compensations = new array< ref JMCompensationEntry >;

		if ( !FileExist( JMConstants.DIR_COMPENSATIONS ) )
			return compensations;

		// Enumerate player directories
		string playerDir;
		FileAttr playerAttr;
		FindFileHandle playerHandle = FindFile( JMConstants.DIR_COMPENSATIONS + "*", playerDir, playerAttr, FindFileFlags.DIRECTORIES );
		if ( playerHandle )
		{
			while ( playerDir != "" )
			{
				if ( playerAttr & FileAttr.DIRECTORY && playerDir != "." && playerDir != ".." )
				{
					string steamID = playerDir;
					string fullPlayerDir = JMConstants.DIR_COMPENSATIONS + steamID + "\\";

					// Enumerate loadout files in player directory
					string fileName;
					FileAttr fileAttr;
					FindFileHandle fileHandle = FindFile( fullPlayerDir + "*" + JMConstants.EXT_LOADOUT, fileName, fileAttr, 0 );
					if ( fileHandle )
					{
						while ( fileName != "" )
						{
							if ( !( fileAttr & FileAttr.DIRECTORY ) )
							{
								string timestamp = fileName.Substring( 0, fileName.Length() - JMConstants.EXT_LOADOUT.Length() );
								compensations.Insert( new JMCompensationEntry( steamID, timestamp ) );
							}
							fileName = "";
							FindNextFile( fileHandle, fileName, fileAttr );
						}
						CloseFindFile( fileHandle );
					}
				}
				playerDir = "";
				FindNextFile( playerHandle, playerDir, playerAttr );
			}
			CloseFindFile( playerHandle );
		}

		return compensations;
	}

	void Load()
	{
		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMCompensationsModuleRPC.Load, true, NULL );
		}
		else
		{
			Server_Load( NULL );
		}
	}

	private void Server_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( ident && !GetPermissionsManager().HasPermission( "Compensations.View", ident, instance ) )
			return;

		array< ref JMCompensationEntry > compensations = GetCompensations();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( compensations );
		rpc.Send( NULL, JMCompensationsModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Server_Load( senderRPC );
		}
		else
		{
			array< ref JMCompensationEntry > compensations;
			if ( ctx.Read( compensations ) )
			{
				m_CachedCompensations = compensations;
				OnSettingsUpdated();
			}
		}
	}

	void SpawnCursor( string steamID, string timestamp, vector position )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( steamID );
			rpc.Write( timestamp );
			rpc.Write( position );
			rpc.Send( NULL, JMCompensationsModuleRPC.SpawnCursor, true, NULL );
		}
		else
		{
			Server_SpawnCursor( steamID, timestamp, position, NULL );
		}
	}

	private void Server_SpawnCursor( string steamID, string timestamp, vector position, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Compensations.Spawn", ident, instance ) )
			return;

		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return;

		SpawnLoadout( loadoutModule, loadout, position );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned compensation for " + steamID + " (" + timestamp + ") at " + position );
		SendWebhookColored( "Spawn", instance, "Spawned compensation for **" + steamID + "** (" + timestamp + ") at " + position.ToString(), JMConstants.WEBHOOK_COLOR_SPAWN );
	}

	private void RPC_SpawnCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string steamID;
			if ( !ctx.Read( steamID ) )
				return;

			string timestamp;
			if ( !ctx.Read( timestamp ) )
				return;

			vector position;
			if ( !ctx.Read( position ) )
				return;

			Server_SpawnCursor( steamID, timestamp, position, senderRPC );
		}
	}

	void SpawnTarget( string steamID, string timestamp, EntityAI ent )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( steamID );
			rpc.Write( timestamp );
			rpc.Write( ent );
			rpc.Send( NULL, JMCompensationsModuleRPC.SpawnTarget, true, NULL );
		}
		else
		{
			Server_SpawnTarget( steamID, timestamp, ent, NULL );
		}
	}

	private void Server_SpawnTarget( string steamID, string timestamp, EntityAI ent, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Compensations.Spawn", ident, instance ) )
			return;

		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return;

		SpawnLoadout( loadoutModule, loadout, ent );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned compensation for " + steamID + " (" + timestamp + ") on " + ent.GetType() );
		SendWebhookColored( "Spawn", instance, "Spawned compensation for **" + steamID + "** (" + timestamp + ") on " + ent.GetType(), JMConstants.WEBHOOK_COLOR_SPAWN );
	}

	private void RPC_SpawnTarget( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string steamID;
			if ( !ctx.Read( steamID ) )
				return;

			string timestamp;
			if ( !ctx.Read( timestamp ) )
				return;

			EntityAI ent;
			if ( !ctx.Read( ent ) )
				return;

			Server_SpawnTarget( steamID, timestamp, ent, senderRPC );
		}
	}

	void Delete( string steamID, string timestamp )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( steamID );
			rpc.Write( timestamp );
			rpc.Send( NULL, JMCompensationsModuleRPC.Delete, true, NULL );
		}
		else
		{
			Server_Delete( steamID, timestamp, NULL );
		}
	}

	private void Server_Delete( string steamID, string timestamp, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Compensations.Delete", ident, instance ) )
			return;

		string filepath = JMConstants.DIR_COMPENSATIONS + steamID + "\\" + timestamp + JMConstants.EXT_LOADOUT;
		if ( DeleteFile( filepath ) )
		{
			GetCommunityOnlineToolsBase().Log( ident, "Deleted compensation for " + steamID + " (" + timestamp + ")" );
			SendWebhookColored( "Delete", instance, "Deleted compensation for **" + steamID + "** (" + timestamp + ")", JMConstants.WEBHOOK_COLOR_DANGER );
		}
	}

	private void RPC_Delete( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string steamID;
			if ( !ctx.Read( steamID ) )
				return;

			string timestamp;
			if ( !ctx.Read( timestamp ) )
				return;

			Server_Delete( steamID, timestamp, senderRPC );
		}
	}

	void SpawnPlayers( string steamID, string timestamp, array< string > guids )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( steamID );
			rpc.Write( timestamp );
			rpc.Write( guids );
			rpc.Send( NULL, JMCompensationsModuleRPC.SpawnPlayers, true, NULL );
		}
		else
		{
			Server_SpawnPlayers( steamID, timestamp, guids, NULL );
		}
	}

	private void Server_SpawnPlayers( string steamID, string timestamp, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Compensations.Spawn", ident, instance ) )
			return;

		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return;

		array< Man > players = new array< Man >;
		g_Game.GetWorld().GetPlayerList( players );

		foreach ( string guid: guids )
		{
			foreach ( Man man: players )
			{
				PlayerBase player;
				if ( Class.CastTo( player, man ) )
				{
					PlayerIdentity playerIdent = player.GetIdentity();
					if ( playerIdent && playerIdent.GetId() == guid )
					{
						SpawnLoadout( loadoutModule, loadout, player );
						break;
					}
				}
			}
		}

		GetCommunityOnlineToolsBase().Log( ident, "Spawned compensation for " + steamID + " (" + timestamp + ") on " + guids.Count() + " player(s)" );
		SendWebhookColored( "Spawn", instance, "Spawned compensation for **" + steamID + "** (" + timestamp + ") on " + guids.Count() + " player(s)", JMConstants.WEBHOOK_COLOR_SPAWN );
	}

	private void RPC_SpawnPlayers( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string steamID;
			if ( !ctx.Read( steamID ) )
				return;

			string timestamp;
			if ( !ctx.Read( timestamp ) )
				return;

			array< string > guids;
			if ( !ctx.Read( guids ) )
				return;

			Server_SpawnPlayers( steamID, timestamp, guids, senderRPC );
		}
	}

	private JMLoadout LoadCompensation( string steamID, string timestamp )
	{
		string filepath = JMConstants.DIR_COMPENSATIONS + steamID + "\\" + timestamp + JMConstants.EXT_LOADOUT;
		if ( !FileExist( filepath ) )
			return NULL;

		JMLoadout loadout = new JMLoadout();
		JsonFileLoader<JMLoadout>.JsonLoadFile( filepath, loadout );
		return loadout;
	}

	private void SpawnLoadout( JMLoadoutModule loadoutModule, JMLoadout loadout, vector position )
	{
		if ( !loadout || !loadout.m_Items || loadout.m_Items.Count() == 0 )
			return;

		foreach ( JMLoadoutItem itemData: loadout.m_Items )
		{
			vector pos;
			if ( loadout.m_IsLocalPosition )
				pos = position - itemData.m_LocalPosition;
			else
				pos = itemData.m_LocalPosition;

			EntityAI spawnedItem = loadoutModule.SpawnItem( itemData, pos, loadout.m_IsLocalPosition );
		}
	}

	private void SpawnLoadout( JMLoadoutModule loadoutModule, JMLoadout loadout, EntityAI ent )
	{
		if ( !loadout || !loadout.m_Items || loadout.m_Items.Count() == 0 )
			return;

		if ( !ent )
			return;

		foreach ( JMLoadoutItem itemData: loadout.m_Items )
		{
			if ( loadoutModule.IsKindOfLoadout( itemData.m_Classname, ent.GetType() ) )
			{
				foreach ( JMLoadoutSubItem subItemData: itemData.m_Attachments )
					loadoutModule.SpawnInItem( subItemData, ent );
			}
			else
			{
				EntityAI spawnedItem = loadoutModule.SpawnItem( itemData, ent );
			}
		}
	}
}
