class JMWebhookConnectionSerialize
{
    string ContextURL;
    string Address;

    ref array< string > Types;

    void JMWebhookConnectionSerialize()
    {
        Types = new array< string >;
    }

    void ~JMWebhookConnectionSerialize()
    {
        Types.Clear();
        delete Types;
    }
};

class JMWebhookModuleSerialize
{
    ref array< ref JMWebhookConnectionSerialize > Connections;

    void JMWebhookModuleSerialize()
    {
        Connections = new array< ref JMWebhookConnectionSerialize >;
    }

    void ~JMWebhookModuleSerialize()
    {
        Connections.Clear();
        delete Connections;
    }
};

class JMWebhookModule: JMModuleBase
{
	private ref JsonSerializer m_Serializer;

    private string m_ContextURL;
    private string m_Address;

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
    }

	override void OnMissionStart()
	{
		if ( FileExist( JMConstants.FILE_WEBHOOK ) )
		{
            JMWebhookModuleSerialize serialize = new JMWebhookModuleSerialize;

			JsonFileLoader< JMWebhookModuleSerialize >.JsonLoadFile( JMConstants.FILE_WEBHOOK, serialize );

            for ( int i = 0; i < serialize.Connections.Count(); ++i )
            {
                string contextURL = serialize.Connections[i].ContextURL;
                string address = serialize.Connections[i].Address;

                for ( int j = 0; j < serialize.Connections[i].Types.Count(); ++j )
                {
                    string name = serialize.Connections[i].Types[j];

                    ref JMWebhookConnection connection = new JMWebhookConnection;
                    connection.Name = name;
                    connection.ContextURL = contextURL;
                    connection.Address = address;

                    array< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
                    if ( !connections )
                    {
                        connections = new array< JMWebhookConnection >;
                        m_ConnectionMap.Insert( name, connections );
                    }

                    connections.Insert( connection );
                    m_Connections.Insert( connection );
                }
            }

            OnConnectionSetup();
		} else 
		{
            m_ContextURL = "https://discordapp.com/api/webhooks/";
            m_Address = "729943333564317726/_K1zSZcKi5qL2_qqJnUvgeH1cieGNxqkNtsxV640Yya-zaKcfMPN5yOTxQEoEAjk3TAS";

            OnConnectionSetup();
            SaveConnections();
		}

		RestApi core = CreateRestApi();
		core.EnableDebug( true );

        for ( int k = 0; k < m_Connections.Count(); ++k )
        {
            m_Connections[k].Init( core );
        }

        m_IsLoading = false;
	}

	override void OnMissionLoaded()
	{
        auto message = CreateDiscordMessage();
                
        message.GetEmbed().AddField( "Server Status", "Online" );

        Post( "Server", message );
	}

    void SaveConnections()
    {
        JMWebhookModuleSerialize serialize = new JMWebhookModuleSerialize;
        
        for ( int i = 0; i < m_Connections.Count(); ++i )
        {
            bool found = false;

            for ( int j = 0; j < serialize.Connections.Count(); ++j )
            {
                if ( serialize.Connections[j].ContextURL != m_Connections[i].ContextURL )
                    continue;
                if ( serialize.Connections[j].Address != m_Connections[i].Address )
                    continue;
                
                found = true;

                serialize.Connections[j].Types.Insert( m_Connections[i].Name );
            }

            if ( !found )
            {
                JMWebhookConnectionSerialize conn = new JMWebhookConnectionSerialize;
                conn.ContextURL = m_Connections[i].ContextURL;
                conn.Address = m_Connections[i].Address;
                serialize.Connections.Insert( conn );
            }
        }

		JsonFileLoader< JMWebhookModuleSerialize >.JsonSaveFile( JMConstants.FILE_WEBHOOK, serialize );
    }

    override void OnMissionFinish()
    {
        auto message = CreateDiscordMessage();

        message.GetEmbed().AddField( "Server Status", "Offline" );

        Post( "Server", message );
    }

    void OnConnectionSetup()
    {
        array< JMModuleBase > modules = GetModuleManager().GetAllModules();
        for ( int i = 0; i < modules.Count(); ++i )
        {
            JMModuleBase module = modules[i]
            AddConnection( module.GetModuleName() );

            array< string > types = new array< string >;
            module.GetWebhookTypes( types );
            for ( int j = 0; j < types.Count(); ++j )
            {
                AddConnection( module.GetModuleName() + types[j] );
            }
        }

        AddConnection( "Server" );

        AddConnection( "PlayerJoin" );
        AddConnection( "PlayerLeave" );
        AddConnection( "PlayerDeath" );
        AddConnection( "PlayerDamaged" );
    }

    void AddConnection( string name, string context = "", string address = "", bool enabled = true )
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
        connection.ContextURL = m_ContextURL;
        connection.Address = m_Address;

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
        if ( Assert_Null( message ) )
            return;

        array< JMWebhookConnection > connections = m_ConnectionMap.Get( connectionType );
        if ( Assert_Null( connections ) )
            return;

        for ( int i = 0; i < connections.Count(); i++ )
        {
            connections[i].Post( m_Serializer, message );
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