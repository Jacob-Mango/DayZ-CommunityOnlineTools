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
		modules.Insert( JMWeatherOldModule );
#endif

		modules.Insert( JMWeatherModule );

		//! TODO: Merge with ItemSets once it will use prefabs
		modules.Insert( JMVehicleSpawnerModule );
		modules.Insert( JMItemSetSpawnerModule );
		
		modules.Insert( JMMapModule );
		modules.Insert( JMCommandModule );

		modules.Insert( JMVehiclesModule );

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