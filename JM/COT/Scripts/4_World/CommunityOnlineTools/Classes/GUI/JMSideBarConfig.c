// =============================================================================
//  JMSideBarConfig.c
//  Sidebar category display configuration.
//  Add, remove or rename categories and their icons here.
//
//  Icons come from the Lucide set under GUI/textures/icons/lucide/ - a single
//  line-art family, so a column of category tiles reads as one set rather than
//  the mix of weights the old game-icons.net picks produced. Module tiles are
//  drawn from the same family (see each module GetIconName override).
// =============================================================================

class JMSideBarConfig
{
	// Preferred display order for categories.
	// Categories not listed here are appended alphabetically at the end.
	static void GetCategoryOrder( out array< string > order )
	{
		order.Insert( "Players"   );
		order.Insert( "World"     );
		order.Insert( "Items"     );
		order.Insert( "Vehicles"  );
		order.Insert( "Events"    );
		order.Insert( "Server"    );
		order.Insert( "Expansion" );
		order.Insert( "Other"     );
	}

	// Returns the icon path for a given category name.
	// Unknown categories fall back to the settings icon.
	static string GetCategoryIcon( string catName )
	{
		if ( catName == "Players"   ) return JMConstants.Lucide( "users"    );
		if ( catName == "World"     ) return JMConstants.Lucide( "earth"    );
		if ( catName == "Items"     ) return JMConstants.Lucide( "boxes"    );
		if ( catName == "Vehicles"  ) return JMConstants.Lucide( "car"      );
		if ( catName == "Events"    ) return JMConstants.Lucide( "siren"    );
		if ( catName == "Server"    ) return JMConstants.Lucide( "server"   );
		if ( catName == "Expansion" ) return JMConstants.Lucide( "puzzle"   );

		return JMConstants.Lucide( "settings" );
	}
}
