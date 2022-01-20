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

#ifdef CF_MODULES
		int count = CF_ModuleCoreManager.Count();
		for (int i = 0; i < count; i++)
		{
			CF_ModuleWorld module;
			if (!Class.CastTo(module, CF_ModuleCoreManager.Get(i))) continue;
#else
		array< CF_ModuleWorld > modules = GetModuleManager().GetAllModules();
		for ( int i = 0; i < modules.Count(); ++i )
		{
			CF_ModuleWorld module = modules[i];
#endif
			constructor.AddConnection( module.ClassName() );

			array< string > moduleTypes = new array< string >;
			module.GetWebhookTypes( moduleTypes );
			for ( int j = 0; j < moduleTypes.Count(); ++j )
			{
				constructor.AddConnection( module.ClassName() + moduleTypes[j] );
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