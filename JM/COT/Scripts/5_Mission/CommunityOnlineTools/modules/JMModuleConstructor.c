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
		modules.Insert( JMCompensationsModule );
		modules.Insert( JMMapModule );
		modules.Insert( JMMapEditorModule );
		modules.Insert( JMAntiCheatModule );
		modules.Insert( JMVehiclesModule );
		modules.Insert( JMCommandModule );
		modules.Insert( JMBanModule );
		modules.Insert( JMRoleManagerModule );
		modules.Insert( JMWebhookCOTModule );
		modules.Insert( JMLootAnalysisModule );
		modules.Insert( JMEventsModule );
		modules.Insert( JMServerStatsModule );


#ifdef DIAG
		modules.Insert( JMExampleModule );
		modules.Insert( JMItemStatsModule );
#endif

		// Namalsk specific checks
		if ( IsModLoaded("NamEventManager") && IsModLoaded("EVRStorm") )
			modules.Insert( JMNamalskEventManagerModule );

#ifdef EXPANSIONMODBASEBUILDING
		modules.Insert( JMTerritoryModule );
#endif
	}
}

static bool IsModLoaded(string type_name_from_mod)
{
	if ( type_name_from_mod.ToType() )
		return true;

	return false;
}
