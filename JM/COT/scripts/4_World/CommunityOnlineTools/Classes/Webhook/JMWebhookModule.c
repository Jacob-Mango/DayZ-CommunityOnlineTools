class JMWebhookQueueItem : Managed
{
    string m_Type;

    int m_Time;

    ref JMWebhookMessage m_Message;

    void JMWebhookQueueItem( string type, ref JMWebhookMessage message )
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

    ref JMWebhookMessage GetMessage()
    {
        return m_Message;
    }
};

class JMWebhookModule: JMModuleBase
{
    private RestApi m_Core;

    private ref map< string, ref set< JMWebhookConnection > > m_ConnectionMap;

    private ref array< ref JMWebhookQueueItem > m_Queue;

    private JMWebhookSerialize m_Settings;

    private ref ConfigFile m_ServerConfig;

    void JMWebhookModule()
    {
    }

    void ~JMWebhookModule()
    {
        delete m_ConnectionMap;
    }

    override void OnInit()
    {
        Print( "Start reading config" );

        string serverCfg;
		GetGame().CommandlineGetParam( "config", serverCfg );
        m_ServerConfig = ConfigFile.Parse( serverCfg );

        Print( "Finished reading config" );

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
            group.Address = "729943333564317726/_K1zSZcKi5qL2_qqJnUvgeH1cieGNxqkNtsxV640Yya-zaKcfMPN5yOTxQEoEAjk3TAS";
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

		GetGame().GameScript.Call( this, "Thread_ProcessQueue", NULL );
	}

	override void OnMissionLoaded()
	{
        super.OnMissionLoaded();

        auto message = CreateDiscordMessage();
                
        message.GetEmbed().AddField( "Server Status", "Server is starting up." );

        Post( "ServerStartup", message );
	}

	override void OnMissionFinish()
	{
        auto message = CreateDiscordMessage();

        message.GetEmbed().AddField( "Server Status", "Server has shutdown safely." );

        Post( "ServerShutdown", message );
        
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

            ref JMWebhookConnection conn = group.Add( name );
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

    bool AddConnection( string name, ref JMWebhookConnectionGroup group = NULL )
    {
        set< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
        if ( !connections )
        {
            connections = new set< JMWebhookConnection >();
            m_ConnectionMap.Insert( name, connections );
        }

        if (group != NULL)
        {
            ref JMWebhookConnection conn = group.Add( name );
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

    void Post( string connectionType, ref JMWebhookMessage message )
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
                ref JMWebhookQueueItem item = m_Queue[0];

                set< JMWebhookConnection > connections = m_ConnectionMap.Get( item.GetType() );
                if ( Assert_Null( connections ) )
                    return;

                for ( int i = 0; i < connections.Count(); i++ )
                    if ( connections[i] != NULL )
                        connections[i].Post( m_Core, serializer, item.GetMessage() );

                delete item;
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

    ref JMWebhookDiscordMessage CreateDiscordMessage()
    {
        ref JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
        auto embed = message.CreateEmbed();
        embed.SetColor( 16766720 );

        embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );
		
        ConfigEntry entry = m_ServerConfig.Get( "hostname" );
        if ( entry && entry.GetText() != "" )
        {
		    embed.AddField( "Server:", entry.GetText(), false );
        }

        return message;
    }

    ref JMWebhookDiscordMessage CreateDiscordMessage( JMPlayerInstance player, string title )
    {
        ref JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
        auto embed = message.CreateEmbed();
        embed.SetColor( 16766720 );

        embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

        ConfigEntry entry = m_ServerConfig.Get( "hostname" );
        if ( entry && entry.GetText() != "" )
        {
		    embed.AddField( "Server:", entry.GetText(), true );
        }

		embed.AddField( title, player.FormatSteamWebhook(), entry && entry.GetText() != "" );

        return message;
    }
};