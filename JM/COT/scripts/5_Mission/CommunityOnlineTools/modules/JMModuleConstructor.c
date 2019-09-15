modded class JMModuleConstructor
{
    override void RegisterModules( out TTypenameArray modules )
	{
        super.RegisterModules( modules );

		// modules.Insert( JMServerInfoModule );
		modules.Insert( JMSelectedModule );
		modules.Insert( JMPlayerModule );
		modules.Insert( JMObjectSpawnerModule );
		modules.Insert( JMESPModule );
		// modules.Insert( JMMapEditorModule );
		modules.Insert( JMTeleportModule );
		modules.Insert( JMCameraModule );
		modules.Insert( JMItemSetSpawnerModule );
		modules.Insert( JMVehicleSpawnerModule );
		modules.Insert( JMWeatherModule );
		modules.Insert( JMMapModule );
    }
}