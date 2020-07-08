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

class JMWebhookSerialize : Managed
{
    ref array< ref JMWebhookConnectionSerialize > Connections;

    void JMWebhookSerialize()
    {
        Connections = new array< ref JMWebhookConnectionSerialize >;
    }

    void ~JMWebhookSerialize()
    {
        Connections.Clear();
        delete Connections;
    }

    void Load()
    {
		JsonFileLoader< JMWebhookSerialize >.JsonLoadFile( JMConstants.FILE_WEBHOOK, this );
    }

    void Save()
    {
		JsonFileLoader< JMWebhookSerialize >.JsonSaveFile( JMConstants.FILE_WEBHOOK, this );
    }

    void Deserialize( out array< ref JMWebhookConnection > connections, out map< string, ref array< JMWebhookConnection > > connectionsMap )
    {
        for ( int i = Connections.Count() - 1; i >= 0; --i )
        {
            string contextURL = Connections[i].ContextURL;
            string address = Connections[i].Address;

            for ( int j = 0; j < Connections[i].Types.Count(); ++j )
            {
                string name = Connections[i].Types[j].Name;

                array< JMWebhookConnection > mappedConnections = connectionsMap.Get( name );
                if ( !mappedConnections )
                {
                    mappedConnections = new array< JMWebhookConnection >;
                    connectionsMap.Insert( name, mappedConnections );
                }

                ref JMWebhookConnection connection = new JMWebhookConnection;
                connection.Name = name;
                connection.ContextURL = contextURL;
                connection.Address = address;
                connection.Enabled = Connections[i].Types[j].Enabled;

                mappedConnections.Insert( connection );
                connections.Insert( connection );
            }
        }
    }

    void Serialize( array< ref JMWebhookConnection > connections )
    {
        for ( int i = 0; i < connections.Count(); ++i )
        {
            JMWebhookConnectionSerialize conn = NULL;

            for ( int j = 0; j < Connections.Count(); ++j )
            {
                if ( Connections[j].ContextURL != connections[i].ContextURL )
                    continue;
                if ( Connections[j].Address != connections[i].Address )
                    continue;
                
                conn = Connections[j];
                break;
            }

            if ( !conn )
            {
                conn = new JMWebhookConnectionSerialize;
                conn.ContextURL = connections[i].ContextURL;
                conn.Address = connections[i].Address;
                Connections.Insert( conn );
            }

            JMWebhookConnectionTypeSerialize typeSerialize = new JMWebhookConnectionTypeSerialize();
            typeSerialize.Name = connections[i].Name;
            typeSerialize.Enabled = connections[i].Enabled;
            conn.Types.Insert( typeSerialize );
        }
    }

    void OnSend( ParamsWriteContext ctx )
    {
        ctx.Write( Connections );
    }

    void OnRecieve( ParamsReadContext ctx )
    {
        ctx.Read( Connections );
    }
};