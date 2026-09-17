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

	static const int TAB_MAP     = 0;
	static const int TAB_ACTIONS = 1;
	static const int TAB_INFO    = 2;

	//! Refresh button + search field. Flex so the field ends on the same right
	//! edge as the full-width delete buttons below it.
	protected ref UIActionFlexRow m_SearchRow;
	protected UIActionImageButton m_RefreshButton;
	protected UIActionConfirmInline m_DeleteAllButton;
	protected UIActionConfirmInline m_DeleteDestroyedButton;
	protected UIActionConfirmInline m_DeleteUnclaimedButton;

	//! The map itself - UIActionMap, the shared component this form's own
	//! markers/hover/selection-ring were the model for. See UIActionMap and
	//! JMUIActionMapMarker.
	protected UIActionMap m_Map;
	protected Widget m_MapWidgetPanel;

	// Right panel - options (left half of map area)
	protected Widget m_VehicleOptionsPanel;
	protected UIActionScroller m_VehicleOptionsScroller;
	protected UIActionConfirmInline m_DeleteVehicleButton;
	protected UIActionButton m_RepairVehicleButton;
	protected UIActionButton m_RefuelVehicleButton;
	protected UIActionButton m_UnstuckVehicleButton;
	protected UIActionButton m_CoverVehicleButton;
	protected UIActionButton m_LockVehicleButton;
	protected UIActionButton m_UnPairVehicleButton;
	protected UIActionButton m_TeleportVehicleButton;
	protected UIActionButton m_TeleportMeButton;

	// Right panel - info (right half of map area)
	protected Widget m_VehicleInfoPanel;
	protected UIActionScroller m_VehicleInfoScroller;
	// Map from copy-button widget -> UIActionText it belongs to
	protected ref map<Widget, ref UIActionText> m_CopyButtonMap;
	protected UIActionText m_VehicleName;
	protected UIActionText m_VehicleClassName;
	protected UIActionText m_VehicleStatus;
	protected UIActionText m_VehicleType;
	protected UIActionText m_VehicleID;
	protected UIActionText m_VehiclePersistentIDAB;
	protected UIActionText m_VehiclePersistentIDCD;
	protected UIActionText m_VehiclePosition;
	protected UIActionText m_VehicleRotation;
	protected UIActionText m_VehicleCoolant;
	protected UIActionText m_VehicleKeys;
	protected UIActionText m_VehicleInfoOwner;
	protected UIActionText m_VehicleLastDriverUID;
	protected UIActionText m_VehicleLastDriverSteam;
	protected UIActionText m_VehicleLastDriverGUID;
	protected UIActionText m_VehicleCovered;

	protected UIActionSearchBox m_SearchBar;

	//! Type filter: the toolbar button, its dropdown, and the bitmask of the
	//! JMVT_* types the roster and the map are currently showing.
	protected UIActionImageButton m_FilterButton;
	protected UIActionContextMenu m_FilterMenu;
	protected int m_TypeFilter = JMVT_ALL;
	//! Anything whose type carries none of the known bits - modded vehicles the
	//! module could not classify. Its own row so it can be hidden on its own.
	protected bool m_FilterShowOther = true;

	static const string TYPE_FILTER_CAR   = "type_car";
	static const string TYPE_FILTER_BOAT  = "type_boat";
	static const string TYPE_FILTER_HELI  = "type_heli";
	static const string TYPE_FILTER_PLANE = "type_plane";
	static const string TYPE_FILTER_BIKE  = "type_bike";
	static const string TYPE_FILTER_OTHER = "type_other";

	//! Vehicle behind each live marker id, by id ("veh_" + netLow + "_" +
	//! netHigh) - UIActionMap owns the marker widgets themselves; this is only
	//! what a hover/click callback needs to get back to the JMVehicleMetaData
	//! it was raised for.
	protected ref map<string, ref JMVehicleMetaData> m_VehicleByMarkerId;
	protected ref array<ref JMVehiclesListEntry> m_VehicleEntries;

	//! Hover readout for map markers. Built on first hover, then reused - it
	//! keeps a local preview entity alive between hovers of the same class.
	protected ref JMVehiclesHoverInfo m_HoverInfo;

	//! The marker the cursor is on, and when its live state is next re-fetched.
	//! Health, fuel, speed and crew are only as fresh as the last sync, and the
	//! panel exists to show exactly those, so the hovered vehicle - and only
	//! the hovered vehicle - is polled while the panel is up.
	protected bool m_HoverActive;
	protected int  m_HoverNetLow;
	protected int  m_HoverNetHigh;
	protected int  m_HoverNextPollAt;

	static const int HOVER_POLL_MS = 1000;

	//! Right-click popup for map markers, plus the vehicle the open menu was
	//! raised on. Built on first use - most sessions never open it.
	protected UIActionContextMenu m_MarkerMenu;
	protected ref JMVehicleMetaData m_MarkerMenuVehicle;

	static const string MK_MENU_SELECT  = "select";
	static const string MK_MENU_TP_TO   = "tp_to";
	static const string MK_MENU_TP_HERE = "tp_here";
	static const string MK_MENU_TP_UNDO = "tp_undo";
	static const string MK_MENU_TP_REDO = "tp_redo";
	static const string MK_MENU_COPYPOS = "copy_pos";
	static const string MK_MENU_FOCUS   = "focus";
	static const string MK_MENU_CARGO   = "clear_cargo";
	static const string MK_MENU_KEY     = "spawn_key";
	static const string MK_MENU_REPAIR  = "repair";
	static const string MK_MENU_REFUEL  = "refuel";
	static const string MK_MENU_UNSTUCK = "unstuck";
	static const string MK_MENU_COVER   = "cover";
	static const string MK_MENU_LOCK    = "lock";
	static const string MK_MENU_UNPAIR  = "unpair";
	static const string MK_MENU_DELETE  = "delete";

	protected JMVehicleMetaData m_CurrentVehicle;
	protected bool m_IsInVehicleInfo = false;
	protected int m_PendingDeleteLow;
	protected int m_PendingDeleteHigh;

	JMVehicleMetaData GetCurrentVehicle() { return m_CurrentVehicle; }
	protected string m_SearchFilter;

	void JMVehiclesForm()
	{
		m_VehicleByMarkerId = new map<string, ref JMVehicleMetaData>;
		m_VehicleEntries    = new array<ref JMVehiclesListEntry>;
		m_CopyButtonMap  = new map<Widget, ref UIActionText>;

		//! Every filter starts open, so the form shows the whole server the
		//! moment it is opened. Assigned here as well as at the declaration:
		//! JMVT_ALL is a global const folded from other global consts, and a
		//! member initialiser that resolved it to 0 would open the form with
		//! every vehicle type hidden and no visible reason why.
		m_TypeFilter      = JMVT_ALL;
		m_FilterShowOther = true;
	}

	// Per-info row: [? 22] [label ????  value ????]
	// Compact spacer keeps the icon tight against the text without a gap.
	protected UIActionText CreateCopyableText( Widget parent, string label, string value = "" )
	{
		Widget row = UIActionManager.CreateWrapSpacerCompact( parent, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionImageButton copyBtn = UIActionManager.CreateIconButton( row, JMConstants.ICON_STACK, this, "OnClick_CopyInfo" );
		copyBtn.SetFixedSize( 22, 22 );
		copyBtn.SetTooltip( "Copy to clipboard" );

		UIActionText txt = UIActionManager.CreateText( row, label, value );
		txt.SetWidth( 0.9 );

		if ( copyBtn )
			m_CopyButtonMap.Insert( copyBtn.GetLayoutRoot(), txt );

		return txt;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_TypeFilter      = JMVT_ALL;
		m_FilterShowOther = true;

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
		m_SearchRow = UIActionManager.CreateFlexRow( toolbar, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		Widget searchRow = m_SearchRow.GetContent();

			m_RefreshButton = UIActionManager.CreateRefreshButton( searchRow, this, "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH" );

			//! Every control on this toolbar is drawn at the same height, which
			//! is the height the search box layout is authored at.
			if ( m_RefreshButton )
			{
				m_RefreshButton.SetFixedSize( TOOLBAR_CONTROL_PX, TOOLBAR_CONTROL_PX );
				m_SearchRow.Add( m_RefreshButton );
			}

			m_SearchBar = UIActionManager.CreateSearchBox( searchRow, this, "OnChange_Search", "Search" );
			if ( m_SearchBar )
			{
				m_SearchBar.SetFlex( 1.0, 60 );
				m_SearchBar.SetTooltip( "Filter the list by vehicle name" );
				m_SearchRow.Add( m_SearchBar );
			}

			m_FilterButton = UIActionManager.CreateIconButton( searchRow, JMConstants.Lucide( "list-filter" ), this, "OnClick_TypeFilters" );
			if ( m_FilterButton )
			{
				m_FilterButton.SetFixedSize( TOOLBAR_CONTROL_PX, TOOLBAR_CONTROL_PX );
				m_FilterButton.SetTooltip( "Filter the list and the map by vehicle type" );
				m_SearchRow.Add( m_FilterButton );
			}

			//! The wrap spacer inserts its own padding between children on top
			//! of the widths the flex pass applies. Under-reserving it by even a
			//! pixel wraps the last child onto a second line, which costs the
			//! toolbar a row and pushes the last delete button out of the block.
			m_SearchRow.SetGap( TOOLBAR_GAP_PX );

		// Rows 2-4 - destructive bulk actions, one button per row (mirrors the
		// event manager layout: each action gets its own row at full width, so
		// labels stay legible even on narrow panels).
			m_DeleteAllButton       = UIActionManager.CreateConfirmInline( toolbar, "Delete All",       this, "OnClick_DeleteVehicleAll"       );
			m_DeleteDestroyedButton = UIActionManager.CreateConfirmInline( toolbar, "Delete Destroyed", this, "OnClick_DeleteVehicleDestroyed" );
			m_DeleteUnclaimedButton = UIActionManager.CreateConfirmInline( toolbar, "Delete Unclaimed", this, "OnClick_DeleteVehicleUnclaimed" );

			UIActionIconGrid.ApplyDeletePreset( m_DeleteAllButton );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteDestroyedButton );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteUnclaimedButton );

			m_DeleteAllButton.SetTooltip( "Delete every vehicle on the server" );
			m_DeleteDestroyedButton.SetTooltip( "Delete every destroyed / exploded vehicle" );
			m_DeleteUnclaimedButton.SetTooltip( "Delete vehicles with no keys or registered owner" );

		RegisterPermission( m_DeleteAllButton,       "Vehicles.Delete.All" );
		RegisterPermission( m_DeleteDestroyedButton, "Vehicles.Delete.Destroyed" );
		RegisterPermission( m_DeleteUnclaimedButton, "Vehicles.Delete.Unclaimed" );

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
		m_Map            = UIActionManager.CreateMapFill( m_MapWidgetPanel, this, "OnClick_Map" );

		m_VehicleOptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_actions_panel" ) );
		m_VehicleInfoPanel    = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_panel" ) );

		ref array<string> tabLabels = { "#STR_COT_VEHICLE_TAB_MAP", "#STR_COT_VEHICLE_TAB_ACTIONS", "#STR_COT_VEHICLE_TAB_INFO" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "map" ), JMConstants.Lucide( "zap" ), JMConstants.Lucide( "info" ) };

		m_Tabs = UIActionManager.CreateTabs( m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_MapWidgetPanel );
		m_Tabs.AddContent( m_VehicleOptionsPanel );
		m_Tabs.AddContent( m_VehicleInfoPanel );

		InitTabState( 3 );

		m_Tabs.SetSelection( TAB_MAP, false );

		BuildTabIfNeeded( TAB_MAP );

		UpdateUI();
	}

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_ACTIONS: InitWidgetsActions(); break;
			case TAB_INFO:    InitWidgetsInfo();    break;
		}

		// A tab built after a vehicle was already picked has missed the pass
		// that would have filled it in.
		if ( m_CurrentVehicle )
			SetVehicleInfo( m_CurrentVehicle );
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	protected void InitWidgetsActions()
	{
		m_VehicleOptionsScroller = UIActionManager.CreateScroller( m_VehicleOptionsPanel );
		Widget optContent = m_VehicleOptionsScroller.GetContentWidget();

		UIActionCard card = UIActionManager.CreateCard( optContent, "#STR_COT_VEHICLE_OPTIONS" );
		Widget gridOptA = UIActionManager.CreateGridSpacer( card.GetContent(), 9, 1 );
			m_DeleteVehicleButton = UIActionManager.CreateConfirmInline( gridOptA, "Delete", this, "OnClick_DeleteVehicle" );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteVehicleButton );
			m_DeleteVehicleButton.SetTooltip( "Remove this vehicle from the world" );
			m_RepairVehicleButton   = UIActionManager.CreateButton( gridOptA, "Repair", this, "OnClick_RepairVehicle" );
			m_RepairVehicleButton.SetTooltip( "Restore health and replace missing attachments" );
			m_RefuelVehicleButton   = UIActionManager.CreateButton( gridOptA, "Refuel", this, "OnClick_RefuelVehicle" );
			m_RefuelVehicleButton.SetTooltip( "Fill fuel, oil, brake and coolant to full" );
			m_UnstuckVehicleButton  = UIActionManager.CreateButton( gridOptA, "Unstuck", this, "OnClick_UnstuckVehicle" );
			m_UnstuckVehicleButton.SetTooltip( "Lift 1.5 m and drop back onto the ground" );
			m_CoverVehicleButton    = UIActionManager.CreateButton( gridOptA, "Cover/Uncover", this, "OnClick_CoverVehicle" );
			m_CoverVehicleButton.SetTooltip( "Toggle Expansion vehicle cover" );
			m_LockVehicleButton     = UIActionManager.CreateButton( gridOptA, "Lock/Unlock", this, "OnClick_LockVehicle" );
			m_LockVehicleButton.SetTooltip( "Toggle the key lock state" );
			m_UnPairVehicleButton   = UIActionManager.CreateButton( gridOptA, "UnPair Keys", this, "OnClick_UnPairVehicle" );
			m_UnPairVehicleButton.SetTooltip( "Detach all paired keys from this vehicle" );
			m_TeleportVehicleButton = UIActionManager.CreateButton( gridOptA, "Teleport To Vehicle", this, "OnClick_TeleportToVehicle" );
			m_TeleportVehicleButton.SetTooltip( "Teleport yourself to this vehicle's location" );
			m_TeleportMeButton      = UIActionManager.CreateButton( gridOptA, "Teleport Vehicle To Me", this, "OnClick_TeleportVehicleToMe" );
			m_TeleportMeButton.SetTooltip( "Move this vehicle to your current position" );

		RegisterPermission( m_DeleteVehicleButton,   "Vehicles.Delete" );
		RegisterPermission( m_RepairVehicleButton,   "Vehicles.Repair" );
		RegisterPermission( m_RefuelVehicleButton,   "Vehicles.Refuel" );
		RegisterPermission( m_UnstuckVehicleButton,  "Vehicles.Unstuck" );
		RegisterPermission( m_CoverVehicleButton,    "Vehicles.Cover" );
		RegisterPermission( m_LockVehicleButton,     "Vehicles.Lock" );
		RegisterPermission( m_UnPairVehicleButton,   "Vehicles.UnPair" );
		RegisterPermission( m_TeleportVehicleButton, "Vehicles.Teleport" );
		RegisterPermission( m_TeleportMeButton,      "Vehicles.Teleport" );

	#ifndef EXPANSIONMODVEHICLE
		// Cover, Lock, UnPair are Expansion-only - disable them when Expansion is not loaded
		if ( m_CoverVehicleButton   ) m_CoverVehicleButton.SetEnabled( false );
		if ( m_LockVehicleButton    ) m_LockVehicleButton.SetEnabled( false );
		if ( m_UnPairVehicleButton  ) m_UnPairVehicleButton.SetEnabled( false );
	#endif

		m_VehicleOptionsScroller.UpdateScroller();
	}

	protected void InitWidgetsInfo()
	{
		m_VehicleInfoScroller = UIActionManager.CreateScroller( m_VehicleInfoPanel );
		Widget infoContent = m_VehicleInfoScroller.GetContentWidget();

		UIActionCard infoCard = UIActionManager.CreateCard( infoContent, "#STR_COT_VEHICLE_INFORMATION_TITLE" );
		Widget gridInfoA = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 10, 1 );
			m_VehicleName           = CreateCopyableText( gridInfoA, "Name:", "Value" );
			m_VehicleClassName      = CreateCopyableText( gridInfoA, "ClassName:", "Value" );
			m_VehicleStatus         = CreateCopyableText( gridInfoA, "Status:", "Value" );
			m_VehicleType           = CreateCopyableText( gridInfoA, "Type:", "Value" );
			m_VehicleID             = CreateCopyableText( gridInfoA, "ID:", "Value" );
			m_VehiclePersistentIDAB = CreateCopyableText( gridInfoA, "ID AB:", "Value" );
			m_VehiclePersistentIDCD = CreateCopyableText( gridInfoA, "ID CD:", "Value" );
			m_VehiclePosition       = CreateCopyableText( gridInfoA, "Position:", "Value" );
			m_VehicleRotation       = CreateCopyableText( gridInfoA, "Rotation:", "Value" );
			m_VehicleCoolant        = CreateCopyableText( gridInfoA, "Coolant:", "Value" );

		Widget gridInfoB = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 7, 1 );
			m_VehicleKeys            = CreateCopyableText( gridInfoB, "Keys:", "Value" );
			m_VehicleInfoOwner       = CreateCopyableText( gridInfoB, "Owner:", "Value" );
			m_VehicleLastDriverUID   = CreateCopyableText( gridInfoB, "Driver UID:", "N/A" );
			m_VehicleLastDriverSteam = CreateCopyableText( gridInfoB, "Driver Steam:", "N/A" );
			m_VehicleLastDriverGUID  = CreateCopyableText( gridInfoB, "Driver GUID:", "N/A" );
			m_VehicleCovered         = CreateCopyableText( gridInfoB, "Covered:", "Value" );
			UIActionManager.CreateText( gridInfoB, "" );

		m_VehicleInfoScroller.UpdateScroller();
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

		if ( m_VehicleOptionsScroller )
			m_VehicleOptionsScroller.UpdateScroller();

		if ( m_VehicleInfoScroller )
			m_VehicleInfoScroller.UpdateScroller();
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
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		//! The hover panel is anchored to the window root, not to the map tab,
		//! so switching tabs does not hide it on its own.
		HideHoverInfo();

		BuildTabIfNeeded( GetActiveTabIndex() );

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
			m_Tabs.SetTabEnabled( TAB_ACTIONS, hasVehicle );
			m_Tabs.SetTabEnabled( TAB_INFO, hasVehicle );
		}

		if ( !hasVehicle && ( sel == TAB_ACTIONS || sel == TAB_INFO ) )
			HideUI();
		else
			ShowUI();
	}

	//! Layer this form's own vehicle markers live under on the shared map -
	//! nothing else currently shares JMVehiclesForm's UIActionMap, but a name
	//! is cheap and ClearLayer needs one.
	static const string MAP_LAYER = "vehicles";

	protected string MarkerIdFor( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return "";

		return "veh_" + vehicle.m_NetworkIDLow + "_" + vehicle.m_NetworkIDHigh;
	}

	void LoadVehicles()
	{
		//! Every marker is about to be cleared, including whichever one the
		//! cursor is on - and a cleared marker never raises the hover-leave
		//! that would take the hover panel down.
		HideHoverInfo();

		if ( m_Map )
			m_Map.ClearLayer( MAP_LAYER );

		m_VehicleByMarkerId.Clear();
		m_VehicleEntries.Clear();

		if ( !m_Module )
			return;

		auto vehicles = m_Module.GetServerVehicles();
		TStringArray vehicleEntryLabels = {};

		foreach ( JMVehicleMetaData currentVehicle: vehicles )
		{
			if ( !PassesFilters( currentVehicle ) )
				continue;

			int color;
			string marker;
			GetVehicleTypeInfo( currentVehicle.m_VehicleType, color, marker );

			string markerId = MarkerIdFor( currentVehicle );
			m_VehicleByMarkerId.Insert( markerId, currentVehicle );

			if ( m_Map )
				m_Map.AddMarker( markerId, currentVehicle.m_Position, currentVehicle.m_DisplayName, color, marker, MAP_LAYER );

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
		ApplySelectionRing();
	}

	//! Marker glyph per vehicle class. Every entry is a real texture path: the
	//! old table handed LoadImageFile bare words ("Boat", "Helicopter", ...),
	//! which resolve to nothing and draw as the engine's magenta placeholder.
	private void GetVehicleTypeInfo( int type, out int color, out string marker )
	{
		switch ( type )
		{
			default:
			case JMVT_CAR:
				color  = ARGB( 255, 243, 156, 18 );
				marker = JMConstants.Lucide( "car" );
			break;
			case JMVT_BOAT:
				color  = ARGB( 255, 243, 18, 156 );
				marker = JMConstants.Lucide( "sailboat" );
			break;
			case JMVT_HELICOPTER:
				color  = ARGB( 255, 18, 156, 243 );
				marker = JMConstants.Lucide( "helicopter" );
			break;
			case JMVT_PLANE:
				color  = ARGB( 255, 18, 243, 156 );
				marker = JMConstants.Lucide( "plane" );
			break;
			case JMVT_BIKE:
				color  = ARGB( 255, 243, 156, 100 );
				marker = JMConstants.Lucide( "bike" );
			break;
		}
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
		if ( m_HoverInfo )
			m_HoverInfo.Refresh( fresh );

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

		string markerId = MarkerIdFor( fresh );
		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return false;

		m_VehicleByMarkerId.Set( markerId, fresh );

		if ( m_Map )
		{
			int color;
			string marker;
			GetVehicleTypeInfo( fresh.m_VehicleType, color, marker );

			//! AddMarker replaces the marker already under this id in place -
			//! this is the delta-refresh path, so the id always already exists.
			m_Map.AddMarker( markerId, fresh.m_Position, fresh.m_DisplayName, color, marker, MAP_LAYER );
		}

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
		string markerId = "veh_" + netLow + "_" + netHigh;

		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return false;

		m_VehicleByMarkerId.Remove( markerId );

		if ( m_Map )
			m_Map.RemoveMarker( markerId );

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

	//! selectTab false repaints the panels for a vehicle that is ALREADY the
	//! selected one - a delta refresh, not a pick. Without it the hover panel's
	//! poll would drag an admin off the map tab once a second for as long as
	//! the cursor rested on the marker they had selected.
	void SetVehicleInfo( JMVehicleMetaData vehicle, bool selectTab = false, bool refocusMap = false )
	{
		m_IsInVehicleInfo = true;
		m_CurrentVehicle  = vehicle;

		if ( selectTab && m_Tabs && GetActiveTabIndex() == TAB_MAP )
			m_Tabs.SetSelection( TAB_ACTIONS );

		if ( refocusMap && vehicle )
			UpdateMapPosition( false, vehicle.m_Position );

		ApplySelectionRing();

		UpdateUI();

		// The Info tab may not have been built yet - it fills itself in from
		// m_CurrentVehicle when it is.
		if ( !m_VehicleName )
			return;

		m_VehicleName.SetText( vehicle.m_DisplayName );
		m_VehicleClassName.SetText( vehicle.m_ClassName );
		string statusText = vehicle.GetVehicleDestructionState();
		if ( statusText == "None" )
			statusText = "OK";
		m_VehicleStatus.SetText( statusText );
		m_VehicleType.SetText( vehicle.GetVehicleType() );
		m_VehicleID.SetText( vehicle.m_NetworkIDHigh.ToString() + " " + vehicle.m_NetworkIDLow.ToString() );
		m_VehiclePersistentIDAB.SetText( vehicle.m_PersistentIDA.ToString() + " " + vehicle.m_PersistentIDB.ToString() );
		m_VehiclePersistentIDCD.SetText( vehicle.m_PersistentIDC.ToString() + " " + vehicle.m_PersistentIDD.ToString() );
		m_VehiclePosition.SetText( vehicle.m_Position.ToString() );
		m_VehicleRotation.SetText( vehicle.m_Orientation.ToString() );

		string coolantText = "N/A";
		if ( vehicle.m_CoolantPct >= 0 )
			coolantText = Math.Round( vehicle.m_CoolantPct * 100 ).ToString() + "%";
		if ( m_VehicleCoolant )
			m_VehicleCoolant.SetText( coolantText );

		string hasKeys;
		if ( vehicle.m_HasKeys )
			hasKeys = "Yes";
		else
			hasKeys = "No";
		m_VehicleKeys.SetText( hasKeys );
		if ( vehicle.m_OwnerName != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_OwnerName + " (" + vehicle.m_OwnerUID + ")" );
		else if ( vehicle.m_LastDriverSteam != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_LastDriverSteam + " (last driver)" );
		else if ( vehicle.m_LastDriverUID != "" )
			m_VehicleInfoOwner.SetText( vehicle.m_LastDriverUID + " (last driver)" );
		else
			m_VehicleInfoOwner.SetText( "Unknown" );

		if ( vehicle.m_LastDriverUID != "" )
			m_VehicleLastDriverUID.SetText( vehicle.m_LastDriverUID );
		else
			m_VehicleLastDriverUID.SetText( "N/A" );

		if ( vehicle.m_LastDriverSteam != "" )
			m_VehicleLastDriverSteam.SetText( vehicle.m_LastDriverSteam );
		else
			m_VehicleLastDriverSteam.SetText( "N/A" );

		if ( vehicle.m_LastDriverGUID != "" )
			m_VehicleLastDriverGUID.SetText( vehicle.m_LastDriverGUID );
		else
			m_VehicleLastDriverGUID.SetText( "N/A" );

		string isCovered;
		if ( vehicle.m_IsCover )
			isCovered = "Yes";
		else
			isCovered = "No";
		m_VehicleCovered.SetText( isCovered );

		m_VehicleInfoScroller.UpdateScroller();
		m_VehicleOptionsScroller.UpdateScroller();
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
			m_Tabs.SetSelection( TAB_MAP );

		ApplySelectionRing();

		UpdateUI();
	}

	// -------------------------------------------------------------------------
	//  Vehicle type filter
	// -------------------------------------------------------------------------

	//! Open the type filter dropdown under the toolbar button. A context menu
	//! rather than a strip of checkboxes: it floats over the roster instead of
	//! costing the toolbar a row it only needs while being changed.
	void OnClick_TypeFilters( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterButton )
			return;

		if ( !m_FilterMenu )
		{
			if ( !m_Window )
				return;

			m_FilterMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_TypeFilterMenu" );
			RegisterOverlay( m_FilterMenu );

			if ( !m_FilterMenu )
				return;

			//! Every row is a toggle, so a click is never "done with this menu".
			m_FilterMenu.SetCloseOnClick( false );
			m_FilterMenu.SetOwnerWidget( m_FilterButton.GetLayoutRoot() );
		}

		//! A second click on the button puts the menu away rather than
		//! reopening it in place, which is what a dropdown is expected to do.
		if ( m_FilterMenu.IsOpen() )
		{
			m_FilterMenu.Close();
			return;
		}

		RebuildTypeFilterMenu();

		float fx;
		float fy;
		float fw;
		float fh;
		m_FilterButton.GetLayoutRoot().GetScreenPos( fx, fy );
		m_FilterButton.GetLayoutRoot().GetScreenSize( fw, fh );

		m_FilterMenu.ShowAt( fx, fy + fh );
	}

	//! An active type is normal text, a hidden one is dimmed - the row IS the
	//! checkbox, so there is no separate tick to keep in step.
	private void RebuildTypeFilterMenu()
	{
		if ( !m_FilterMenu )
			return;

		m_FilterMenu.ClearItems();

		AddTypeFilterItem( TYPE_FILTER_CAR,   "Cars",         "car",        IsTypeShown( JMVT_CAR ) );
		AddTypeFilterItem( TYPE_FILTER_BOAT,  "Boats",        "sailboat",   IsTypeShown( JMVT_BOAT ) );
		AddTypeFilterItem( TYPE_FILTER_HELI,  "Helicopters",  "helicopter", IsTypeShown( JMVT_HELICOPTER ) );
		AddTypeFilterItem( TYPE_FILTER_PLANE, "Planes",       "plane",      IsTypeShown( JMVT_PLANE ) );
		AddTypeFilterItem( TYPE_FILTER_BIKE,  "Motorbikes",   "bike",       IsTypeShown( JMVT_BIKE ) );
		AddTypeFilterItem( TYPE_FILTER_OTHER, "Unclassified", "circle-help", m_FilterShowOther );
	}

	private void AddTypeFilterItem( string id, string label, string icon, bool shown )
	{
		m_FilterMenu.AddItem( id, label, JMConstants.Lucide( icon ), TypeFilterItemColor( shown ) );
	}

	private int TypeFilterItemColor( bool shown )
	{
		if ( shown )
			return JMTheme.TEXT_PRIMARY;

		return JMTheme.TEXT_DISABLED;
	}

	private bool IsTypeShown( int typeBit )
	{
		return ( m_TypeFilter & typeBit ) != 0;
	}

	//! Repaint the rows in place. Rebuilding them here would destroy the very
	//! row the click landed on, and the engine answers a vanished press target
	//! by recentring the cursor.
	private void RefreshTypeFilterColors()
	{
		if ( !m_FilterMenu )
			return;

		m_FilterMenu.SetItemTextColor( TYPE_FILTER_CAR,   TypeFilterItemColor( IsTypeShown( JMVT_CAR ) ) );
		m_FilterMenu.SetItemTextColor( TYPE_FILTER_BOAT,  TypeFilterItemColor( IsTypeShown( JMVT_BOAT ) ) );
		m_FilterMenu.SetItemTextColor( TYPE_FILTER_HELI,  TypeFilterItemColor( IsTypeShown( JMVT_HELICOPTER ) ) );
		m_FilterMenu.SetItemTextColor( TYPE_FILTER_PLANE, TypeFilterItemColor( IsTypeShown( JMVT_PLANE ) ) );
		m_FilterMenu.SetItemTextColor( TYPE_FILTER_BIKE,  TypeFilterItemColor( IsTypeShown( JMVT_BIKE ) ) );
		m_FilterMenu.SetItemTextColor( TYPE_FILTER_OTHER, TypeFilterItemColor( m_FilterShowOther ) );
	}

	void OnClick_TypeFilterMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterMenu )
			return;

		string id = m_FilterMenu.GetLastClickedId();

		if ( id == TYPE_FILTER_CAR )
			m_TypeFilter = m_TypeFilter ^ JMVT_CAR;
		else if ( id == TYPE_FILTER_BOAT )
			m_TypeFilter = m_TypeFilter ^ JMVT_BOAT;
		else if ( id == TYPE_FILTER_HELI )
			m_TypeFilter = m_TypeFilter ^ JMVT_HELICOPTER;
		else if ( id == TYPE_FILTER_PLANE )
			m_TypeFilter = m_TypeFilter ^ JMVT_PLANE;
		else if ( id == TYPE_FILTER_BIKE )
			m_TypeFilter = m_TypeFilter ^ JMVT_BIKE;
		else if ( id == TYPE_FILTER_OTHER )
			m_FilterShowOther = !m_FilterShowOther;
		else
			return;

		RefreshTypeFilterColors();
		LoadVehicles();
	}

	//! m_VehicleType is a bitmask, and a vehicle can legitimately carry more
	//! than one bit, so a match on any shown bit keeps it. A vehicle carrying
	//! none of the known bits is unclassified and answers to its own toggle.
	private bool PassesTypeFilter( JMVehicleMetaData vehicle )
	{
		if ( ( vehicle.m_VehicleType & JMVT_ALL ) == 0 )
			return m_FilterShowOther;

		return ( vehicle.m_VehicleType & m_TypeFilter ) != 0;
	}

	private bool PassesSearchFilter( JMVehicleMetaData vehicle )
	{
		if ( m_SearchFilter == "" )
			return true;

		string displayNameLower = vehicle.m_DisplayName;
		displayNameLower.ToLower();

		string filterLower = m_SearchFilter;
		filterLower.ToLower();

		return displayNameLower.IndexOf( filterLower ) != -1;
	}

	//! Everything the roster and the map filter on. Shared so a single-vehicle
	//! delta can ask "does this still belong on screen?" with the same answer
	//! the full rebuild would give.
	private bool PassesFilters( JMVehicleMetaData vehicle )
	{
		return PassesTypeFilter( vehicle ) && PassesSearchFilter( vehicle );
	}

	// -------------------------------------------------------------------------
	//  Map events - hover, click, right-click, all funnelled through here from
	//  UIActionMap's single callback.
	// -------------------------------------------------------------------------

	void OnClick_Map( UIEvent eid, UIActionBase action )
	{
		if ( !m_Map )
			return;

		if ( eid == UIEvent.MOUSE_ENTER )
		{
			string hoveredId = m_Map.GetHoveredMarkerId();
			if ( m_VehicleByMarkerId.Contains( hoveredId ) )
				OnMarker_HoverEnter( m_VehicleByMarkerId.Get( hoveredId ) );
			return;
		}

		if ( eid == UIEvent.MOUSE_LEAVE )
		{
			//! UIActionMap only fires a leave for the marker it currently has
			//! hovered - a leave for one already superseded by a newer enter
			//! is swallowed there, so there is no stale-vs-fresh race to sort
			//! out here the way there was chasing raw widget events by hand.
			HideHoverInfo();
			return;
		}

		if ( eid != UIEvent.CLICK )
			return;

		string markerId = m_Map.GetLastClickedMarkerId();
		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return;

		JMVehicleMetaData vehicle = m_VehicleByMarkerId.Get( markerId );

		if ( m_Map.GetLastClickButton() == MouseState.RIGHT )
		{
			OnMarker_RightClick( vehicle );
			return;
		}

		SetVehicleInfo( vehicle );
	}

	//! Raised via UIActionMap's UIEvent.MOUSE_ENTER (see OnClick_Map). Opens
	//! the info panel and starts polling that one vehicle's live state.
	void OnMarker_HoverEnter( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_Window )
			return;

		if ( !m_HoverInfo )
			m_HoverInfo = new JMVehiclesHoverInfo( m_Window.GetWidgetRoot() );

		int x, y;
		GetMousePos( x, y );

		m_HoverInfo.ShowAt( vehicle, x, y );

		m_HoverActive  = true;
		m_HoverNetLow  = vehicle.m_NetworkIDLow;
		m_HoverNetHigh = vehicle.m_NetworkIDHigh;

		//! The panel is up with whatever the last sync knew; ask for the real
		//! figures straight away rather than waiting out the first interval.
		PollHoveredVehicle();
	}

	void HideHoverInfo()
	{
		m_HoverActive = false;

		if ( m_HoverInfo )
			m_HoverInfo.Hide();
	}

	// -------------------------------------------------------------------------
	//  Selection ring
	// -------------------------------------------------------------------------

	//! Point UIActionMap's selection ring at whatever m_CurrentVehicle is now.
	//! UIActionMap owns the ring widget and spins it itself from TickMarkers()
	//! (see Update()) - this only has to say which marker it belongs on.
	protected void ApplySelectionRing()
	{
		if ( !m_Map )
			return;

		m_Map.SetSelectedMarker( MarkerIdFor( m_CurrentVehicle ) );
	}

	//! One vehicle, once per HOVER_POLL_MS. The server answers with a normal
	//! Upsert, which lands in OnDeltaUpsert and repaints the panel.
	protected void PollHoveredVehicle()
	{
		if ( !m_Module )
			return;

		m_HoverNextPollAt = g_Game.GetTime() + HOVER_POLL_MS;

		m_Module.RequestVehicleUpsert( m_HoverNetLow, m_HoverNetHigh );
	}

	// -------------------------------------------------------------------------
	//  Map marker context menu
	// -------------------------------------------------------------------------

	//! Raised via UIActionMap's right-click CLICK event (see OnClick_Map).
	//! Offers the same actions as the Actions tab against the marker under the
	//! cursor, so an admin does not have to select the vehicle and change tab
	//! first.
	void OnMarker_RightClick( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return;

		if ( !m_MarkerMenu )
		{
			if ( !m_Window )
				return;

			//! Anchored to the window root, not to the map panel: a menu
			//! parented to the map would be clipped by it near the edges.
			m_MarkerMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_MarkerMenu" );
			RegisterOverlay( m_MarkerMenu );

			if ( !m_MarkerMenu )
				return;
		}

		m_MarkerMenuVehicle = vehicle;

		m_MarkerMenu.ClearItems();

		bool isAlreadySelected = ( m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow == vehicle.m_NetworkIDLow && m_CurrentVehicle.m_NetworkIDHigh == vehicle.m_NetworkIDHigh );
		if ( !isAlreadySelected )
			m_MarkerMenu.AddItem( MK_MENU_SELECT,  "Select",                        JMConstants.Lucide( "mouse-pointer-click" ) );

		m_MarkerMenu.AddItem( MK_MENU_TP_TO,   "Teleport To Vehicle",           JMConstants.Lucide( "footprints" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_HERE, "Teleport Vehicle To Me",        JMConstants.Lucide( "move-down-left" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_UNDO, "Undo Teleport",                 JMConstants.Lucide( "undo-2" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_REDO, "Redo Teleport",                 JMConstants.Lucide( "redo-2" ) );
		m_MarkerMenu.AddItem( MK_MENU_COPYPOS, "Copy Position",                 JMConstants.Lucide( "copy" ) );
		m_MarkerMenu.AddItem( MK_MENU_FOCUS,   "Center Map Here",               JMConstants.Lucide( "locate-fixed" ) );
		m_MarkerMenu.AddItem( MK_MENU_REPAIR,  "Repair",                        JMConstants.Lucide( "wrench" ) );
		m_MarkerMenu.AddItem( MK_MENU_REFUEL,  "Refuel",                        JMConstants.Lucide( "fuel" ) );
		m_MarkerMenu.AddItem( MK_MENU_UNSTUCK, "Unstuck",                       JMConstants.Lucide( "arrow-up-from-line" ) );
		m_MarkerMenu.AddItem( MK_MENU_CARGO,   "Clear Cargo",                   JMConstants.Lucide( "package-x" ), JMTheme.DANGER );

	#ifdef EXPANSIONMODVEHICLE
		m_MarkerMenu.AddItem( MK_MENU_COVER,  "Cover/Uncover", JMConstants.Lucide( "tent" ) );
		m_MarkerMenu.AddItem( MK_MENU_LOCK,   "Lock/Unlock",   JMConstants.Lucide( "lock" ) );
		m_MarkerMenu.AddItem( MK_MENU_UNPAIR, "UnPair Keys",   JMConstants.Lucide( "key-round" ) );
		m_MarkerMenu.AddItem( MK_MENU_KEY,    "Spawn Key",     JMConstants.Lucide( "key" ) );
	#endif

		m_MarkerMenu.AddItem( MK_MENU_DELETE, "Delete", JMConstants.ICON_TRASH_CAN, JMTheme.DANGER );

		//! SetItemEnabled on an id the menu is not carrying is a no-op, so the
		//! Expansion-only entries need no special case here.
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_TO,   GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_HERE, GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) );

		//! Greyed rather than dropped when there is nothing recorded: an entry
		//! that comes and goes moves every row under it between one opening of
		//! the menu and the next.
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_UNDO, GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) && HasTeleportUndo( vehicle ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_REDO, GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) && HasTeleportRedo( vehicle ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_CARGO,   GetPermissionsManager().HasPermission( "Vehicles.ClearCargo" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_KEY,     GetPermissionsManager().HasPermission( "Vehicles.SpawnKey" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_REPAIR,  GetPermissionsManager().HasPermission( "Vehicles.Repair" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_REFUEL,  GetPermissionsManager().HasPermission( "Vehicles.Refuel" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_UNSTUCK, GetPermissionsManager().HasPermission( "Vehicles.Unstuck" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_COVER,   GetPermissionsManager().HasPermission( "Vehicles.Cover" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_LOCK,    GetPermissionsManager().HasPermission( "Vehicles.Lock" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_UNPAIR,  GetPermissionsManager().HasPermission( "Vehicles.UnPair" ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_DELETE,  GetPermissionsManager().HasPermission( "Vehicles.Delete" ) );

		int mx, my;
		GetMousePos( mx, my );

		m_MarkerMenu.ShowAt( mx, my );
	}

	void OnClick_MarkerMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_MarkerMenu || !m_Module )
			return;

		JMVehicleMetaData vehicle = m_MarkerMenuVehicle;

		if ( !vehicle )
			return;

		string id = m_MarkerMenu.GetLastClickedId();

		//! Enforce has no block scope, so this is declared once up here rather
		//! than inside the delete branch that uses it.
		bool deletingSelected = false;

		if ( id == MK_MENU_SELECT )
		{
			SetVehicleInfo( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_TO )
		{
			m_Module.RequestTeleportToVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_HERE )
		{
			m_Module.RequestTeleportVehicleToMe( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_UNDO )
		{
			UndoTeleport( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_REDO )
		{
			RedoTeleport( vehicle );
			return;
		}

		if ( id == MK_MENU_COPYPOS )
		{
			//! Same "<x, y, z>" shape every other copy in COT writes, so it
			//! pastes into the player manager's coordinate box or an ESP paste.
			g_Game.CopyToClipboard( "<" + vehicle.m_Position[0] + ", " + vehicle.m_Position[1] + ", " + vehicle.m_Position[2] + ">" );
			return;
		}

		if ( id == MK_MENU_FOCUS )
		{
			UpdateMapPosition( false, vehicle.m_Position );
			return;
		}

		if ( id == MK_MENU_CARGO )
		{
			if ( GetPermissionsManager().HasPermission( "Vehicles.ClearCargo" ) )
				m_Module.RequestClearVehicleCargo( vehicle );
			return;
		}

		if ( id == MK_MENU_KEY )
		{
			if ( GetPermissionsManager().HasPermission( "Vehicles.SpawnKey" ) )
				m_Module.RequestSpawnVehicleKey( vehicle );
			return;
		}

		if ( id == MK_MENU_REPAIR )
		{
			m_Module.RequestRepairVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_REFUEL )
		{
			m_Module.RequestRefuelVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_UNSTUCK )
		{
			m_Module.RequestUnstuckVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_COVER )
		{
			m_Module.RequestCoverVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_LOCK )
		{
			m_Module.RequestLockVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_UNPAIR )
		{
			m_Module.RequestUnPairVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_DELETE )
		{
			if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete" ) )
				return;

			m_PendingDeleteLow  = vehicle.m_NetworkIDLow;
			m_PendingDeleteHigh = vehicle.m_NetworkIDHigh;

			CreateConfirmation_Two( JMConfirmationType.INFO, "Delete Vehicle", "Are you sure you want to delete " + vehicle.m_DisplayName + "?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmDeleteVehicle_Yes" );
			return;
		}
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

		action.AnimateSpin( 2 );
		SyncAndRefreshVehicles();
	}

	void OnClick_CopyInfo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !action )
			return;

		UIActionText txt = m_CopyButtonMap.Get( action.GetLayoutRoot() );
		if ( txt )
			g_Game.CopyToClipboard( txt.GetText() );
	}

	//! Whether this vehicle has a move of its own this session that can still
	//! be taken back.
	protected bool HasTeleportUndo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return false;

		return JMTeleportHistory.Has( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ) );
	}

	protected bool HasTeleportRedo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return false;

		return JMTeleportHistory.HasRedo( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ) );
	}

	//! Put back a move an undo took away.
	//!
	//! Goes out through the same RPC the undo does - the two differ only in
	//! which stack they read, which is what keeps them exact mirrors.
	protected void RedoTeleport( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_Module )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) )
			return;

		vector forward = JMTeleportHistory.PopRedo( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ), 0, vehicle.m_Position );

		if ( forward == vector.Zero )
			return;

		JMTeleportHistory.BeginApply();
		m_Module.RequestTeleportVehicleTo( vehicle, forward );
		JMTeleportHistory.EndApply();
	}

	//! Put the vehicle back where it was before its last move. Each use walks
	//! one more step back, up to JMTeleportHistory.MAX_ENTRIES.
	protected void UndoTeleport( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_Module )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Teleport" ) )
			return;

		vector back = JMTeleportHistory.Pop( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ), 0, vehicle.m_Position );

		if ( back == vector.Zero )
			return;

		JMTeleportHistory.BeginApply();
		m_Module.RequestTeleportVehicleTo( vehicle, back );
		JMTeleportHistory.EndApply();
	}

	void OnClick_TeleportToVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestTeleportToVehicle( m_CurrentVehicle );
	}

	void OnClick_TeleportVehicleToMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestTeleportVehicleToMe( m_CurrentVehicle );
	}

	void OnClick_RepairVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestRepairVehicle( m_CurrentVehicle );
	}

	void OnClick_RefuelVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestRefuelVehicle( m_CurrentVehicle );
	}

	void OnClick_UnstuckVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestUnstuckVehicle( m_CurrentVehicle );
	}

	void OnClick_CoverVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestCoverVehicle( m_CurrentVehicle );
	}

	void OnClick_LockVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestLockVehicle( m_CurrentVehicle );
	}

	void OnClick_UnPairVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestUnPairVehicle( m_CurrentVehicle );
	}

	void OnClick_DeleteVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_CurrentVehicle )
		{
			m_PendingDeleteLow  = m_CurrentVehicle.m_NetworkIDLow;
			m_PendingDeleteHigh = m_CurrentVehicle.m_NetworkIDHigh;

			CreateConfirmation_Two( JMConfirmationType.INFO, "Delete Vehicle", "Are you sure you want to delete " + m_CurrentVehicle.m_DisplayName + "?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmDeleteVehicle_Yes" );
		}
	}

	void ConfirmDeleteVehicle_Yes( JMConfirmation confirmation = NULL )
	{
		bool deletingSelected = false;
		if ( m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow == m_PendingDeleteLow && m_CurrentVehicle.m_NetworkIDHigh == m_PendingDeleteHigh )
			deletingSelected = true;

		m_Module.DeleteVehicle( m_PendingDeleteLow, m_PendingDeleteHigh );

		if ( deletingSelected )
			BackToList();
	}

	void OnClick_DeleteVehicleAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Delete All Vehicles", "Are you sure you want to delete ALL vehicles on the server?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmDeleteVehicleAll_Yes" );
	}

	void ConfirmDeleteVehicleAll_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleAll();
	}

	void OnClick_DeleteVehicleDestroyed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Delete Destroyed Vehicles", "Are you sure you want to delete all destroyed vehicles?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmDeleteVehicleDestroyed_Yes" );
	}

	void ConfirmDeleteVehicleDestroyed_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleDestroyed();
	}

	void OnClick_DeleteVehicleUnclaimed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Delete Unclaimed Vehicles", "Are you sure you want to delete all unclaimed vehicles?", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmDeleteVehicleUnclaimed_Yes" );
	}

	void ConfirmDeleteVehicleUnclaimed_Yes( JMConfirmation confirmation = NULL )
	{
		OnConfirmation_DeleteVehicleUnclaimed();
	}

	void OnConfirmation_DeleteVehicle()
	{
		m_Module.DeleteVehicle( m_CurrentVehicle.m_NetworkIDLow, m_CurrentVehicle.m_NetworkIDHigh );
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

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );

		// OnResize fires before the form is rendered, where GetScreenSize
		// returns 0 and Layout() no-ops. Re-flow one tick after it is up.
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( _LayoutSearchRow, 34 );
	}

	override void OnHide()
	{
		super.OnHide();

		HideHoverInfo();
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

		//! Ahead of the marker-placement early-out below: the hover poll is
		//! what keeps the panel's speed and crew readouts honest, and a still
		//! map is exactly when a vehicle is most likely to be the thing moving.
		if ( m_HoverActive && g_Game.GetTime() >= m_HoverNextPollAt )
			PollHoveredVehicle();

		if ( !m_Map )
			return;

		//! Markers only exist on the map tab; off it there is nothing to place.
		if ( GetActiveTabIndex() != TAB_MAP )
			return;

		m_Map.TickMarkers();
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( !m_Map )
			return;

		if ( usePlayerPosition )
		{
			PlayerBase player;
			float scale;
			if ( Class.CastTo( player, GetGame().GetPlayer() ) && !player.GetLastMapInfo( scale, mapPosition ) )
			{
				scale = 0.33;
				mapPosition = player.GetWorldPosition();
			}

			m_Map.SetScale( scale );
		}

		m_Map.SetCenter( mapPosition );
	}
};
