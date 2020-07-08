class JMWebhookModule: JMModuleBase
{
	private ref JsonSerializer m_Serializer;

    private RestApi m_Core;

    private string m_DefaultContextURL;
    private string m_DefaultAddress;

    private bool m_IsLoading;

    private ref array< ref JMWebhookConnection > m_Connections;
    private ref map< string, ref array< JMWebhookConnection > > m_ConnectionMap;

    void JMWebhookModule()
    {
    }

    void ~JMWebhookModule()
    {
        delete m_Serializer;

        delete m_Connections;
        delete m_ConnectionMap;
    }

    override void OnInit()
    {
        m_IsLoading = false;

        m_Serializer = new JsonSerializer();

        m_Connections = new array< ref JMWebhookConnection >();
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
        JMWebhookSerialize serialize = new JMWebhookSerialize;

		if ( FileExist( JMConstants.FILE_WEBHOOK ) )
		{
			serialize.Load();
            serialize.Deserialize( m_Connections, m_ConnectionMap );

            m_DefaultContextURL = "";
            m_DefaultAddress = "";
		} else
        {
            m_DefaultContextURL = "https://discordapp.com/api/webhooks/";
            m_DefaultAddress = "729943333564317726/_K1zSZcKi5qL2_qqJnUvgeH1cieGNxqkNtsxV640Yya-zaKcfMPN5yOTxQEoEAjk3TAS";
        }

        array< string > types = new array< string >;
        JMWebhookConstructor.Generate( types );
        for ( int i = 0; i < types.Count(); ++i )
        {
            AddConnection( types[i], m_DefaultContextURL, m_DefaultAddress );
        }

        serialize.Serialize( m_Connections );
		serialize.Save();

        m_IsLoading = false;
	}

	override void OnMissionLoaded()
	{
        auto message = CreateDiscordMessage();
                
        message.GetEmbed().AddField( "Server Status", "Server is starting up." );

        Post( "ServerStartup", message );
	}

    void SaveConnections()
    {
        JMWebhookSerialize serialize = new JMWebhookSerialize;
        serialize.Serialize( m_Connections );
		serialize.Save();
    }

    override void OnMissionFinish()
    {
        auto message = CreateDiscordMessage();

        message.GetEmbed().AddField( "Server Status", "Server has shutdown safely." );

        Post( "ServerShutdown", message );
    }

    private void AddConnection( string name, string context = "", string address = "", bool enabled = true )
    {
        array< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
        if ( !connections )
        {
            connections = new array< JMWebhookConnection >;
            m_ConnectionMap.Insert( name, connections );
        } else 
        {
            for ( int i = 0; i < connections.Count(); ++i )
            {
                if ( connections[i].ContextURL != context )
                    continue;
                if ( connections[i].Address != address )
                    continue;

                return;
            }
        }

        JMWebhookConnection connection = new JMWebhookConnection();
        connection.Name = name;
        connection.ContextURL = context;
        connection.Address = address;

        if ( m_IsLoading )
        {
            connection.Enabled = false;
        } else
        {
            connection.Enabled = enabled;
        }

        connections.Insert( connection );
        m_Connections.Insert( connection );
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