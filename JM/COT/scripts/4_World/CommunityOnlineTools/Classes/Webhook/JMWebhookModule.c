class JMWebhookConnectionTypeSerialize : Managed
{
    string Name;
    bool Enabled;
};

class JMWebhookConnectionSerialize : Managed
{
    string ContextURL;
    string Address;

    ref array< ref JMWebhookConnectionTypeSerialize > Types;

    void JMWebhookConnectionSerialize()
    {
        Types = new array< ref JMWebhookConnectionTypeSerialize >;
    }

    void ~JMWebhookConnectionSerialize()
    {
        Types.Clear();
        delete Types;
    }
};

class JMWebhookModuleSerialize : Managed
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
                    string name = serialize.Connections[i].Types[j].Name;

                    array< JMWebhookConnection > connections = m_ConnectionMap.Get( name );
                    if ( !connections )
                    {
                        connections = new array< JMWebhookConnection >;
                        m_ConnectionMap.Insert( name, connections );
                    }

                    ref JMWebhookConnection connection = new JMWebhookConnection;
                    connection.Name = name;
                    connection.ContextURL = contextURL;
                    connection.Address = address;
                    connection.Enabled = serialize.Connections[i].Types[j].Enabled;

                    connections.Insert( connection );
                    m_Connections.Insert( connection );
                }
            }

            OnConnectionSetup();
		} else 
		{
            m_DefaultContextURL = "https://discordapp.com/api/webhooks/";
            m_DefaultAddress = "729943333564317726/_K1zSZcKi5qL2_qqJnUvgeH1cieGNxqkNtsxV640Yya-zaKcfMPN5yOTxQEoEAjk3TAS";

            OnConnectionSetup();
            SaveConnections();
		}

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
        JMWebhookModuleSerialize serialize = new JMWebhookModuleSerialize;
        
        for ( int i = 0; i < m_Connections.Count(); ++i )
        {
            JMWebhookConnectionSerialize conn = NULL;

            for ( int j = 0; j < serialize.Connections.Count(); ++j )
            {
                if ( serialize.Connections[j].ContextURL != m_Connections[i].ContextURL )
                    continue;
                if ( serialize.Connections[j].Address != m_Connections[i].Address )
                    continue;
                
                conn = serialize.Connections[j];
                break;
            }

            if ( !conn )
            {
                conn = new JMWebhookConnectionSerialize;
                conn.ContextURL = m_Connections[i].ContextURL;
                conn.Address = m_Connections[i].Address;
                serialize.Connections.Insert( conn );
            }

            JMWebhookConnectionTypeSerialize typeSerialize = new JMWebhookConnectionTypeSerialize();
            typeSerialize.Name = m_Connections[i].Name;
            typeSerialize.Enabled = m_Connections[i].Enabled;
            conn.Types.Insert( typeSerialize );
        }

		JsonFileLoader< JMWebhookModuleSerialize >.JsonSaveFile( JMConstants.FILE_WEBHOOK, serialize );
    }

    override void OnMissionFinish()
    {
        auto message = CreateDiscordMessage();

        message.GetEmbed().AddField( "Server Status", "Server has shutdown safely." );

        Post( "ServerShutdown", message );
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

        AddConnection( "ServerStartup" );
        AddConnection( "ServerShutdown" );

        AddConnection( "PlayerJoin" );
        AddConnection( "PlayerLeave" );
        AddConnection( "PlayerDeath" );
        AddConnection( "PlayerDamage" );
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
        connection.ContextURL = m_DefaultContextURL;
        connection.Address = m_DefaultAddress;

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