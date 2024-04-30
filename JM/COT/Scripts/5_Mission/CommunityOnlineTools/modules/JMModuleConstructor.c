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

#ifdef DIAG
		//modules.Insert( JMTestingModule );
		//modules.Insert( JMMapEditorModule );
		modules.Insert( JMExampleModule );
		modules.Insert( JMWeatherModule );
#else
		modules.Insert( JMWeatherOldModule );
#endif

		//! TODO: Merge with ItemSets once it will use prefabs
		modules.Insert( JMVehicleSpawnerModule );
		modules.Insert( JMItemSetSpawnerModule );
		
		modules.Insert( JMMapModule );
		modules.Insert( JMCommandModule );

		modules.Insert( JMVehiclesModule );

		//! TODO: make the territory module part of COT and make Expansion expand on it
		//! 	  It will allow other Territory mods to do the same if they want 
#ifdef EXPANSION_MODULES
		modules.Insert( JMTerritoriesModule );
#endif

		//! TODO: Find a way to check of offline mode without being triggered while in main menu
		//if ( IsMissionOffline() )
		//	modules.Insert( JMMissionSelectorModule );

		
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