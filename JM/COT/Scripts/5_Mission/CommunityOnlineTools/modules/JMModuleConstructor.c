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
//#ifdef NAMALSK_SURVIVAL
		modules.Insert( JMEventSpawnerModule );
//#endif

		//modules.Insert( JMWebhookCOTModule );
/*#ifdef VPPADMINTOOLS
		modules.Insert( JMVPPMenuPlayerManager );
		modules.Insert( JMVPPMenuItemManager );
		modules.Insert( JMVPPMenuTeleportManager );
		modules.Insert( JMVPPEspToolsMenu );
		modules.Insert( JMVPPMenuCommandsConsole );
		modules.Insert( JMVPPMenuServerManager );
		modules.Insert( JMVPPMenuWeatherManager );
		modules.Insert( JMVPPMenuObjectManager );
		modules.Insert( JMVPPMenuBansManager );
		modules.Insert( JMVPPMenuPermissionsEditor );
		modules.Insert( JMVPPMenuWebHooks );
		modules.Insert( JMVPPMenuXMLEditor );
#endif*/
	}
}