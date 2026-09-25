//! The roster's vehicle type filter: the toolbar button, its dropdown, and the bitmask of JMVT_* types the roster and the map are showing.
class JMVehiclesTypeFilter
{
	protected JMVehiclesForm m_Form;

	//! Type filter: the toolbar button, its dropdown, and the bitmask of the
	//! JMVT_* types the roster and the map are currently showing.
	protected UIActionImageButton m_FilterButton;
	protected ref UIActionFilterMenu m_FilterMenu;
	protected int m_ShownTypes;

	//! Anything whose type carries none of the known bits - modded vehicles the
	//! module could not classify. Its own row so it can be hidden on its own.
	protected bool m_FilterShowOther = true;
	static const string TYPE_FILTER_CAR   = "type_car";
	static const string TYPE_FILTER_BOAT  = "type_boat";
	static const string TYPE_FILTER_HELI  = "type_heli";
	static const string TYPE_FILTER_PLANE = "type_plane";
	static const string TYPE_FILTER_BIKE  = "type_bike";
	static const string TYPE_FILTER_OTHER = "type_other";

	void JMVehiclesTypeFilter( JMVehiclesForm form )
	{
		m_Form = form;
		Reset();
	}

	protected bool IsShown( int typeBit )
	{
		return ( m_ShownTypes & typeBit ) != 0;
	}

	//! Every filter starts open, so the form shows the whole server the moment it
	//! is opened. Assigned here rather than at the declaration: JMVT_ALL is a
	//! global const folded from other global consts, and a member initialiser that
	//! resolved it to 0 would open the form with every vehicle type hidden and no
	//! visible reason why.
	void Reset()
	{
		m_ShownTypes      = JMVT_ALL;
		m_FilterShowOther = true;
	}

	//! Creates the toolbar button as a child of parent and adds it to the flex row.
	void Build( Widget parent, UIActionFlexRow row )
	{
		m_FilterButton = UIActionManager.CreateIconButton( parent, JMConstants.Lucide( "list-filter" ), this, "OnClick_TypeFilters" );
		if ( m_FilterButton )
		{
			m_FilterButton.SetFixedSize( JMVehiclesForm.TOOLBAR_CONTROL_PX, JMVehiclesForm.TOOLBAR_CONTROL_PX );
			m_FilterButton.SetTooltip( "#STR_COT_VEHICLES_FILTER_THE_LIST_AND_THE_MAP" );
			row.Add( m_FilterButton );
		}
	}

	// -------------------------------------------------------------------------
	//  Vehicle type filter
	// -------------------------------------------------------------------------

	//! Open the type filter dropdown under the toolbar button. A context menu
	//! rather than a strip of checkboxes: it floats over the roster instead of
	//! costing the toolbar a row it only needs while being changed.
	//! Mods add rows with JMFilterRegistry.Register( JMFilterRegistry.VEHICLES, ... ).
	void OnClick_TypeFilters( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_FilterButton )
		{
			Error("[JMVehiclesTypeFilter] OnClick_TypeFilters failed: m_FilterButton is null!");
			return;
		}

		if ( !m_FilterMenu )
		{
			m_FilterMenu = UIActionManager.CreateOverlayFilterMenu( m_Form, m_FilterButton.GetLayoutRoot(), JMFilterRegistry.VEHICLES );

			if ( !m_FilterMenu )
			{
				Error("[JMVehiclesTypeFilter] OnClick_TypeFilters failed: Could not create UIActionFilterMenu (m_FilterMenu is null)!");
				return;
			}

			m_FilterMenu.AddPage( "root", this, "BuildFilterRootPage", "", false, "", this, "OnFilterRootChange" );
		}

		m_FilterMenu.ToggleAt( m_FilterButton.GetLayoutRoot() );
	}

	//! An active type is normal text, a hidden one is dimmed - the row IS the
	//! checkbox, so there is no separate tick to keep in step.
	void BuildFilterRootPage( UIActionFilterMenu menu )
	{
		menu.AddToggleRow( TYPE_FILTER_CAR,   "Cars",         IsShown( JMVT_CAR ),        JMConstants.Lucide( "car" ) );
		menu.AddToggleRow( TYPE_FILTER_BOAT,  "Boats",        IsShown( JMVT_BOAT ),       JMConstants.Lucide( "sailboat" ) );
		menu.AddToggleRow( TYPE_FILTER_HELI,  "Helicopters",  IsShown( JMVT_HELICOPTER ), JMConstants.Lucide( "helicopter" ) );
		menu.AddToggleRow( TYPE_FILTER_PLANE, "Planes",       IsShown( JMVT_PLANE ),      JMConstants.Lucide( "plane" ) );
		menu.AddToggleRow( TYPE_FILTER_BIKE,  "Motorbikes",   IsShown( JMVT_BIKE ),       JMConstants.Lucide( "bike" ) );
		menu.AddToggleRow( TYPE_FILTER_OTHER, "Unclassified", m_FilterShowOther,              JMConstants.Lucide( "circle-help" ) );
	}

	void OnFilterRootChange( string id )
	{
		if ( id == TYPE_FILTER_CAR )
			m_ShownTypes = m_ShownTypes ^ JMVT_CAR;
		else if ( id == TYPE_FILTER_BOAT )
			m_ShownTypes = m_ShownTypes ^ JMVT_BOAT;
		else if ( id == TYPE_FILTER_HELI )
			m_ShownTypes = m_ShownTypes ^ JMVT_HELICOPTER;
		else if ( id == TYPE_FILTER_PLANE )
			m_ShownTypes = m_ShownTypes ^ JMVT_PLANE;
		else if ( id == TYPE_FILTER_BIKE )
			m_ShownTypes = m_ShownTypes ^ JMVT_BIKE;
		else if ( id == TYPE_FILTER_OTHER )
			m_FilterShowOther = !m_FilterShowOther;
		else
			return;

		m_Form.LoadVehicles();
	}

	//! m_VehicleType is a bitmask, and a vehicle can legitimately carry more
	//! than one bit, so a match on any shown bit keeps it. A vehicle carrying
	//! none of the known bits is unclassified and answers to its own toggle.
	bool Passes( JMVehicleMetaData vehicle )
	{
		if ( ( vehicle.m_VehicleType & JMVT_ALL ) == 0 )
			return m_FilterShowOther;

		return ( vehicle.m_VehicleType & m_ShownTypes ) != 0;
	}
}
