class JMItemStatsModule: JMRenderableModuleBase
{
	//! Items that are unfinished may not work or show properly or may even cause the game to segfault
	protected ref array< string > m_UnfinishedItems =
	{
		"quickiebow",
		"recurvebow",
		"gp25base",
		"gp25",
		"gp25_standalone",
		"m203base",
		"m203",
		"m203_standalone",
		"red9",
		"pvcbow",
		"m249",
		"undersluggrenadem4",
		"groza",
		"trumpet",
		"lawbase",
		"law",
		"rpg7base",
		"rpg7",
		"dartgun",
		"shockpistol",
		"shockpistol_black",
		"fnx45_arrow",
		"makarovpb",
		"mp133shotgun_pistolgrip",

		"largetentbackpack",
		"leatherbelt_natural",
		"leatherbelt_beige",
		"leatherbelt_brown",
		"leatherbelt_black",
		"leatherknifesheath"
	};
	protected ref array< string > m_RestrictedClassNames =
	{
		"placing",
		"debug",
		"bldr_",
		"land_",
		"staticobj_"
	};

	bool IsExcludedClassName( string className )
	{
		if ( m_UnfinishedItems.Find( className ) > -1 )
			return true;

		foreach ( string restrictedClassName: m_RestrictedClassNames )
		{
			if ( className.Contains( restrictedClassName ) )
				return true;
		}

		return false;
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "ItemStats";
		info.WebhookTitle = "ItemStats Module";
		info.Layout = "JM/COT/GUI/layouts/ItemStatsForm.layout";
		info.ViewPermission = JMConstants.PERM_ITEMSTATS_VIEW;
	}

}
