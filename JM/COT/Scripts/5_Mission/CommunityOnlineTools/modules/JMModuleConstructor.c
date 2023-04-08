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

		modules.Insert( JMVehiclesModule );

#ifdef EXPANSION_MODULES
		modules.Insert( JMTerritoriesModule );
#endif

		//! TODO: Find a way to check of offline mode without being triggered while in main menu
		//if ( IsMissionOffline() )
		//	modules.Insert( JMMissionSelectorModule );

		//modules.Insert( JMTestingModule );
		
		// Namalsk specific checks
		if ( IsModLoaded("NamEventManager") && IsModLoaded("EVRStorm") )
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