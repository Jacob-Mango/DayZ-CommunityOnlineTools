//! The one place a mod adds rows to COT's filter menus.
//!
//!     JMFilterRegistry.Register( JMFilterRegistry.PLAYERS, "only_admins", "Only Admins", "shield", 0, this, "OnFilterAdmins", "IsFilterAdmins" );
//!
//! Every filter menu built with UIActionFilterMenu (Player list, Vehicles, Teleport, Object Spawner, ESP objects,
//! Loot Analysis categories) names the scope it serves with SetRegistryScope(); rows registered for that scope
//! are appended to the menu's root page and their callbacks run when clicked. Registering the same id again
//! replaces the entry, so a form that registers from OnCreate() can be created more than once per session.
//!
//! Callback shapes:   void OnFilterAdmins( string id )    - the row was clicked, flip your state here
//!                    bool IsFilterAdmins()               - optional; makes the row a checkbox
class JMFilterRegistry
{
	//! Scopes - one per filter menu. Any other string works too, for a menu of your own.
	static const string PLAYERS  = "players";
	static const string VEHICLES = "vehicles";
	static const string TELEPORT = "teleport";
	static const string OBJECTS  = "objects";
	static const string ESP      = "esp";
	static const string ITEMS    = "items";
	protected static ref map< string, ref array< ref JMFilterEntry > > s_Filters = new map< string, ref array< ref JMFilterEntry > >;

	//! Never null; empty when nothing is registered for `scope`.
	static array< ref JMFilterEntry > Get( string scope )
	{
		array< ref JMFilterEntry > entries;

		if ( !s_Filters.Find( scope, entries ) )
		{
			entries = new array< ref JMFilterEntry >;
			s_Filters.Insert( scope, entries );
		}

		return entries;
	}

	static array< ref JMFilterEntry > GetESPFilters()
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.GetESPFilters() is deprecated. Please use Get( JMFilterRegistry.ESP )." );
		return Get( ESP );
	}

	static array< ref JMFilterEntry > GetItemFilters()
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.GetItemFilters() is deprecated. Please use Get( JMFilterRegistry.ITEMS )." );
		return Get( ITEMS );
	}

	static array< ref JMFilterEntry > GetObjectFilters()
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.GetObjectFilters() is deprecated. Please use Get( JMFilterRegistry.OBJECTS )." );
		return Get( OBJECTS );
	}

	static array< ref JMFilterEntry > GetPlayerFilters()
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.GetPlayerFilters() is deprecated. Please use Get( JMFilterRegistry.PLAYERS )." );
		return Get( PLAYERS );
	}

	static array< ref JMFilterEntry > GetTeleportFilters()
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.GetTeleportFilters() is deprecated. Please use Get( JMFilterRegistry.TELEPORT )." );
		return Get( TELEPORT );
	}

	static void Register( string scope, string id, string label, string icon = "", int color = 0, Class target = null, string callback = "", string stateCallback = "" )
	{
		array< ref JMFilterEntry > entries = Get( scope );
		JMFilterEntry entry = new JMFilterEntry( id, label, icon, color, target, callback, stateCallback );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			if ( entries[i].m_Id == id )
			{
				entries[i] = entry;
				return;
			}
		}

		entries.Insert( entry );
	}

	//! The entry for `id` in `scope`, or null.
	static JMFilterEntry Find( string scope, string id )
	{
		array< ref JMFilterEntry > entries = Get( scope );

		foreach ( JMFilterEntry entry : entries )
		{
			if ( entry.m_Id == id )
				return entry;
		}

		return null;
	}

	// -- DEPRECATED per-scope spellings - use Register( scope, ... ) / Get( scope ) --

	static void RegisterPlayerFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.RegisterPlayerFilter() is deprecated. Please use Register( JMFilterRegistry.PLAYERS, ... )." );
		Register( PLAYERS, id, label, icon, color, target, callback );
	}

	static void RegisterItemFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.RegisterItemFilter() is deprecated. Please use Register( JMFilterRegistry.ITEMS, ... )." );
		Register( ITEMS, id, label, icon, color, target, callback );
	}

	static void RegisterObjectFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.RegisterObjectFilter() is deprecated. Please use Register( JMFilterRegistry.OBJECTS, ... )." );
		Register( OBJECTS, id, label, icon, color, target, callback );
	}

	static void RegisterTeleportFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.RegisterTeleportFilter() is deprecated. Please use Register( JMFilterRegistry.TELEPORT, ... )." );
		Register( TELEPORT, id, label, icon, color, target, callback );
	}

	static void RegisterESPFilter( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		JMDeprecated.WarnOnce( null, "JMFilterRegistry.RegisterESPFilter() is deprecated. Please use Register( JMFilterRegistry.ESP, ... )." );
		Register( ESP, id, label, icon, color, target, callback );
	}
}
