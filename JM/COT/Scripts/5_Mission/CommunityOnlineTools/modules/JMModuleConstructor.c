modded class JMModuleConstructor
{
	override void RegisterModules( out TTypenameArray modules )
	{
		super.RegisterModules( modules );

		modules.Insert( JMPlayerModule );
		modules.Insert( JMObjectSpawnerModule );
		modules.Insert( JMESPModule );
		//modules.Insert( JMMapEditorModule );
		modules.Insert( JMTeleportModule );
		modules.Insert( JMCameraModule );
		modules.Insert( JMVehicleSpawnerModule );
		modules.Insert( JMItemSetSpawnerModule );
		//modules.Insert( JMWeatherModule );
		modules.Insert( JMWeatherOldModule );
		modules.Insert( JMMapModule );
		modules.Insert( JMCommandModule );

#ifdef EXPANSION_MODULES
		modules.Insert( JMVehiclesModule );
		modules.Insert( JMTerritoriesModule );
#endif

		if ( IsMissionOffline() )
			modules.Insert( JMMissionSelectorModule );

		//modules.Insert( JMTestingModule );
		
		// Namalsk specific checks
		if ( IsModLoaded("EventManager") && IsModLoaded("EVRStorm") )
		{
			modules.Insert( JMNamalskEventManagerModule );
		}


		//modules.Insert( JMWebhookCOTModule );
	}
};

static bool IsModLoaded(string type_name_from_mod)
{
	return (type_name_from_mod.ToType().ToString().Length() > 0);
};