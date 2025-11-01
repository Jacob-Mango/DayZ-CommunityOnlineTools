modded class JMModuleConstructor
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		modules.Insert( JMPlayerModule );
		modules.Insert( JMObjectSpawnerModule );
		modules.Insert( JMESPModule );
		modules.Insert( JMTeleportModule );
		modules.Insert( JMCameraModule );
		modules.Insert( JMWeatherModule );
		modules.Insert( JMLoadoutModule );
		modules.Insert( JMMapModule );
		modules.Insert( JMVehiclesModule );
		modules.Insert( JMCommandModule );

#ifdef DIAG
		modules.Insert( JMExampleModule );
		modules.Insert( JMtemStatsModule );
#endif

		if ( IsMissionOffline() )
			modules.Insert( JMMissionSelectorModule );
		else if (modules.Find(JMMissionSelectorModule) != -1)
			modules.RemoveItem(JMMissionSelectorModule);
		
		// Namalsk specific checks
		if ( IsModLoaded("NamEventManager") && IsModLoaded("EVRStorm") )
			modules.Insert( JMNamalskEventManagerModule );
	}
};

static bool IsModLoaded(string type_name_from_mod)
{
	return (type_name_from_mod.ToType().ToString().Length() > 0);
};
