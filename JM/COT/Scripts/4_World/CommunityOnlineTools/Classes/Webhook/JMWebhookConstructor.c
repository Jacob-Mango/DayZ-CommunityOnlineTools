class JMWebhookConstructor : Managed
{
	private ref array< string > m_Types;

	private void JMWebhookConstructor()
	{
		m_Types = new array< string >();
	}

	static void Generate( out array< string > types )
	{
		JMWebhookConstructor constructor = new JMWebhookConstructor();

		int count = 0;
#ifdef CF_MODULES
		count = CF_ModuleCoreManager.Count();
#else
		array< JMModuleBase > modules = GetModuleManager().GetAllModules();
		count = modules.Count();
#endif
		for (int i = 0; i < count; i++)
		{
			JMModuleBase module;
#ifdef CF_MODULES
			if (!Class.CastTo(module, CF_ModuleCoreManager.Get(i))) continue;
#else
			module = modules[i];
#endif
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
		AddConnection( "PlayerDeathNoPos" );
		AddConnection( "PlayerDamage" );

		AddConnection( "AdminActive" );

		AddConnection( "TerritoryDeployed" );
		AddConnection( "ItemDeployed" );

		AddConnection( "PlayerCount" );
	}

	protected void AddConnection( string type )
	{
		m_Types.Insert( type );
	}
}