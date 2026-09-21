class JMCompensationsModule: JMRenderableModuleBase
{
	protected ref array< ref JMCompensationEntry > m_CachedCompensations;

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

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "Compensations";
		info.Icon = "hand-coins";
		info.Layout = "JM\\COT\\GUI\\layouts\\compensations_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_ITEMS;
		info.ViewPermission = JMConstants.PERM_COMPENSATIONS_VIEW;
		info.InputToggle = "UACOTToggleCompensations";
		info.WebhookTitle = "Compensations Module";
		info.SetRPCRange( JMCompensationsModuleRPC.INVALID, JMCompensationsModuleRPC.COUNT );

		//! Temporarily hidden from the sidebar.
		info.HasButton = false;
	}

	//! The spawn and delete permissions and their webhook types come from the actions.
	override void RegisterActions()
	{
		super.RegisterActions();

		DefineAction( JMCompensationSpawnCursor );
		DefineAction( JMCompensationSpawnTarget );
		DefineAction( JMCompensationSpawnPlayers );
		DefineAction( JMCompensationDelete );
	}

	override void EnableUpdate()
	{
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMCompensationsModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		default:
			RunAction( sender, rpc_type, ctx );
			break;
		}
	}

	override void RequestData()
	{
		Load();
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
			Exec_Load( NULL );
		}
	}

	protected void Exec_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( ident && !JMPermissions.Has( JMConstants.PERM_COMPENSATIONS_VIEW, ident, instance ) )
			return;

		array< ref JMCompensationEntry > compensations = GetCompensations();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( compensations );
		rpc.Send( NULL, JMCompensationsModuleRPC.Load, true, ident );
	}

	protected void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			Exec_Load( senderRPC );
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
		JMCompensationSpawnCursor action = new JMCompensationSpawnCursor();
		action.SteamID = steamID;
		action.Timestamp = timestamp;
		action.Position = position;
		SubmitAction( action );
	}

	void SpawnTarget( string steamID, string timestamp, EntityAI ent )
	{
		JMCompensationSpawnTarget action = new JMCompensationSpawnTarget();
		action.SteamID = steamID;
		action.Timestamp = timestamp;
		action.Target = ent;
		SubmitAction( action );
	}

	void SpawnPlayers( string steamID, string timestamp, array< string > guids )
	{
		JMCompensationSpawnPlayers action = new JMCompensationSpawnPlayers();
		action.SteamID = steamID;
		action.Timestamp = timestamp;
		action.GUIDs = guids;
		SubmitAction( action );
	}

	void Delete( string steamID, string timestamp )
	{
		JMCompensationDelete action = new JMCompensationDelete();
		action.SteamID = steamID;
		action.Timestamp = timestamp;
		SubmitAction( action );
	}

	//! What the actions do once they are permitted. False when there was nothing to spawn or delete.
	bool SpawnAtPosition( string steamID, string timestamp, vector position )
	{
		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return false;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return false;

		SpawnLoadout( loadoutModule, loadout, position );
		return true;
	}

	bool SpawnOnEntity( string steamID, string timestamp, EntityAI ent )
	{
		if ( !ent )
			return false;

		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return false;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return false;

		SpawnLoadout( loadoutModule, loadout, ent );
		return true;
	}

	bool SpawnOnPlayers( string steamID, string timestamp, array< string > guids )
	{
		JMLoadout loadout = LoadCompensation( steamID, timestamp );
		if ( !loadout )
			return false;

		JMLoadoutModule loadoutModule = JMLoadoutModule.Cast( GetModuleManager().GetModule( JMLoadoutModule ) );
		if ( !loadoutModule )
			return false;

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

		return true;
	}

	bool DeleteCompensation( string steamID, string timestamp )
	{
		return DeleteFile( JMConstants.DIR_COMPENSATIONS + steamID + "\\" + timestamp + JMConstants.EXT_LOADOUT );
	}

	protected JMLoadout LoadCompensation( string steamID, string timestamp )
	{
		string filepath = JMConstants.DIR_COMPENSATIONS + steamID + "\\" + timestamp + JMConstants.EXT_LOADOUT;
		if ( !FileExist( filepath ) )
			return NULL;

		JMLoadout loadout = new JMLoadout();
		JMJsonFile<JMLoadout>.Load( filepath, loadout );
		return loadout;
	}

	protected void SpawnLoadout( JMLoadoutModule loadoutModule, JMLoadout loadout, vector position )
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

	protected void SpawnLoadout( JMLoadoutModule loadoutModule, JMLoadout loadout, EntityAI ent )
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
