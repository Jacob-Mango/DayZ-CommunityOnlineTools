class JMWebhookQueueItem : Managed
{
	string m_Type;

	int m_Time;

	ref JMWebhookMessage m_Message;

	void JMWebhookQueueItem( string type, JMWebhookMessage message )
	{
		m_Type = type;
		m_Message = message;
		m_Time = g_Game.GetTickTime();
	}

	string GetType()
	{
		return m_Type;
	}

	int GetTime()
	{
		return m_Time;
	}

	JMWebhookMessage GetMessage()
	{
		return m_Message;
	}
}

class JMWebhookModule: JMModuleBase
{
	static ref JsonSerializer s_Serializer = new JsonSerializer();

	private RestApi m_Core;

	private ref map< string, ref set< JMWebhookConnection > > m_ConnectionMap;

	private ref array< ref JMWebhookQueueItem > m_Queue;

	private JMWebhookSerialize m_Settings;

	private string m_ServerHostName;

	void JMWebhookModule()
	{
		m_Queue = new array< ref JMWebhookQueueItem >();
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

		JMWebhookSerialize serialize = new JMWebhookSerialize;
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

	private void FixConnectionMap()
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

	private void Thread_ProcessQueue()
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
					if ( !Assert_Null( connections ) )
					{
						#ifdef JM_COT_DIAG_LOGGING
						CF_Log.Debug("Thread_ProcessQueue - item type \"%1\"", item.GetType());
						#endif
						for ( int i = 0; i < connections.Count(); i++ )
							if ( connections[i] != NULL )
								connections[i].Post( m_Core, s_Serializer, item.GetMessage() );
					}
					else
					{
						CF_Log.Error("Thread_ProcessQueue - webhook type \"%1\" not registered, please override JMModuleBase::GetWebhookTypes to insert it", item.GetType());
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

		if ( m_ServerHostName != "" )
			embed.AddField( "Server:", m_ServerHostName, false );

		return message;
	}

	// Creates a message with a custom embed color.
	JMWebhookDiscordMessage CreateDiscordMessageColored( int color )
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( color );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		if ( m_ServerHostName != "" )
			embed.AddField( "Server:", m_ServerHostName, false );

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

		if ( m_ServerHostName != "" )
			embed.AddField( "Server:", m_ServerHostName, true );

		embed.AddField( title, player.FormatSteamWebhook(), m_ServerHostName != "" );

		return message;
	}

	// Creates an admin-action message with a custom embed color and optional target player.
	JMWebhookDiscordMessage CreateDiscordMessageAdmin( JMPlayerInstance admin, string title, int color )
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( color );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		if ( m_ServerHostName != "" )
			embed.AddField( "Server:", m_ServerHostName, true );

		embed.AddField( title, admin.FormatSteamWebhook(), m_ServerHostName != "" );

		return message;
	}
}