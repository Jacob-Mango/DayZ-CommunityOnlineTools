class JMWebhookModule: JMModuleBase
{
	private ref JsonSerializer m_Serializer;

    private RestApi m_Core;

    private ref map< string, ref array< JMWebhookConnection > > m_ConnectionMap;

    private JMWebhookSerialize m_Settings;

    void JMWebhookModule()
    {
    }

    void ~JMWebhookModule()
    {
        delete m_Serializer;

        delete m_ConnectionMap;
    }

    override void OnInit()
    {
        m_Serializer = new JsonSerializer();

        m_Settings = GetCOTWebhookSettings();

        m_ConnectionMap = new map< string, ref array< JMWebhookConnection > >();

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
		if ( FileExist( JMConstants.FILE_WEBHOOK ) )
		{
			m_Settings.Load();
		} else
        {
            JMWebhookConnectionGroup group = m_Settings.Get( "Main" );

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
        #ifdef JM_COT_WEBHOOK_DEBUG
            AddConnection( types[i], "Main", true );
        #else
            AddConnection( types[i], "Main", false );
        #endif
        }

        FixConnectionMap();

		m_Settings.Save();
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

    void FixConnectionMap()
    {
        for ( int i = 0; i < m_ConnectionMap.Count(); ++i )
        {
            array< JMWebhookConnection > connections = m_ConnectionMap.GetElement( i );

            for ( int j = 0; j < connections.Count(); ++j )
            {
                if ( connections[j] == NULL )
                {
                    connections.RemoveOrdered( j );
                    j--;
                }
            }
        }

        for ( i = 0; i < m_Settings.Connections.Count(); ++i )
        {
            for ( j = 0; j < m_Settings.Connections[i].Types.Count(); ++j )
            {
                string name = m_Settings.Connections[i].Types[j].Name;

                array< JMWebhookConnection > mappedConnections = m_ConnectionMap.Get( name );
                if ( !mappedConnections )
                {
                    mappedConnections = new array< JMWebhookConnection >;
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

    bool AddConnection( string name, string grpName, bool enabled )
    {
        JMWebhookConnectionGroup group = m_Settings.Get( grpName );
        if ( Assert_Null( group ) )
            return false;

        array< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
        if ( !connections )
        {
            connections = new array< JMWebhookConnection >();
            m_ConnectionMap.Insert( name, connections );
        }

        ref JMWebhookConnection conn = group.SetNoOverride( name, enabled );
        if ( !conn )
            return false;

        int idx = connections.Find( conn );
        if ( idx >= 0 )
            return false;

        connections.Insert( conn );
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

        array< JMWebhookConnection > connections = m_ConnectionMap.Get( connectionType );
        if ( Assert_Null( connections ) )
            return;

        for ( int i = 0; i < connections.Count(); i++ )
        {
            connections[i].Post( m_Core, m_Serializer, message );
        }
    }

    ref JMWebhookDiscordMessage CreateDiscordMessage()
    {
        ref JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
        auto embed = message.CreateEmbed();
        embed.SetColor( 16766720 );

        embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );
		
        return message;
    }

    ref JMWebhookDiscordMessage CreateDiscordMessage( JMPlayerInstance player, string title )
    {
        ref JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
        auto embed = message.CreateEmbed();
        embed.SetColor( 16766720 );

        embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );
		
		embed.AddField( title, player.FormatSteamWebhook(), true );

        return message;
    }
};