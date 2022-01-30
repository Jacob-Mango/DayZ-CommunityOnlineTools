class JMWebhookQueueItem : Managed
{
	string m_Type;

	int m_Time;

	ref COTWebhookMessage m_Message;

	void JMWebhookQueueItem( string type, COTWebhookMessage message )
	{
		m_Type = type;
		m_Message = message;
		m_Time = GetGame().GetTickTime();
	}

	void ~JMWebhookQueueItem()
	{
		delete m_Message;
	}

	string GetType()
	{
		return m_Type;
	}

	int GetTime()
	{
		return m_Time;
	}

	COTWebhookMessage GetMessage()
	{
		return m_Message;
	}
};

[CF_RegisterModule(COTWebhookModule)]
class COTWebhookModule: COTModule
{
	private RestApi m_Core;

	private autoptr map< string, ref set< JMWebhookConnection > > m_ConnectionMap;

	private autoptr array< ref JMWebhookQueueItem > m_Queue;

	private JMWebhookSerialize m_Settings;

	private autoptr ConfigFile m_ServerConfig;
	string m_ServerHostName;

	COTWebhook WH_ServerStartup;
	COTWebhook WH_ServerShutdown;

	override void OnInit()
	{
		super.OnInit();
		
		if (!GetGame().IsServer())
			return;
		
		string serverCfg;
		GetGame().CommandlineGetParam( "config", serverCfg );

		// attempt to fallback to defaults since otherwise it would fail regardless
		if ( serverCfg == "" )
			serverCfg = "serverdz.cfg";
		else if ( serverCfg.Contains( ":\\" ) || serverCfg.Contains( ":/" ) )
			serverCfg = "serverdz.cfg";

		m_ServerConfig = ConfigFile.Parse( serverCfg );
		if ( m_ServerConfig )
		{
			ConfigEntry entry = m_ServerConfig.Get( "hostname" );
			if ( entry && entry.GetText() != "" )
				m_ServerHostName = entry.GetText();
		}

		m_Settings = GetCOTWebhookSettings();

		m_ConnectionMap = new map< string, ref set< JMWebhookConnection > >();

		m_Core = CreateRestApi();
		m_Core.EnableDebug( true );
		
		EnableMissionStart();
		EnableMissionFinish();
		EnableMissionLoaded();
	}

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		JMWebhookConnectionGroup group = NULL;

		if ( FileExist( JMConstants.FILE_WEBHOOK ) )
		{
			m_Settings.Load();
		} else
		{
			group = m_Settings.Get( "Main" );

		#ifdef JM_COT_WEBHOOK_DEBUG
			group.ContextURL = "https://discord.com/api/webhooks/";
			group.Address = "937215489648783411/5HVZ-tgloq_oA7MuTFO1jEh56ZVKcjWxeFEJIFGKWTJ3oUykShBK6jCeptX_x8djkBXX";
		#else
			group.ContextURL = "";
			group.Address = "";
		#endif
		}

		foreach (auto name, auto webhook : COTWebhook.s_All)
		{
			AddConnection(name, group );
		}

		FixConnectionMap();

		m_Settings.Save();

		GetGame().GameScript.Call( this, "Thread_ProcessQueue", NULL );
	}

	override string GetWebhookTitle()
	{
		return "Server Status";
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		WH_ServerStartup.Send("Server is starting up.");
	}

	override void OnMissionFinish(Class sender, CF_EventArgs args)
	{
		WH_ServerShutdown.Send("Server is shutting down.");
		
		m_Settings.Save();
	}

	void SaveConnections()
	{
		JMWebhookSerialize serialize = new JMWebhookSerialize;
		m_Settings.Save();
	}

	bool RemoveGroup( string name )
	{
		m_Settings.Remove( name );

		FixConnectionMap();

		return true;
	}

	private void FixConnectionMap()
	{
		for ( int i = 0; i < m_Settings.Connections.Count(); ++i )
		{
			for ( int j = 0; j < m_Settings.Connections[i].Types.Count(); ++j )
			{
				string name = m_Settings.Connections[i].Types[j].Name;

				set< JMWebhookConnection > mappedConnections = m_ConnectionMap.Get( name );
				if ( !mappedConnections )
				{
					mappedConnections = new set< JMWebhookConnection >;
					m_ConnectionMap.Insert( name, mappedConnections );
				}

				mappedConnections.Insert( m_Settings.Connections[i].Types[j] );
			}
		}
	}

	bool SetConnection( string name, string grpName, bool enabled )
	{
		JMWebhookConnectionGroup group = m_Settings.Get( grpName );
		if ( Assert_Null( group ) )
			return false;

		group.Set( name, enabled );
		return true;
	}

	bool RemoveConnection( string name, string grpName )
	{
		JMWebhookConnectionGroup group = m_Settings.Get( grpName );
		if ( Assert_Null( group ) )
			return false;
			
		group.Remove( name );

		FixConnectionMap();
		return true;
	}

	bool AddConnection( string name, string grpName = "" )
	{
		set< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
		if ( !connections )
		{
			connections = new set< JMWebhookConnection >();
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
		set< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
		if ( !connections )
		{
			connections = new set< JMWebhookConnection >();
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

	void Post( string connectionType, COTWebhookMessage message )
	{
		if ( IsMissionClient() )
			return;

		if ( Assert_Null( message ) )
			return;

		m_Queue.Insert( new JMWebhookQueueItem( connectionType, message ) );
	}

	private void Thread_ProcessQueue()
	{
		JsonSerializer serializer = new JsonSerializer();

		m_Queue = new array< ref JMWebhookQueueItem >();

		int num = 0;
		int startTime = GetGame().GetTickTime();
		int lastSendTime = GetGame().GetTickTime();
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
						for ( int i = 0; i < connections.Count(); i++ )
							if ( connections[i] != NULL )
								connections[i].Post( m_Core, serializer, item.GetMessage() );
					}
					delete item;
				}

				m_Queue.RemoveOrdered( 0 );

				num++;
				lastSendTime = GetGame().GetTickTime();

				int defer = (int) Math.Clamp( num * 0.1, 1, 2 );
				Sleep( 250 * Math.Clamp( num, 1, 4 ) * defer );
			} else
			{
				if ( startTime - lastSendTime > 1000 )
				{
					startTime = GetGame().GetTickTime();
					lastSendTime = startTime;
					num = 0;
				}

				Sleep( 50 );
			}
		}

		delete m_Queue;
	}
};
