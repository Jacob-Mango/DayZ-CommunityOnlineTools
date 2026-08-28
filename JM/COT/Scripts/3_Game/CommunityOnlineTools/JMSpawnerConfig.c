// Spawner configuration loaded from $profile:CommunityOnlineTools/SpawnerConfig.json
// Admins can edit this file at runtime to update blocked/restricted class lists
// without recompiling the mod.
class JMSpawnerConfig
{
	// Class names (lowercase) that are blocked from spawning due to being
	// unfinished, broken, or likely to crash/segfault the server.
	ref array< string > UnfinishedItems = new array< string >;

	// Substring patterns (lowercase) matched against class names to filter
	// out map objects, building pieces, etc. that should not be spawnable.
	ref array< string > RestrictedPatterns = new array< string >;

	// Substring patterns (lowercase) matched against class names to filter
	// out map objects, building pieces, etc. that should not be spawnable.
	ref array< string > RestrictedStartPatterns = new array< string >;

	static JMSpawnerConfig Defaults()
	{
		JMSpawnerConfig cfg = new JMSpawnerConfig();

		cfg.UnfinishedItems.Insert( "quickiebow" );
		cfg.UnfinishedItems.Insert( "recurvebow" );
		cfg.UnfinishedItems.Insert( "gp25base" );
		cfg.UnfinishedItems.Insert( "gp25" );
		cfg.UnfinishedItems.Insert( "gp25_standalone" );
		cfg.UnfinishedItems.Insert( "m203base" );
		cfg.UnfinishedItems.Insert( "m203" );
		cfg.UnfinishedItems.Insert( "m203_standalone" );
		cfg.UnfinishedItems.Insert( "red9" );
		cfg.UnfinishedItems.Insert( "pvcbow" );
		cfg.UnfinishedItems.Insert( "m249" );
		cfg.UnfinishedItems.Insert( "undersluggrenadem4" );
		cfg.UnfinishedItems.Insert( "groza" );
		cfg.UnfinishedItems.Insert( "trumpet" );
		cfg.UnfinishedItems.Insert( "lawbase" );
		cfg.UnfinishedItems.Insert( "law" );
		cfg.UnfinishedItems.Insert( "rpg7base" );
		cfg.UnfinishedItems.Insert( "rpg7" );
		cfg.UnfinishedItems.Insert( "dartgun" );
		cfg.UnfinishedItems.Insert( "shockpistol" );
		cfg.UnfinishedItems.Insert( "shockpistol_black" );
		cfg.UnfinishedItems.Insert( "fnx45_arrow" );
		cfg.UnfinishedItems.Insert( "makarovpb" );
		cfg.UnfinishedItems.Insert( "mp133shotgun_pistolgrip" );
		cfg.UnfinishedItems.Insert( "largetentbackpack" );
		cfg.UnfinishedItems.Insert( "leatherbelt_natural" );
		cfg.UnfinishedItems.Insert( "leatherbelt_beige" );
		cfg.UnfinishedItems.Insert( "leatherbelt_brown" );
		cfg.UnfinishedItems.Insert( "leatherbelt_black" );
		cfg.UnfinishedItems.Insert( "leatherknifesheath" );

		cfg.RestrictedPatterns.Insert( "placing" );
		cfg.RestrictedPatterns.Insert( "debug" );

		cfg.RestrictedStartPatterns.Insert( "bldr_" );
		cfg.RestrictedStartPatterns.Insert( "land_" );
		cfg.RestrictedStartPatterns.Insert( "staticobj_" );

		return cfg;
	}

	static JMSpawnerConfig Load()
	{
		if ( !FileExist( JMConstants.FILE_SPAWNER_CONFIG ) )
		{
			JMSpawnerConfig defaults = Defaults();
			defaults.Save();
			return defaults;
		}

		JMSpawnerConfig cfg = new JMSpawnerConfig();
		JsonFileLoader< JMSpawnerConfig >.JsonLoadFile( JMConstants.FILE_SPAWNER_CONFIG, cfg );
		return cfg;
	}

	void Save()
	{
		string dir = JMConstants.DIR_COT;
		if ( !FileExist( dir ) )
			MakeDirectory( dir );

		JsonFileLoader< JMSpawnerConfig >.JsonSaveFile( JMConstants.FILE_SPAWNER_CONFIG, this );
	}
}
