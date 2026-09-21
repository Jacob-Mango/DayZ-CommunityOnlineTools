//! The Teleport form's filter dropdown: which location categories, player roles, vehicle types
//! and map layers (markers, heli crashes, toxic zones) are shown, and the state behind it.
//! The form asks this class whether a row or marker survives; sub-mods add their own rows
//! through JMFilterRegistry.TELEPORT.
class JMTeleportFilter
{
	protected JMTeleportForm m_Form;

	//! Top-level filter menu rows with sub-menus - also used as the
	//! UIActionFilterMenu page keys they drill down into.
	static const string SUB_LOCATIONS        = "*SUB_LOCATIONS*";
	static const string SUB_VEHICLES         = "*SUB_VEHICLES*";
	static const string SUB_PLAYERS          = "*SUB_PLAYERS*";
	static const string FILTER_PAGE_ROOT     = "*FILTER_ROOT*";

	//! Main menu toggles.
	static const string FILTER_MAP_MARKERS   = "*FILTER_MAP_MARKERS*";
	static const string FILTER_HELI_CRASHES  = "*FILTER_HELI_CRASHES*";
	static const string FILTER_TOXIC_ZONES   = "*FILTER_TOXIC_ZONES*";

	//! Vehicle filter sub-entries.
	static const string FILTER_VEH_CAR       = "*VEH_CAR*";
	static const string FILTER_VEH_BOAT      = "*VEH_BOAT*";
	static const string FILTER_VEH_HELI      = "*VEH_HELI*";
	static const string FILTER_VEH_PLANE     = "*VEH_PLANE*";
	static const string FILTER_VEH_BIKE      = "*VEH_BIKE*";
	protected UIActionImageButton m_FilterButton;
	protected ref UIActionFilterMenu m_FilterMenu;

	//! Categories currently switched OFF, rather than the one that is on: a
	//! new category shows up the moment it exists, which is what an admin who
	//! has hidden two of eight expects.
	protected ref TStringArray m_HiddenCategories = new TStringArray;
	protected bool m_ShowPlayers = true;
	protected ref TStringArray m_HiddenPlayerRoles = new TStringArray;
	protected bool m_ShowMapMarkers = true;
	protected bool m_ShowHeliCrashes;
	protected bool m_ShowToxicZones;

	//! Locations start hidden - the first RefreshCategories() call fills
	//! m_HiddenCategories with every real category it finds, then leaves it
	//! alone so a later toggle isn't overwritten on the next refresh.
	protected bool m_CategoriesInitialized;

	//! Bitmask of JMVT_* vehicle types currently shown on the overlay - 0
	//! means the whole vehicle layer is off, same as the old plain bool did,
	//! but broken down per type the way Vehicle Manager's own filter is.
	protected int m_VehicleTypeFilter = JMVT_ALL;

	void JMTeleportFilter( JMTeleportForm form )
	{
		m_Form = form;
	}

	int GetVehicleTypeFilter()
	{
		return m_VehicleTypeFilter;
	}

	bool IsCategoryHidden( string category )
	{
		if ( !m_HiddenCategories )
			return false;

		return m_HiddenCategories.Find( category ) >= 0;
	}

	//! Shared by AppendPlayerRows and RefreshPlayerMarkers, so a role hidden on
	//! the map is hidden here too - the two would otherwise be free to disagree
	//! about who "the visible players" are.
	bool IsPlayerVisibleByRole( JMPlayerInstance player )
	{
		if ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 )
			return true;

		array< string > playerRoles = player.GetRoles();

		if ( playerRoles && playerRoles.Count() > 0 )
		{
			for ( int r = 0; r < playerRoles.Count(); r++ )
			{
				if ( !IsRoleHidden( playerRoles[r] ) )
					return true;
			}

			return false;
		}

		return !IsRoleHidden( "Default" ) && !IsRoleHidden( "Player" ) && !IsRoleHidden( "Everyone" );
	}

	bool IsRoleHidden( string roleName )
	{
		if ( !m_HiddenPlayerRoles )
			return false;

		return m_HiddenPlayerRoles.Find( roleName ) >= 0;
	}

	bool IsVehicleTypeShown( int typeBit )
	{
		return ( m_VehicleTypeFilter & typeBit ) != 0;
	}

	//! The filter button of the search row; the menu opens under it.
	void SetButton( UIActionImageButton button )
	{
		m_FilterButton = button;
	}

	// =========================================================================
	//  Category filter
	//
	//  A popup hung off the filter button, the way the object spawner and the
	//  vehicle form do it: needed for one click per search and costing nothing
	//  in between, where a selector parked on the form costs a band of the left
	//  column forever.
	//
	//  One UIActionFilterMenu (see its class header), not a menu plus a
	//  second UIActionContextMenu for the drill-down - two live overlay
	//  menus sharing one anchor is what used to leave the drill-down dead
	//  to clicks and hover until the whole COT window was closed and
	//  reopened.
	// =========================================================================

	void OnClick_CategoryFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterButton )
			return;

		if ( !m_FilterMenu )
		{
			m_FilterMenu = UIActionManager.CreateOverlayFilterMenu( m_Form, m_FilterButton.GetLayoutRoot(), JMFilterRegistry.TELEPORT );

			if ( !m_FilterMenu )
				return;

			m_FilterMenu.AddPage( FILTER_PAGE_ROOT, this, "BuildFilterRootPage", "", false, "", this, "OnFilterRootChange" );
			m_FilterMenu.AddPage( SUB_LOCATIONS, this, "BuildFilterLocationsPage", FILTER_PAGE_ROOT, true, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", this, "OnFilterLocationsChange" );
			m_FilterMenu.AddPage( SUB_VEHICLES, this, "BuildFilterVehiclesPage", FILTER_PAGE_ROOT, true, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", this, "OnFilterVehiclesChange" );
			m_FilterMenu.AddPage( SUB_PLAYERS, this, "BuildFilterPlayersPage", FILTER_PAGE_ROOT, true, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", this, "OnFilterPlayersChange" );
		}

		m_FilterMenu.ToggleAt( m_FilterButton.GetLayoutRoot() );
	}

	void BuildFilterRootPage( UIActionFilterMenu menu )
	{
		bool allCategoriesShown = ( !m_HiddenCategories || m_HiddenCategories.Count() == 0 );
		bool anyVehicleTypeShown = ( m_VehicleTypeFilter != 0 );
		bool allPlayersShown = m_ShowPlayers && ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 );

		menu.AddRow( SUB_LOCATIONS, "#STR_COT_TELEPORT_MODULE_MENU_LOCATIONS", JMConstants.Lucide( "map-pin" ), UIActionFilterMenu.ToggleTextColor( allCategoriesShown ), true, SUB_LOCATIONS );
		menu.AddRow( SUB_VEHICLES,  "#STR_COT_TELEPORT_MODULE_MENU_VEHICLES",  JMConstants.Lucide( "car"     ), UIActionFilterMenu.ToggleTextColor( anyVehicleTypeShown ), JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ), SUB_VEHICLES );
		menu.AddRow( SUB_PLAYERS,   "#STR_COT_TELEPORT_MODULE_MENU_PLAYERS",   JMConstants.Lucide( "user"    ), UIActionFilterMenu.ToggleTextColor( allPlayersShown ), JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS ), SUB_PLAYERS );

		menu.AddRow( FILTER_MAP_MARKERS,  "#STR_COT_TELEPORT_MODULE_MENU_MAP_MARKERS",  UIActionFilterMenu.CheckIcon( m_ShowMapMarkers  ), UIActionFilterMenu.ToggleTextColor( m_ShowMapMarkers  ) );
		menu.AddRow( FILTER_HELI_CRASHES, "#STR_COT_TELEPORT_MODULE_MENU_HELI_CRASHES", UIActionFilterMenu.CheckIcon( m_ShowHeliCrashes ), UIActionFilterMenu.ToggleTextColor( m_ShowHeliCrashes ) );
		menu.AddRow( FILTER_TOXIC_ZONES,  "#STR_COT_TELEPORT_MODULE_MENU_TOXIC_ZONES",  UIActionFilterMenu.CheckIcon( m_ShowToxicZones  ), UIActionFilterMenu.ToggleTextColor( m_ShowToxicZones  ) );
	}

	//! Only ever reached for the three flat toggles - the three drill-down
	//! rows above switch page instead, intercepted by UIActionFilterMenu
	//! before this runs.
	void OnFilterRootChange( string id )
	{
		if ( id == FILTER_MAP_MARKERS )
			m_ShowMapMarkers = !m_ShowMapMarkers;
		else if ( id == FILTER_HELI_CRASHES )
			m_ShowHeliCrashes = !m_ShowHeliCrashes;
		else if ( id == FILTER_TOXIC_ZONES )
			m_ShowToxicZones = !m_ShowToxicZones;
		else
			return;

		m_Form.ApplyOverlays();
	}

	void BuildFilterLocationsPage( UIActionFilterMenu menu )
	{
		if ( !m_Form.GetCategories() )
			return;

		for ( int i = 0; i < m_Form.GetCategories().Count(); i++ )
		{
			if ( m_Form.GetCategories()[i] == JMTeleportForm.CATEGORY_ALL )
				continue;

			string categoryName = m_Form.GetCategories()[i];
			bool shown = !IsCategoryHidden( categoryName );
			string categoryIcon = UIActionFilterMenu.CheckIcon( shown );
			int categoryColor = UIActionFilterMenu.ToggleTextColor( shown );
			menu.AddRow( categoryName, categoryName, categoryIcon, categoryColor );
		}
	}

	void OnFilterLocationsChange( string id )
	{
		if ( id == UIActionFilterMenu.ID_TOGGLE_ALL )
		{
			bool allShown = ( !m_HiddenCategories || m_HiddenCategories.Count() == 0 );
			if ( allShown )
				HideAllCategories();
			else
				ShowAllCategories();
		}
		else
		{
			ToggleCategory( id );
		}

		UpdateFilterTooltip();
		m_Form.RebuildList();
	}

	void BuildFilterVehiclesPage( UIActionFilterMenu menu )
	{
		bool canSeeVehicles = JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW );

		menu.AddRow( FILTER_VEH_CAR,   "#STR_COT_TELEPORT_MODULE_VEH_CARS",        UIActionFilterMenu.CheckIcon( IsVehicleTypeShown( JMVT_CAR ) ),        UIActionFilterMenu.ToggleTextColor( IsVehicleTypeShown( JMVT_CAR ) ),        canSeeVehicles );
		menu.AddRow( FILTER_VEH_BOAT,  "#STR_COT_TELEPORT_MODULE_VEH_BOATS",       UIActionFilterMenu.CheckIcon( IsVehicleTypeShown( JMVT_BOAT ) ),       UIActionFilterMenu.ToggleTextColor( IsVehicleTypeShown( JMVT_BOAT ) ),       canSeeVehicles );
		menu.AddRow( FILTER_VEH_HELI,  "#STR_COT_TELEPORT_MODULE_VEH_HELICOPTERS", UIActionFilterMenu.CheckIcon( IsVehicleTypeShown( JMVT_HELICOPTER ) ), UIActionFilterMenu.ToggleTextColor( IsVehicleTypeShown( JMVT_HELICOPTER ) ), canSeeVehicles );
		menu.AddRow( FILTER_VEH_PLANE, "#STR_COT_TELEPORT_MODULE_VEH_PLANES",      UIActionFilterMenu.CheckIcon( IsVehicleTypeShown( JMVT_PLANE ) ),      UIActionFilterMenu.ToggleTextColor( IsVehicleTypeShown( JMVT_PLANE ) ),      canSeeVehicles );
		menu.AddRow( FILTER_VEH_BIKE,  "#STR_COT_TELEPORT_MODULE_VEH_BIKES",       UIActionFilterMenu.CheckIcon( IsVehicleTypeShown( JMVT_BIKE ) ),       UIActionFilterMenu.ToggleTextColor( IsVehicleTypeShown( JMVT_BIKE ) ),       canSeeVehicles );
	}

	void OnFilterVehiclesChange( string id )
	{
		if ( id == UIActionFilterMenu.ID_TOGGLE_ALL )
		{
			if ( m_VehicleTypeFilter == 0 )
				m_VehicleTypeFilter = JMVT_ALL;
			else
				m_VehicleTypeFilter = 0;
		}
		else
		{
			ToggleVehicleType( id );
		}

		m_Form.ApplyOverlays();
	}

	void BuildFilterPlayersPage( UIActionFilterMenu menu )
	{
		bool canSeePlayers = JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS );

		array< JMRole > roles = new array< JMRole >;
		GetPermissionsManager().GetRolesAsList( roles );

		for ( int i = 0; i < roles.Count(); i++ )
		{
			JMRole role = roles[i];
			if ( !role )
				continue;

			string roleName = role.Name;
			bool roleShown = m_ShowPlayers && !IsRoleHidden( roleName );

			menu.AddRow( roleName, roleName, UIActionFilterMenu.CheckIcon( roleShown ), UIActionFilterMenu.ToggleTextColor( roleShown ), canSeePlayers );
		}
	}

	void OnFilterPlayersChange( string id )
	{
		if ( id == UIActionFilterMenu.ID_TOGGLE_ALL )
		{
			bool allPlayersOn = m_ShowPlayers && ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 );

			if ( allPlayersOn )
			{
				m_ShowPlayers = false;
				HideAllPlayerRoles();
			}
			else
			{
				m_ShowPlayers = true;
				m_HiddenPlayerRoles.Clear();
			}
		}
		else
		{
			m_ShowPlayers = true;
			TogglePlayerRole( id );
		}

		m_Form.ApplyOverlays();
	}

	protected void TogglePlayerRole( string roleName )
	{
		if ( !m_HiddenPlayerRoles )
			m_HiddenPlayerRoles = new TStringArray;

		int idx = m_HiddenPlayerRoles.Find( roleName );
		if ( idx >= 0 )
		{
			m_HiddenPlayerRoles.Remove( idx );
			return;
		}

		m_HiddenPlayerRoles.Insert( roleName );
	}

	protected void HideAllPlayerRoles()
	{
		m_HiddenPlayerRoles = new TStringArray;

		array< JMRole > roles = new array< JMRole >;
		GetPermissionsManager().GetRolesAsList( roles );

		for ( int i = 0; i < roles.Count(); i++ )
		{
			if ( roles[i] )
				m_HiddenPlayerRoles.Insert( roles[i].Name );
		}
	}

	protected int VehicleTypeBitFor( string id )
	{
		if ( id == FILTER_VEH_CAR )
			return JMVT_CAR;

		if ( id == FILTER_VEH_BOAT )
			return JMVT_BOAT;

		if ( id == FILTER_VEH_HELI )
			return JMVT_HELICOPTER;

		if ( id == FILTER_VEH_PLANE )
			return JMVT_PLANE;

		if ( id == FILTER_VEH_BIKE )
			return JMVT_BIKE;

		return 0;
	}

	//! Flip one vehicle type's bit. False (and no change) if `id` is not one
	//! of the per-type rows.
	protected bool ToggleVehicleType( string id )
	{
		int bit = VehicleTypeBitFor( id );

		if ( bit == 0 )
			return false;

		m_VehicleTypeFilter = m_VehicleTypeFilter ^ bit;
		return true;
	}

	protected void HideAllCategories()
	{
		m_HiddenCategories = new TStringArray;

		if ( !m_Form.GetCategories() )
			return;

		for ( int i = 0; i < m_Form.GetCategories().Count(); i++ )
		{
			if ( m_Form.GetCategories()[i] == JMTeleportForm.CATEGORY_ALL )
				continue;

			m_HiddenCategories.Insert( m_Form.GetCategories()[i] );
		}
	}

	protected void ShowAllCategories()
	{
		m_HiddenCategories = new TStringArray;
	}

	protected void ToggleCategory( string category )
	{
		if ( !m_HiddenCategories )
			m_HiddenCategories = new TStringArray;

		int at = m_HiddenCategories.Find( category );

		if ( at >= 0 )
		{
			m_HiddenCategories.Remove( at );
			return;
		}

		m_HiddenCategories.Insert( category );
	}

	//! The button is the only thing showing that a filter is on at all, so it
	//! has to say how many rows are being held back.
	void UpdateFilterTooltip()
	{
		int hidden = 0;

		if ( !m_FilterButton )
			return;

		if ( m_HiddenCategories )
			hidden = m_HiddenCategories.Count();

		if ( hidden == 0 )
			m_FilterButton.SetTooltip( "#STR_COT_TELEPORT_MODULE_FILTERS_TOOLTIP" );
		else
			m_FilterButton.SetTooltip( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_FILTERS_HIDDEN_PREFIX" ) + hidden + Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_FILTERS_HIDDEN_SUFFIX" ) );
	}

	void CloseMenu()
	{
		if ( m_FilterMenu )
			m_FilterMenu.Close();
	}

	bool ShowsPlayers()
	{
		return m_ShowPlayers;
	}

	bool ShowsMapMarkers()
	{
		return m_ShowMapMarkers;
	}

	bool ShowsHeliCrashes()
	{
		return m_ShowHeliCrashes;
	}

	bool ShowsToxicZones()
	{
		return m_ShowToxicZones;
	}

	//! Called after the form rebuilt its category list. The first time, the hidden set is
	//! seeded with every real category (see m_CategoriesInitialized).
	void OnCategoriesRefreshed()
	{
		TStringArray categories = m_Form.GetCategories();

		if ( !m_CategoriesInitialized )
		{
			m_CategoriesInitialized = true;

			m_HiddenCategories = new TStringArray;
			for ( int j = 0; j < categories.Count(); j++ )
			{
				if ( categories[j] == JMTeleportForm.CATEGORY_ALL )
					continue;

				m_HiddenCategories.Insert( categories[j] );
			}
		}

		//! The hidden set is deliberately not pruned against the categories
		//! that still exist. A category emptied by a delete and recreated by
		//! the next save is the same category to the admin who hid it.
		UpdateFilterTooltip();
	}
}
