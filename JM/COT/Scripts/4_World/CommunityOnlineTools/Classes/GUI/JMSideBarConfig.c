// =============================================================================
//  JMSideBarConfig.c
//  Sidebar category display configuration.
//  Add, remove or rename categories and their icons here.
// =============================================================================

class JMSideBarConfig
{
	// Preferred display order for categories.
	// Categories not listed here are appended alphabetically at the end.
	static void GetCategoryOrder( out array< string > order )
	{
		order.Insert( "Players" );
		order.Insert( "World"   );
		order.Insert( "Items"   );
		order.Insert( "Other"   );
	}

	// Returns the icon path for a given category name.
	// Unknown categories fall back to the settings icon.
	static string GetCategoryIcon( string catName )
	{
		if ( catName == "Players" ) return JMIcon_User;
		if ( catName == "World"   ) return JMIcon_PlanetEarth;
		if ( catName == "Items"   ) return JMIcon_Container;

		return JMIcon_Setting;
	}
}
