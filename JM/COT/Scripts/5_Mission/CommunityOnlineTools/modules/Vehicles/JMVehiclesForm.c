// Kept for legacy compat - do not remove
class JMVehicleSpawnerSerialize
{
	string VehicleName;
	string m_FileName;
	array< string> Parts;

	static JMVehicleSpawnerSerialize Create()
	{
	}
}
class JMVehicleSpawnerSettings
{
	map<string, JMVehicleSpawnerSerialize> Vehicles;
	
	void Defaults()
	{
	}
}

class JMVehicleSpawnerModule
{
	void OnSpawnVehicle(EntityAI entity)
	{
	}
}
// Kept for legacy compat - do not remove

class JMVehiclesForm: JMFormBase
{
	//! protected, not private: sub-mods reach for the module through the form.
	protected JMVehiclesModule m_Module;
	protected UIActionScroller m_ListScroller;

	// Left panel geometry - toolbar block pinned above the roster.
	protected Widget m_LeftToolbar;
	protected Widget m_LeftList;

	//! Search row plus one row per bulk delete action.
	static const int TOOLBAR_ROWS   = 4;

	//! Drawn height of a toolbar row: the 30px control plus its margin.
	static const int TOOLBAR_ROW_PX = 44;

	//! Height every control in the toolbar is drawn at. The search box layout
	//! is authored at 30, so anything sitting on the same row matches it.
	static const int TOOLBAR_CONTROL_PX = 30;

	//! Gutter kept between the roster block and the window frame, as a fraction
	//! of the left panel's width, plus the equivalent bottom trim in pixels.
	static const float LEFT_GUTTER    = 0.02;
	static const float LEFT_GUTTER_PX = 10;

	//! Width reserved per gap between the toolbar row's children. Generous on
	//! purpose - see the note where it is applied.
	static const float TOOLBAR_GAP_PX = 14;
	protected Widget m_VehiclesListContent;
	protected UIActionTabs m_Tabs;
	protected Widget m_TabMapContent;
	protected Widget m_TabActionsContent;
	protected Widget m_TabInfoContent;

	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdMap;
	protected int m_TabIdActions;
	protected int m_TabIdInfo;

	//! Refresh button + search field. Flex so the field ends on the same right
	//! edge as the full-width delete buttons below it.
	protected ref UIActionFlexRow m_SearchRow;
	protected UIActionImageButton m_RefreshButton;
	protected UIActionConfirmInline m_DeleteAllButton;
	protected UIActionConfirmInline m_DeleteDestroyedButton;
	protected UIActionConfirmInline m_DeleteUnclaimedButton;

	//! Layout panel the Map tab's MapWidget lives in - a MapWidget cannot be
	//! created from script. Owned by JMVehiclesFormTabMap.
	protected Widget m_MapWidgetPanel;

	// Right panel - options (left half of map area). Owned by JMVehiclesFormTabActions.
	protected Widget m_VehicleOptionsPanel;

	// Right panel - info (right half of map area). Owned by JMVehiclesFormTabInfo.
	protected Widget m_VehicleInfoPanel;

	//! One class per tab, in its own file. Not private: SetVehicleInfo(),
	//! OnResize() and the roster forward into them.
	ref JMVehiclesFormTabMap     m_TabMap;
	ref JMVehiclesFormTabActions m_TabActions;
	ref JMVehiclesFormTabInfo    m_TabInfo;
	protected UIActionSearchBox m_SearchBar;

	//! The toolbar's vehicle type filter - button, dropdown and the shown-types bitmask.
	protected ref JMVehiclesTypeFilter m_TypeFilter;

	//! One roster row per vehicle passing the filters. The map's markers are
	//! owned by JMVehiclesFormTabMap.
	protected ref array<ref JMVehiclesListEntry> m_VehicleEntries;
	protected JMVehicleMetaData m_CurrentVehicle;
	protected bool m_IsInVehicleInfo = false;
	protected JMVehicleMetaData m_PendingDeleteVehicle;
	protected string m_SearchFilter;

	//! The search box's needle, prepared once per change instead of once per vehicle - PassesSearchFilter
	//! runs for every vehicle on every roster and map refresh.
	protected ref JMSearchMatcher m_SearchMatcher;
	protected string m_SearchMatcherFor;

	void JMVehiclesForm()
	{
		m_VehicleEntries = new array<ref JMVehiclesListEntry>;
		m_TypeFilter = new JMVehiclesTypeFilter( this );
	}

	JMVehicleMetaData GetCurrentVehicle() { return m_CurrentVehicle; }

	//! The module this form drives. Public for the tab classes, which run the
	//! per-vehicle commands through it.
	JMVehiclesModule GetModule()
	{
		return m_Module;
	}

	//! selectTab false repaints the panels for a vehicle that is ALREADY the
	//! selected one - a delta refresh, not a pick. Without it the hover panel's
	//! poll would drag an admin off the map tab once a second for as long as
	//! the cursor rested on the marker they had selected.
	void SetVehicleInfo( JMVehicleMetaData vehicle, bool selectTab = false, bool refocusMap = false )
	{
		m_IsInVehicleInfo = true;
		m_CurrentVehicle  = vehicle;

		if ( selectTab && m_Tabs && GetActiveTabIndex() == m_TabIdMap )
			m_Tabs.SetSelection( m_TabIdActions );

		if ( refocusMap && vehicle && m_TabMap )
			m_TabMap.UpdateMapPosition( false, vehicle.m_Position );

		if ( m_TabMap )
			m_TabMap.ApplySelectionRing();

		UpdateUI();

		// The Info tab may not have been built yet - it fills itself in from
		// m_CurrentVehicle when it is.
		if ( m_TabInfo )
			m_TabInfo.PopulateInfo( vehicle );

		if ( m_TabActions )
			m_TabActions.UpdateScroller();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnCreate()
	{
		m_TypeFilter.Reset();

		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip     = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );

		InitWidgetsLeft();
		InitWidgetsRight();
	}

	//! Roster: a toolbar block on top, the scrolling list under it. The layout
	//! gives the two frames their split, so there is no scroller arithmetic
	//! here - the old form recomputed it at 20Hz from the marker tick.
	protected void InitWidgetsLeft()
	{
		m_LeftToolbar = layoutRoot.FindAnyWidget( "panel_left_top" );
		m_LeftList    = layoutRoot.FindAnyWidget( "panel_left_bottom" );

		//! panel_left_top is a FrameWidget: every direct child sits at 0,0 and
		//! paints over the ones before it. One vertical grid owns the block, and
		//! each toolbar row is a cell inside it.
		Widget toolbar = UIActionManager.CreateGridSpacer( m_LeftToolbar, TOOLBAR_ROWS, 1 );

		// Row 1 - search bar with icon-only refresh button.
		//
		// A flex row, not a plain wrap spacer: the search box has to end on the
		// same right edge as the delete buttons under it. A fraction width
		// cannot do that - it is a fraction of the whole block, so the 30px
		// refresh button in front of it always pushed the field short of the
		// edge. Here the button is reserved at its fixed width and the field
		// takes whatever is left.
		//! Every control on this toolbar is drawn at the same height, which
		//! is the height the search box layout is authored at.
		JMSearchRow searchToolbar = UIActionManager.CreateSearchFlexRow( toolbar, "Search", this, "OnChange_Search", "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH", TOOLBAR_CONTROL_PX );

		m_SearchRow = searchToolbar.Row;
		Widget searchRow = m_SearchRow.GetContent();

			m_RefreshButton = searchToolbar.Refresh;

			m_SearchBar = searchToolbar.Search;
			if ( m_SearchBar )
				m_SearchBar.SetTooltip( "#STR_COT_VEHICLES_FILTER_THE_LIST_BY_VEHICLE_NAME" );

			m_TypeFilter.Build( searchRow, m_SearchRow );

			//! The wrap spacer inserts its own padding between children on top
			//! of the widths the flex pass applies. Under-reserving it by even a
			//! pixel wraps the last child onto a second line, which costs the
			//! toolbar a row and pushes the last delete button out of the block.
			m_SearchRow.SetGap( TOOLBAR_GAP_PX );

		// Rows 2-4 - destructive bulk actions, one button per row (mirrors the
		// event manager layout: each action gets its own row at full width, so
		// labels stay legible even on narrow panels).
			m_DeleteAllButton       = UIActionManager.CreateConfirmInline( toolbar, "#STR_COT_VEHICLES_DELETE_ALL",       this, "OnClick_DeleteVehicleAll"       );
			m_DeleteDestroyedButton = UIActionManager.CreateConfirmInline( toolbar, "#STR_COT_VEHICLES_DELETE_DESTROYED", this, "OnClick_DeleteVehicleDestroyed" );
			m_DeleteUnclaimedButton = UIActionManager.CreateConfirmInline( toolbar, "#STR_COT_VEHICLES_DELETE_UNCLAIMED", this, "OnClick_DeleteVehicleUnclaimed" );

			UIActionIconGrid.ApplyDeletePreset( m_DeleteAllButton );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteDestroyedButton );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteUnclaimedButton );

			m_DeleteAllButton.SetTooltip( "#STR_COT_VEHICLES_DELETE_EVERY_VEHICLE_ON_THE_SERVER" );
			m_DeleteDestroyedButton.SetTooltip( "#STR_COT_VEHICLES_DELETE_EVERY_DESTROYED_EXPLODED_VEHICLE" );
			m_DeleteUnclaimedButton.SetTooltip( "#STR_COT_VEHICLES_DELETE_VEHICLES_WITH_NO_KEYS_OR" );

		BindPermission( m_DeleteAllButton,       JMConstants.PERM_VEHICLES_DELETE_ALL );
		BindPermission( m_DeleteDestroyedButton, JMConstants.PERM_VEHICLES_DELETE_DESTROYED );
		BindPermission( m_DeleteUnclaimedButton, JMConstants.PERM_VEHICLES_DELETE_UNCLAIMED );

		// The roster itself. JMVehiclesListEntry parents into whatever widget it
		// is handed, so the scroller's content widget is all it needs.
		m_ListScroller        = UIActionManager.CreateScroller( m_LeftList );
		m_VehiclesListContent = m_ListScroller.GetContentWidget();
	}

	//! Right-hand panel: a tab strip over Map / Actions / Info. The map lives in
	//! the layout because a MapWidget cannot be created from script; the other
	//! two tabs are scrollers built on first activation.
	protected void InitWidgetsRight()
	{
		m_MapWidgetPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_map_panel" ) );

		m_VehicleOptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_actions_panel" ) );
		m_VehicleInfoPanel    = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_panel" ) );


		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );

		m_TabIdMap = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_MAP", JMConstants.Lucide( "map" ), m_MapWidgetPanel );
		m_TabIdActions = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_ACTIONS", JMConstants.Lucide( "zap" ), m_VehicleOptionsPanel );
		m_TabIdInfo = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_INFO", JMConstants.Lucide( "info" ), m_VehicleInfoPanel );

		DeclareTabs( 3 );

		m_Tabs.SetSelection( m_TabIdMap, false );

		InitTabFocus( m_TabIdMap );

		UpdateUI();
	}

	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdMap )
		{
			m_TabMap = new JMVehiclesFormTabMap( this );
			RegisterTab( m_TabIdMap, m_TabMap );
			m_TabMap.OnCreate( panel );
		}
		else if ( tab == m_TabIdActions )
		{
			m_TabActions = new JMVehiclesFormTabActions( this );
			RegisterTab( m_TabIdActions, m_TabActions );
			m_TabActions.OnCreate( panel );
		}
		else if ( tab == m_TabIdInfo )
		{
			m_TabInfo = new JMVehiclesFormTabInfo( this );
			RegisterTab( m_TabIdInfo, m_TabInfo );
			m_TabInfo.OnCreate( panel );
		}

		// A tab built after a vehicle was already picked has missed the pass
		// that would have filled it in.
		if ( m_CurrentVehicle )
			SetVehicleInfo( m_CurrentVehicle );
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	override void OnResize( float w, float h )
	{
		PinRightPanelGeometry( h );
		PinStripGeometry( m_LeftToolbar, m_LeftList, h, TOOLBAR_ROWS * TOOLBAR_ROW_PX );

		ApplyLeftGutter( h );

		if ( m_SearchRow )
			m_SearchRow.Layout();

		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();

		ResizeTabs( w, h );
	}

	//! PinStripGeometry pins both left-hand blocks to x=0 / width=1 every time
	//! it runs, so a gutter written into the layout file survives only until the
	//! first resize. It has to be re-applied here or the toolbar's buttons are
	//! drawn edge to edge and the surface's rounded corners clip them.
	//!
	//! Only the horizontal values are touched: the vertical split is what
	//! PinStripGeometry was just called for.
	protected void ApplyLeftGutter( float contentHeight )
	{
		float x;
		float y;
		float w;
		float h;

		if ( m_LeftToolbar )
		{
			m_LeftToolbar.GetPos( x, y );
			m_LeftToolbar.GetSize( w, h );
			m_LeftToolbar.SetPos( LEFT_GUTTER, y );
			m_LeftToolbar.SetSize( 1.0 - ( LEFT_GUTTER * 2 ), h );
		}

		if ( m_LeftList )
		{
			m_LeftList.GetPos( x, y );
			m_LeftList.GetSize( w, h );
			m_LeftList.SetPos( LEFT_GUTTER, y );

			//! Trim the same gap off the bottom so a partly scrolled row is
			//! clipped inside the window instead of against its frame.
			if ( h > LEFT_GUTTER_PX )
				h = h - LEFT_GUTTER_PX;

			m_LeftList.SetSize( 1.0 - ( LEFT_GUTTER * 2 ), h );
		}
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
			HandleTabChange();
	}

	override protected void OnTabUpdate( int tab )
	{
		super.OnTabUpdate( tab );

		UpdateUI();
	}

	//! Actions and Info are meaningless with nothing picked, so they get the
	//! disable overlay. The map is a browser - it stays usable either way.
	void UpdateUI()
	{
		int sel = GetActiveTabIndex();
		bool hasVehicle = ( m_CurrentVehicle != null );

		if ( m_Tabs )
		{
			m_Tabs.SetTabEnabled( m_TabIdActions, hasVehicle );
			m_Tabs.SetTabEnabled( m_TabIdInfo, hasVehicle );
		}

		if ( !hasVehicle && ( sel == m_TabIdActions || sel == m_TabIdInfo ) )
			SetPanelEnabled( false );
		else
			SetPanelEnabled( true );
	}

	void LoadVehicles()
	{
		if ( m_TabMap )
			m_TabMap.ClearMarkers();

		m_VehicleEntries.Clear();

		if ( !m_Module )
			return;

		auto vehicles = m_Module.GetServerVehicles();
		TStringArray vehicleEntryLabels = {};

		foreach ( JMVehicleMetaData currentVehicle: vehicles )
		{
			if ( !PassesFilters( currentVehicle ) )
				continue;

			if ( m_TabMap )
				m_TabMap.AddVehicleMarker( currentVehicle );

			JMVehiclesListEntry vehicleListEntry = new JMVehiclesListEntry( m_VehiclesListContent, this, currentVehicle );
			m_VehicleEntries.Insert( vehicleListEntry );

			int idx = 0;
			foreach ( string vehicleEntryLabel: vehicleEntryLabels )
			{
				if ( JMStatics.StrCaseCmp( vehicleListEntry.GetLabel(), vehicleEntryLabel ) < 0 )
					break;
				idx++;
			}
			vehicleEntryLabels.InsertAt( vehicleListEntry.GetLabel(), idx );
		}

		int sort;
		foreach ( auto listEntry: m_VehicleEntries )
		{
			sort = vehicleEntryLabels.Find( listEntry.GetLabel() );
			if ( sort > -1 )
				listEntry.SetSort( sort, false );
		}

		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();

		//! Fresh markers start unselected; put the ring back on whichever one
		//! the Actions tab is still pointed at.
		if ( m_TabMap )
			m_TabMap.ApplySelectionRing();
	}

	// -------------------------------------------------------------------------
	//  Delta hooks - invoked by JMVehiclesModule after an action modifies one
	//  vehicle on the server. Avoids the old full-refresh churn.
	// -------------------------------------------------------------------------

	void OnDeltaUpsert( JMVehicleMetaData fresh )
	{
		if ( !fresh )
			return;

		// The hover panel is the reason most upserts arrive at all, so it gets
		// the new figures whatever else this delta turns out to touch.
		if ( m_TabMap )
			m_TabMap.RefreshHoverInfo( fresh );

		// If the user is currently looking at this vehicle's detail panel,
		// refresh it in place with the new data.
		if ( m_IsInVehicleInfo && m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow == fresh.m_NetworkIDLow && m_CurrentVehicle.m_NetworkIDHigh == fresh.m_NetworkIDHigh )
			SetVehicleInfo( fresh, false );

		// A vehicle that is already on screen and still passes the filters can
		// be patched where it stands. Rebuilding would destroy the marker under
		// the cursor - which, while the hover panel is polling, is the one this
		// delta is about - and the engine answers a vanished hover target by
		// recentring the cursor.
		if ( PatchVehicleInPlace( fresh ) )
			return;

		// New vehicle, or one whose refreshed data changes whether it belongs
		// in the roster at all. Module already patched its cache.
		LoadVehicles();
	}

	//! Repoint the existing marker and roster row at `fresh`. False means the
	//! caller has to rebuild: the vehicle is not on screen, or it no longer
	//! belongs there.
	protected bool PatchVehicleInPlace( JMVehicleMetaData fresh )
	{
		if ( !PassesFilters( fresh ) )
			return false;

		if ( !m_TabMap || !m_TabMap.PatchVehicleMarker( fresh ) )
			return false;

		foreach ( JMVehiclesListEntry entry: m_VehicleEntries )
		{
			JMVehicleMetaData entryVehicle = entry.GetVehicle();
			if ( !entryVehicle )
				continue;

			if ( entryVehicle.m_NetworkIDLow != fresh.m_NetworkIDLow || entryVehicle.m_NetworkIDHigh != fresh.m_NetworkIDHigh )
				continue;

			entry.SetVehicle( fresh );
			break;
		}

		return true;
	}

	void OnDeltaRemove( int netLow, int netHigh )
	{
		if ( m_IsInVehicleInfo && m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow  == netLow && m_CurrentVehicle.m_NetworkIDHigh == netHigh )
		{
			BackToList();
		}

		if ( RemoveVehicleInPlace( netLow, netHigh ) )
			return;

		// Not on screen (filtered out, or already gone) - nothing to splice.
		LoadVehicles();
	}

	//! Drop the one marker and one roster row for a vehicle that left the
	//! server list. Mirrors PatchVehicleInPlace: false means the caller has
	//! to fall back, because the vehicle was never on screen to begin with.
	protected bool RemoveVehicleInPlace( int netLow, int netHigh )
	{
		if ( !m_TabMap || !m_TabMap.RemoveVehicleMarker( netLow, netHigh ) )
			return false;

		for ( int i = 0; i < m_VehicleEntries.Count(); i++ )
		{
			JMVehicleMetaData entryVehicle = m_VehicleEntries[i].GetVehicle();
			if ( !entryVehicle )
				continue;

			if ( entryVehicle.m_NetworkIDLow != netLow || entryVehicle.m_NetworkIDHigh != netHigh )
				continue;

			m_VehicleEntries.Remove( i );
			break;
		}

		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();

		return true;
	}

	void SyncAndRefreshVehicles()
	{
		m_Module.RequestServerVehicles();
	}

	void BackToList()
	{
		m_IsInVehicleInfo = false;
		m_CurrentVehicle  = NULL;

		if ( m_Tabs )
			m_Tabs.SetSelection( m_TabIdMap );

		if ( m_TabMap )
			m_TabMap.ApplySelectionRing();

		UpdateUI();
	}

	protected bool PassesSearchFilter( JMVehicleMetaData vehicle )
	{
		if ( !m_SearchMatcher || m_SearchMatcherFor != m_SearchFilter )
		{
			m_SearchMatcher = new JMSearchMatcher( m_SearchFilter );
			m_SearchMatcherFor = m_SearchFilter;
		}

		return m_SearchMatcher.Matches( vehicle.m_DisplayName );
	}

	//! Everything the roster and the map filter on. Shared so a single-vehicle
	//! delta can ask "does this still belong on screen?" with the same answer
	//! the full rebuild would give.
	protected bool PassesFilters( JMVehicleMetaData vehicle )
	{
		return m_TypeFilter.Passes( vehicle ) && PassesSearchFilter( vehicle );
	}

	// -------------------------------------------------------------------------
	//  Map - owned by JMVehiclesFormTabMap. What the window and the module reach
	//  stays here as a thin forward.
	// -------------------------------------------------------------------------

	void HideHoverInfo()
	{
		if ( m_TabMap )
			m_TabMap.HideHoverInfo();
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( m_TabMap )
			m_TabMap.UpdateMapPosition( usePlayerPosition, mapPosition );
	}

	void OnChange_Search( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_SearchBar )
			m_SearchFilter = m_SearchBar.GetText();
		else
			m_SearchFilter = "";

		LoadVehicles();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SyncAndRefreshVehicles();
	}

	//! Ask before deleting one vehicle - from the Actions tab or a map marker's
	//! menu. The answer arrives on ConfirmDeleteVehicle_Yes below.
	void RequestDeleteVehicle( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return;

		m_PendingDeleteVehicle  = vehicle;

		ConfirmAction( "#STR_COT_VEHICLES_DELETE_VEHICLE", "Are you sure you want to delete " + vehicle.m_DisplayName + "?", "ConfirmDeleteVehicle_Yes" );
	}

	void ConfirmDeleteVehicle_Yes( JMConfirmation confirmation = NULL )
	{
		bool deletingSelected = false;
		if ( m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow == m_PendingDeleteVehicle.m_NetworkIDLow && m_CurrentVehicle.m_NetworkIDHigh == m_PendingDeleteVehicle.m_NetworkIDHigh )
			deletingSelected = true;

		m_Module.DeleteVehicle( m_PendingDeleteVehicle );

		if ( deletingSelected )
			BackToList();
	}

	void OnClick_DeleteVehicleAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		ConfirmAction( "#STR_COT_VEHICLES_DELETE_ALL_VEHICLES", "#STR_COT_VEHICLES_ARE_YOU_SURE_YOU_WANT_TO_3", "ConfirmDeleteVehicleAll_Yes" );
	}

	void ConfirmDeleteVehicleAll_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleAll();
	}

	void OnClick_DeleteVehicleDestroyed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		ConfirmAction( "#STR_COT_VEHICLES_DELETE_DESTROYED_VEHICLES", "#STR_COT_VEHICLES_ARE_YOU_SURE_YOU_WANT_TO_2", "ConfirmDeleteVehicleDestroyed_Yes" );
	}

	void ConfirmDeleteVehicleDestroyed_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleDestroyed();
	}

	void OnClick_DeleteVehicleUnclaimed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		ConfirmAction( "#STR_COT_VEHICLES_DELETE_UNCLAIMED_VEHICLES", "#STR_COT_VEHICLES_ARE_YOU_SURE_YOU_WANT_TO", "ConfirmDeleteVehicleUnclaimed_Yes" );
	}

	void ConfirmDeleteVehicleUnclaimed_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleUnclaimed();
	}

	void OnConfirmation_DeleteVehicle()
	{
		m_Module.DeleteVehicle( m_CurrentVehicle );
		BackToList();
	}

	void OnConfirmation_DeleteVehicleAll()
	{
		m_Module.DeleteVehicleAll();
	}

	void OnConfirmation_DeleteVehicleDestroyed()
	{
		m_Module.DeleteVehicleDestroyed();
	}

	void OnConfirmation_DeleteVehicleUnclaimed()
	{
		m_Module.DeleteVehicleUnclaimed();
	}

	//! this-bound trampoline for the deferred search row flex layout, which is
	//! the CallLater target below.
	void _LayoutSearchRow()
	{
		if ( m_SearchRow )
			m_SearchRow.Layout();
	}

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefreshVehicles();

		DeferCall( "UpdateMapPosition", 34, false, new Param2< bool, vector >( true, vector.Zero ) );

		// OnResize fires before the form is rendered, where GetScreenSize
		// returns 0 and Layout() no-ops. Re-flow one tick after it is up.
		DeferCall( "_LayoutSearchRow", 34 );
	}

	override void OnHide()
	{
		super.OnHide();

		HideHoverInfo();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		UpdateUI();
	}

	//! Marker sync runs off the window's per-frame Update() rather than a
	//! Timer, same reasoning UIActionMap.TickMarkers() itself documents: a
	//! 20 Hz Timer meant a marker only caught up with the map three frames
	//! out of four, which reads as the markers lagging behind the terrain
	//! while the map is being dragged. Per-frame is the only rate that
	//! tracks a drag, and it stays cheap because TickMarkers() skips the
	//! work whenever the view has not actually moved.
	override void Update()
	{
		super.Update();

		if ( m_TabMap )
			m_TabMap.Tick();
	}
};
