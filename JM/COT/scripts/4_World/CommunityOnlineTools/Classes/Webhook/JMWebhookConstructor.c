class JMWebhookConstructor : Managed
{
    private ref array< string > m_Types;

    private void JMWebhookConstructor()
    {
        m_Types = new array< string >();
    }

    void ~JMWebhookConstructor()
    {
        delete m_Types;
    }

    static void Generate( out array< string > types )
    {
        JMWebhookConstructor constructor = new JMWebhookConstructor();

        array< JMModuleBase > modules = GetModuleManager().GetAllModules();
        for ( int i = 0; i < modules.Count(); ++i )
        {
            JMModuleBase module = modules[i];
            constructor.AddConnection( module.GetModuleName() );

            array< string > moduleTypes = new array< string >;
            module.GetWebhookTypes( moduleTypes );
            for ( int j = 0; j < moduleTypes.Count(); ++j )
            {
                constructor.AddConnection( module.GetModuleName() + moduleTypes[j] );
            }
        }

        constructor.RegisterTypes();

        types.Copy( constructor.m_Types );
    }

    protected void RegisterTypes()
    {
        AddConnection( "ServerStartup" );
        AddConnection( "ServerShutdown" );

        AddConnection( "PlayerJoin" );
        AddConnection( "PlayerLeave" );
        AddConnection( "PlayerDeath" );
        AddConnection( "PlayerDamage" );

        AddConnection( "AdminActive" );
    }

    protected void AddConnection( string type )
    {
        m_Types.Insert( type );
    }
};