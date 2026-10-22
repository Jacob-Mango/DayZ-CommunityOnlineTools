class JMWebhookQueueItem : Managed
{
	string m_Type;
	int m_Time;
	ref JMWebhookMessage m_Message;

	// Optional target context for per-player/per-role filtering.
	// Empty strings mean "no specific target" (send to all groups).
	string m_TargetGUID;

	//! Comma separated role names the target held when the event was posted - kept on the item
	//! because the player may be gone from the permissions manager by the time it is sent.
	string m_TargetRole;

	//! A second player involved in the event (the killer in a death or hit), matched by the
	//! group filters exactly like the target.
	string m_OtherGUID;
	string m_OtherRole;

	//! Steam64 of each, so a group can be filtered by either the GUID or the Steam64 ID.
	string m_TargetSteam;
	string m_OtherSteam;

	void JMWebhookQueueItem( string type, JMWebhookMessage message, string targetGUID = "", string targetRole = "" )
	{
		m_Type = type;
		m_Message = message;
		m_Time = g_Game.GetTickTime();
		m_TargetGUID = targetGUID;
		m_TargetRole = targetRole;
	}

	JMWebhookMessage GetMessage()
	{
		return m_Message;
	}

	string GetTargetGUID()
	{
		return m_TargetGUID;
	}

	string GetTargetRole()
	{
		return m_TargetRole;
	}

	void SetOther( string guid, string roles, string steam = "" )
	{
		m_OtherGUID = guid;
		m_OtherRole = roles;
		m_OtherSteam = steam;
	}

	void SetTargetSteam( string steam )
	{
		m_TargetSteam = steam;
	}

	string GetTargetSteam()
	{
		return m_TargetSteam;
	}

	string GetOtherSteam()
	{
		return m_OtherSteam;
	}

	string GetOtherGUID()
	{
		return m_OtherGUID;
	}

	string GetOtherRole()
	{
		return m_OtherRole;
	}

	int GetTime()
	{
		return m_Time;
	}

	string GetType()
	{
		return m_Type;
	}
}

class JMWebhookModule: JMModuleBase
{
	static ref JsonSerializer s_Serializer = new JsonSerializer();
	protected RestApi m_Core;
	protected ref map< string, ref set< JMWebhookConnection > > m_ConnectionMap;
	protected ref array< ref JMWebhookQueueItem > m_Queue;
	protected JMWebhookSerialize m_Settings;
	protected string m_ServerHostName;

	void JMWebhookModule()
	{
		m_Queue = new array< ref JMWebhookQueueItem >();
	}

	bool SetConnection( string name, string grpName, bool enabled )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_3(this, "SetConnection").Add(name).Add(grpName).Add(enabled);
		#endif

		JMWebhookConnectionGroup group = m_Settings.Get( grpName );
		if ( Assert_Null( group ) )
			return false;

		group.Set( name, enabled );
		return true;
	}

	override void EnableUpdate()
	{
	}

	override void OnInit()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnInit");
		#endif

	#ifdef SERVER
		//! Dedicated server
		string serverCfg;
		g_Game.CommandlineGetParam( "config", serverCfg );

		string profile;
		g_Game.CommandlineGetParam( "profiles", profile );

		serverCfg.ToLower();
		profile.ToLower();

		string serverCfgAbs = serverCfg;

		if (profile && serverCfg.IndexOf(profile) == 0)
		{
			int len = profile.Length();
			serverCfg = serverCfg.Substring(len, serverCfg.Length() - len);

			while (serverCfg[0] == "\\" || serverCfg[0] == "/")
			{
				serverCfg = serverCfg.Substring(1, serverCfg.Length() - 1);
			}

			serverCfg = "$profile:" + serverCfg;
		}

		// attempt to fallback to defaults since otherwise it would fail regardless
		if ( serverCfg == "" )
		{
			CF_Log.Info("No server config file set, using default serverdz.cfg");
			serverCfg = "serverdz.cfg";
		}
		else if (serverCfg.IndexOf("$profile:") != 0 && (serverCfg.Contains(":\\") || serverCfg.Contains(":/")))
		{
			serverCfg.Replace("\\", "/");
			TStringArray serverCfgFullPath = {};
			serverCfg.Split("/", serverCfgFullPath);
			serverCfg = serverCfgFullPath[serverCfgFullPath.Count() - 1];
			CF_Log.Warn("Cannot resolve absolute path '%1', treating filename '%2' as relative to $currentdir", serverCfgAbs, serverCfg);
		}
		else if (serverCfg != serverCfgAbs)
		{
			CF_Log.Info("Resolved absolute path '%1' to '%2'", serverCfgAbs, serverCfg);
		}

		ConfigFile cfg = ConfigFile.Parse( serverCfg );
		if ( cfg )
		{
			ConfigEntry entry = cfg.Get( "hostname" );
			if ( entry && entry.GetText() != "" )
			{
				m_ServerHostName = entry.GetText();
				CF_Log.Info("Got hostname '%1' from '%2'", m_ServerHostName, serverCfg);
			}
			else
			{
				CF_Log.Warn("No hostname set in '%1', server name in Discord webhook messages will be empty", serverCfg);
			}

			delete cfg;
		}
		else
		{
			CF_Log.Warn("Couldn't read '%1', server name in Discord webhook messages will be empty", serverCfg);
		}
	#else
		//! Client or singleplayer/offline mode
		m_ServerHostName = g_Game.GetHostName();
	#endif

		m_Settings = GetCOTWebhookSettings();

		m_ConnectionMap = new map< string, ref set< JMWebhookConnection > >();

		m_Core = CreateRestApi();
		m_Core.EnableDebug( true );

		PluginAdminLog adminLog;
		if ( IsMissionHost() && Class.CastTo( adminLog, GetPlugin( PluginAdminLog ) ) )
		{
			adminLog.SetWebhook( this );
		}
	}

	override void OnMissionStart()
	{
		JMWebhookConnectionGroup group = NULL;

		if ( FileExist( JMConstants.FILE_WEBHOOK ) )
		{
			m_Settings.Load();
		} else
		{
			group = m_Settings.Get( "Main" );

		#ifdef JM_COT_WEBHOOK_DEBUG
			group.ContextURL = "https://discordapp.com/api/webhooks/";
			group.Address = "YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN";
		#else
			group.ContextURL = "";
			group.Address = "";
		#endif
		}

		array< string > types = new array< string >;
		JMWebhookConstructor.Generate( types );
		for ( int i = 0; i < types.Count(); ++i )
		{
			AddConnection( types[i], group );
		}

		FixConnectionMap();

		m_Settings.Save();

		g_Game.GameScript.Call( this, "Thread_ProcessQueue", NULL );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		auto message = CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_SUCCESS );
		message.GetEmbed().SetTitle( "Server Online" );
		message.GetEmbed().SetDescription( "The server has started up and is accepting connections." );

		Post( "ServerStartup", message );
	}

	override void OnMissionFinish()
	{
		auto message = CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_NEUTRAL );
		message.GetEmbed().SetTitle( "Server Offline" );
		message.GetEmbed().SetDescription( "The server has shut down safely." );

		Post( "ServerShutdown", message );
	}

	void SaveConnections()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "SaveConnections");
		#endif

		m_Settings.Save();
	}

	bool RemoveGroup( string name )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RemoveGroup").Add(name);
		#endif

		m_Settings.Remove( name );

		FixConnectionMap();

		return true;
	}

	protected void FixConnectionMap()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "FixConnectionMap");
		#endif

		for ( int i = 0; i < m_Settings.Connections.Count(); ++i )
		{
			for ( int j = 0; j < m_Settings.Connections[i].Types.Count(); ++j )
			{
				string name = m_Settings.Connections[i].Types[j].Name;

				set< JMWebhookConnection > mappedConnections = m_ConnectionMap.Get( name );
				if ( !mappedConnections )
				{
					mappedConnections = new set< JMWebhookConnection >;
					#ifdef JM_COT_DIAG_LOGGING
					CF_Log.Debug("FixConnectionMap %1", name);
					#endif
					m_ConnectionMap.Insert( name, mappedConnections );
				}

				mappedConnections.Insert( m_Settings.Connections[i].Types[j] );
			}
		}
	}

	bool RemoveConnection( string name, string grpName )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RemoveConnection").Add(name).Add(grpName);
		#endif

		JMWebhookConnectionGroup group = m_Settings.Get( grpName );
		if ( Assert_Null( group ) )
			return false;
			
		group.Remove( name );

		FixConnectionMap();
		return true;
	}

	bool AddConnection( string name, string grpName = "" )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "AddConnection").Add(name).Add(grpName);
		#endif

		set< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
		if ( !connections )
		{
			connections = new set< JMWebhookConnection >();
			#ifdef JM_COT_DIAG_LOGGING
			CF_Log.Debug("AddConnection(\"%1\", \"%2\")", name, grpName);
			#endif
			m_ConnectionMap.Insert( name, connections );
		}

		if (grpName != "")
		{
			JMWebhookConnectionGroup group = m_Settings.Get( grpName );
			if ( Assert_Null( group ) )
				return false;

			JMWebhookConnection conn = group.Add( name );
			if ( Assert_Null( conn ) )
				return false;

			int idx = connections.Find( conn );
			if ( idx >= 0 )
				return false;

			connections.Insert( conn );
			return true;
		}

		return true;
	}

	bool AddConnection( string name, JMWebhookConnectionGroup group = NULL )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "AddConnection").Add(name).Add(group);
		#endif

		set< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
		if ( !connections )
		{
			connections = new set< JMWebhookConnection >();
			#ifdef JM_COT_DIAG_LOGGING
			CF_Log.Debug("AddConnection(\"%1\", %2)", name, group.ToString());
			#endif
			m_ConnectionMap.Insert( name, connections );
		}

		if (group != NULL)
		{
			JMWebhookConnection conn = group.Add( name );
			if ( Assert_Null( conn ) )
				return false;

			int idx = connections.Find( conn );
			if ( idx >= 0 )
				return false;

			connections.Insert( conn );
			return true;
		}

		return true;
	}

	override bool IsClient()
	{
		return false;
	}

	void Post( string connectionType, JMWebhookMessage message )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Post").Add(connectionType).Add(message);
		#endif

		if ( IsMissionClient() )
			return;

		if ( Assert_Null( message ) )
			return;

		m_Queue.Insert( new JMWebhookQueueItem( connectionType, message ) );
	}

	// Post with an optional target GUID / role for per-player or per-role filtering.
	void Post( string connectionType, JMWebhookMessage message, string targetGUID, string targetRole = "" )
	{
		if ( IsMissionClient() )
			return;

		if ( Assert_Null( message ) )
			return;

		m_Queue.Insert( new JMWebhookQueueItem( connectionType, message, targetGUID, targetRole ) );
	}

	//! Post an event done by (or to) a player: a group filtered to that player's GUID or to one of
	//! their roles receives it. `other` is a second player in the same event, e.g. the killer.
	//! Either may be NULL; with neither it is a plain Post().
	void PostAs( string connectionType, JMWebhookMessage message, JMPlayerInstance player, JMPlayerInstance other = NULL )
	{
		if ( IsMissionClient() )
			return;

		if ( Assert_Null( message ) )
			return;

		string guid = "";
		string roles = "";

		if ( player )
		{
			guid = player.GetGUID();
			roles = JoinRoles( player );
		}

		JMWebhookQueueItem item = new JMWebhookQueueItem( connectionType, message, guid, roles );

		if ( player )
			item.SetTargetSteam( player.GetSteam64ID() );

		if ( other )
			item.SetOther( other.GetGUID(), JoinRoles( other ), other.GetSteam64ID() );

		m_Queue.Insert( item );
	}

	//! The permissions instance behind a player entity, or NULL (AI, a body with no identity).
	JMPlayerInstance InstanceOf( PlayerBase player )
	{
		if ( !player )
			return NULL;

		PlayerIdentity identity = player.GetIdentity();
		if ( !identity )
			return NULL;

		return GetPermissionsManager().GetPlayer( identity.GetId() );
	}

	protected string JoinRoles( JMPlayerInstance player )
	{
		string joined = "";

		array< string > roles = player.GetRoles();
		for ( int i = 0; i < roles.Count(); i++ )
			joined += "," + roles[i];

		return joined;
	}

	//! A group with a GUID and/or role filter only receives events whose target or other player
	//! matches it (both filters, when both are set). Unfiltered groups receive everything.
	protected bool MatchesGroupFilter( JMWebhookConnectionGroup grp, JMWebhookQueueItem item )
	{
		if ( grp.FilterGUID != "" )
		{
			bool isTarget = grp.FilterGUID == item.GetTargetGUID() || grp.FilterGUID == item.GetTargetSteam();
			bool isOther  = grp.FilterGUID == item.GetOtherGUID() || grp.FilterGUID == item.GetOtherSteam();

			if ( !isTarget && !isOther )
				return false;
		}

		if ( grp.FilterRole != "" )
		{
			if ( !HasRoleFor( grp.FilterRole, item.GetTargetGUID(), item.GetTargetRole() ) && !HasRoleFor( grp.FilterRole, item.GetOtherGUID(), item.GetOtherRole() ) )
				return false;
		}

		return true;
	}

	//! `roles` is what the player held when the event was posted; the permissions manager is the
	//! fallback for a Post() that only knew the GUID.
	protected bool HasRoleFor( string role, string guid, string roles )
	{
		if ( guid == "" )
			return false;

		string padded = roles + ",";
		if ( padded.IndexOf( "," + role + "," ) != -1 )
			return true;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( guid );

		return pi && pi.HasRole( role );
	}

	protected void Thread_ProcessQueue()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Thread_ProcessQueue");
		#endif

		int num = 0;
		int startTime = g_Game.GetTickTime();
		int lastSendTime = g_Game.GetTickTime();
		int qps = 1;

		while ( true )
		{
			if ( m_Queue.Count() > 0 )
			{
				JMWebhookQueueItem item = m_Queue[0];

				if ( !Assert_Null( item ) )
				{
					set< JMWebhookConnection > connections = m_ConnectionMap.Get( item.GetType() );
					if ( connections )
					{
						#ifdef JM_COT_DIAG_LOGGING
						CF_Log.Debug("Thread_ProcessQueue - item type \"%1\"", item.GetType());
						#endif
						for ( int i = 0; i < connections.Count(); i++ )
						{
							if ( connections[i] == NULL )
								continue;

							// Check group-level filter: if the group has a GUID or role filter,
							// only fire if the queue item's target matches.
							JMWebhookConnectionGroup grp = connections[i].m_Group;
							if ( grp && !MatchesGroupFilter( grp, item ) )
								continue;

							connections[i].Post( m_Core, s_Serializer, item.GetMessage() );
						}
					}
					else
					{
						CF.FormatError("Thread_ProcessQueue - webhook type \"%1\" not registered, please override JMModuleBase::GetWebhookTypes to insert it", item.GetType());
					}
				}

				m_Queue.RemoveOrdered( 0 );

				num++;
				lastSendTime = g_Game.GetTickTime();

				int defer = (int) Math.Clamp( num * 0.1, 1, 2 );
				Sleep( 250 * Math.Clamp( num, 1, 4 ) * defer );
			} else
			{
				if ( startTime - lastSendTime > 1000 )
				{
					startTime = g_Game.GetTickTime();
					lastSendTime = startTime;
					num = 0;
				}

				Sleep( 50 );
			}
		}

		m_Queue.Clear();
	}

	JMWebhookDiscordMessage CreateDiscordMessage()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "CreateDiscordMessage");
		#endif

		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( JMConstants.WEBHOOK_COLOR_DEFAULT );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		return message;
	}

	// Creates a message with a custom embed color.
	JMWebhookDiscordMessage CreateDiscordMessageColored( int color )
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( color );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		return message;
	}

	JMWebhookDiscordMessage CreateDiscordMessage( JMPlayerInstance player, string title )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "CreateDiscordMessage").Add(player).Add(title);
		#endif

		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( JMConstants.WEBHOOK_COLOR_DEFAULT );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		AddPlayerFields( embed, player, title );

		return message;
	}

	// Creates an admin-action message with a custom embed color and optional target player.
	JMWebhookDiscordMessage CreateDiscordMessageAdmin( JMPlayerInstance admin, string title, int color )
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( color );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		AddPlayerFields( embed, admin, title );

		return message;
	}

	//! The acting player, up front on every event: who (linked to their Steam profile), their
	//! roles and where they stood. The caller adds what was done after these.
	protected void AddPlayerFields( JMWebhookDiscordEmbed embed, JMPlayerInstance player, string title )
	{
		embed.AddField( title, player.FormatSteamWebhook(), true );

		string roles = JoinRoleNames( player );
		if ( roles != "" )
			embed.AddField( "Role:", roles, true );

		if ( player.PlayerObject )
			embed.AddField( "Position:", FormatPosition( player.PlayerObject.GetPosition() ), true );
	}

	//! One decimal at most, so 29.999998 reads as 30.
	static string FormatNumber( float value )
	{
		return ( Math.Round( value * 10 ) / 10 ).ToString();
	}

	string JoinRoleNames( JMPlayerInstance player )
	{
		string names = "";

		array< string > roles = player.GetRoles();
		for ( int i = 0; i < roles.Count(); i++ )
		{
			if ( i > 0 )
				names += ", ";

			names += roles[i];
		}

		return names;
	}

	//! "x, y, z" in whole metres - what a webhook reader can paste into the map, without the
	//! float noise of vector.ToString().
	static string FormatPosition( vector position )
	{
		return Math.Round( position[0] ).ToString() + ", " + Math.Round( position[1] ).ToString() + ", " + Math.Round( position[2] ).ToString();
	}
}