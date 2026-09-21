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
	//! Category ids. A module returns one from GetCategory(); anything else is a category of its
	//! own, listed after these alphabetically with the settings icon.
	static const string CATEGORY_PLAYERS   = "Players";
	static const string CATEGORY_WORLD     = "World";
	static const string CATEGORY_ITEMS     = "Items";
	static const string CATEGORY_VEHICLES  = "Vehicles";
	static const string CATEGORY_EVENTS    = "Events";
	static const string CATEGORY_SERVER    = "Server";
	static const string CATEGORY_EXPANSION = "Expansion";
	static const string CATEGORY_OTHER     = "Other";

	// Returns the localized display name for a given category id.
	static string GetCategoryDisplayName( string catName )
	{
		if ( catName == CATEGORY_PLAYERS ) return "#STR_COT_CATEGORY_PLAYERS";
		if ( catName == CATEGORY_WORLD ) return "#STR_COT_CATEGORY_WORLD";
		if ( catName == CATEGORY_ITEMS ) return "#STR_COT_CATEGORY_ITEMS";
		if ( catName == CATEGORY_VEHICLES ) return "#STR_COT_CATEGORY_VEHICLES";
		if ( catName == CATEGORY_EVENTS ) return "#STR_COT_CATEGORY_EVENTS";
		if ( catName == CATEGORY_SERVER ) return "#STR_COT_CATEGORY_SERVER";
		if ( catName == CATEGORY_EXPANSION ) return "#STR_COT_CATEGORY_EXPANSION";
		if ( catName == CATEGORY_OTHER ) return "#STR_COT_CATEGORY_OTHER";

		return catName;
	}

	// Returns the icon path for a given category name.
	// Unknown categories fall back to the settings icon.
	static string GetCategoryIcon( string catName )
	{
		if ( catName == CATEGORY_PLAYERS ) return JMConstants.Lucide( "users"    );
		if ( catName == CATEGORY_WORLD ) return JMConstants.Lucide( "earth"    );
		if ( catName == CATEGORY_ITEMS ) return JMConstants.Lucide( "boxes"    );
		if ( catName == CATEGORY_VEHICLES ) return JMConstants.Lucide( "car"      );
		if ( catName == CATEGORY_EVENTS ) return JMConstants.Lucide( "siren"    );
		if ( catName == CATEGORY_SERVER ) return JMConstants.Lucide( "server"   );
		if ( catName == CATEGORY_EXPANSION ) return JMConstants.Lucide( "puzzle"   );

		return JMConstants.Lucide( "settings" );
	}

	// Preferred display order for categories.
	// Categories not listed here are appended alphabetically at the end.
	static void GetCategoryOrder( out array< string > order )
	{
		order.Insert( CATEGORY_PLAYERS );
		order.Insert( CATEGORY_WORLD );
		order.Insert( CATEGORY_ITEMS );
		order.Insert( CATEGORY_VEHICLES );
		order.Insert( CATEGORY_EVENTS );
		order.Insert( CATEGORY_SERVER );
		order.Insert( CATEGORY_EXPANSION );
		order.Insert( CATEGORY_OTHER );
	}
}
