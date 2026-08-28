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
	protected Widget m_VehiclesListContent;

	protected UIActionTabs m_Tabs;
	protected Widget m_TabMapContent;
	protected Widget m_TabActionsContent;
	protected Widget m_TabInfoContent;

	static const int TAB_MAP     = 0;
	static const int TAB_ACTIONS = 1;
	static const int TAB_INFO    = 2;

	protected UIActionImageButton m_RefreshButton;
	protected UIActionConfirmInline m_DeleteAllButton;
	protected UIActionConfirmInline m_DeleteDestroyedButton;
	protected UIActionConfirmInline m_DeleteUnclaimedButton;

	protected Widget m_MapWidgetPanel;
	protected MapWidget m_MapWidget;

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
	protected UIActionButton m_ReturnButton;

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
	protected UIActionText m_VehicleKeys;
	protected UIActionText m_VehicleInfoOwner;
	protected UIActionText m_VehicleLastDriverUID;
	protected UIActionText m_VehicleLastDriverSteam;
	protected UIActionText m_VehicleLastDriverGUID;
	protected UIActionText m_VehicleCovered;

	protected UIActionSearchBox m_SearchBar;

	protected ref array<ref JMVehiclesMapMarker> m_MapMarkers;
	protected ref array<ref JMVehiclesListEntry> m_VehicleEntries;

	// One shared Timer at 20 Hz repositions every marker. Replaces the old
	// per-marker 100 Hz timers - on 200-vehicle servers that's a drop from
	// 20,000 timer callbacks/sec to 20.
	protected ref Timer m_MarkerTickTimer;

	static const float MARKER_TICK_INTERVAL = 0.05;

	protected JMVehicleMetaData m_CurrentVehicle;
	protected bool m_IsInVehicleInfo = false;

	JMVehicleMetaData GetCurrentVehicle() { return m_CurrentVehicle; }
	protected string m_SearchFilter;

	void JMVehiclesForm()
	{
		m_MapMarkers     = new array<ref JMVehiclesMapMarker>;
		m_VehicleEntries = new array<ref JMVehiclesListEntry>;
		m_CopyButtonMap  = new map<Widget, ref UIActionText>;
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
		txt.SetWidth( 1.0 );

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
		Widget toolbar = layoutRoot.FindAnyWidget( "panel_left_top" );

		// Row 1 - search bar with icon-only refresh button
		Widget searchRow = UIActionManager.CreateWrapSpacer( toolbar, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			m_RefreshButton = UIActionManager.CreateRefreshButton( searchRow, this, "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH" );

			m_SearchBar = UIActionManager.CreateSearchBox( searchRow, this, "OnChange_Search", "Search" );
			if ( m_SearchBar )
			{
				m_SearchBar.SetWidth( 0.85 );
				m_SearchBar.SetTooltip( "Filter the list by vehicle name" );
			}

		// Row 2 - destructive bulk actions, one button per row (mirrors the event
		// manager layout: each action gets its own row at full width, labels stay
		// legible even on narrow panels). 1-col vertical grid = stacked rows.
		Widget bulkDeleteRow = UIActionManager.CreateGridSpacer( toolbar, 3, 1 );
			m_DeleteAllButton       = UIActionManager.CreateConfirmInline( bulkDeleteRow, "Delete All",       this, "OnClick_DeleteVehicleAll"       );
			m_DeleteDestroyedButton = UIActionManager.CreateConfirmInline( bulkDeleteRow, "Delete Destroyed", this, "OnClick_DeleteVehicleDestroyed" );
			m_DeleteUnclaimedButton = UIActionManager.CreateConfirmInline( bulkDeleteRow, "Delete Unclaimed", this, "OnClick_DeleteVehicleUnclaimed" );

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
		m_ListScroller        = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel_left_bottom" ) );
		m_VehiclesListContent = m_ListScroller.GetContentWidget();
	}

	//! Right-hand panel: a tab strip over Map / Actions / Info. The map lives in
	//! the layout because a MapWidget cannot be created from script; the other
	//! two tabs are scrollers built on first activation.
	protected void InitWidgetsRight()
	{
		m_MapWidgetPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_map_panel" ) );
		m_MapWidget      = MapWidget.Cast( layoutRoot.FindAnyWidget( "vehicles_map" ) );

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

		Widget gridOptB = UIActionManager.CreateGridSpacer( card.GetContent(), 1, 1 );
			m_ReturnButton = UIActionManager.CreateButton( gridOptB, "Return", this, "OnClick_Return" );
			m_ReturnButton.SetTooltip( "Go back to the map" );

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
		Widget gridInfoA = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 9, 1 );
			m_VehicleName           = CreateCopyableText( gridInfoA, "Name:", "Value" );
			m_VehicleClassName      = CreateCopyableText( gridInfoA, "ClassName:", "Value" );
			m_VehicleStatus         = CreateCopyableText( gridInfoA, "Status:", "Value" );
			m_VehicleType           = CreateCopyableText( gridInfoA, "Type:", "Value" );
			m_VehicleID             = CreateCopyableText( gridInfoA, "ID:", "Value" );
			m_VehiclePersistentIDAB = CreateCopyableText( gridInfoA, "ID AB:", "Value" );
			m_VehiclePersistentIDCD = CreateCopyableText( gridInfoA, "ID CD:", "Value" );
			m_VehiclePosition       = CreateCopyableText( gridInfoA, "Position:", "Value" );
			m_VehicleRotation       = CreateCopyableText( gridInfoA, "Rotation:", "Value" );

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

		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();

		if ( m_VehicleOptionsScroller )
			m_VehicleOptionsScroller.UpdateScroller();

		if ( m_VehicleInfoScroller )
			m_VehicleInfoScroller.UpdateScroller();
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		BuildTabIfNeeded( GetActiveTabIndex() );

		UpdateUI();
	}

	//! Actions and Info are meaningless with nothing picked, so they get the
	//! disable overlay. The map is a browser - it stays usable either way.
	void UpdateUI()
	{
		int sel = GetActiveTabIndex();

		if ( !m_CurrentVehicle && ( sel == TAB_ACTIONS || sel == TAB_INFO ) )
			HideUI();
		else
			ShowUI();
	}

	void LoadVehicles()
	{
		m_MapMarkers.Clear();
		m_VehicleEntries.Clear();

		if ( !m_Module )
			return;

		auto vehicles = m_Module.GetServerVehicles();
		TStringArray vehicleEntryLabels = {};

		foreach ( JMVehicleMetaData currentVehicle: vehicles )
		{
			if ( m_SearchFilter != "" )
			{
				string displayNameLower = currentVehicle.m_DisplayName;
				displayNameLower.ToLower();
				string filterLower = m_SearchFilter;
				filterLower.ToLower();
				if ( displayNameLower.IndexOf( filterLower ) == -1 )
					continue;
			}

			int color;
			string marker;
			GetVehicleTypeInfo( currentVehicle.m_VehicleType, color, marker );

			JMVehiclesMapMarker vehicleMapMarker = new JMVehiclesMapMarker( m_MapWidgetPanel, m_MapWidget, currentVehicle.m_Position, color, marker, currentVehicle, this );
			m_MapMarkers.Insert( vehicleMapMarker );

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
	}

	private void GetVehicleTypeInfo( int type, out int color, out string marker )
	{
		switch ( type )
		{
			default:
			case JMVT_CAR:
				color  = ARGB( 255, 243, 156, 18 );
				marker = "set:dayz_gui image:stance_car";
			break;
			case JMVT_BOAT:
				color  = ARGB( 255, 243, 18, 156 );
				marker = "Boat";
			break;
			case JMVT_HELICOPTER:
				color  = ARGB( 255, 18, 156, 243 );
				marker = "Helicopter";
			break;
			case JMVT_PLANE:
				color  = ARGB( 255, 18, 243, 156 );
				marker = "Plane";
			break;
			case JMVT_BIKE:
				color  = ARGB( 255, 243, 156, 100 );
				marker = "Bike";
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

		// If the user is currently looking at this vehicle's detail panel,
		// refresh it in place with the new data.
		if ( m_IsInVehicleInfo && m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow == fresh.m_NetworkIDLow && m_CurrentVehicle.m_NetworkIDHigh == fresh.m_NetworkIDHigh )
		{
			SetVehicleInfo( fresh );
			return;
		}

		// Otherwise rebuild the list view - module already patched its cache.
		LoadVehicles();
	}

	void OnDeltaRemove( int netLow, int netHigh )
	{
		if ( m_IsInVehicleInfo && m_CurrentVehicle && m_CurrentVehicle.m_NetworkIDLow  == netLow && m_CurrentVehicle.m_NetworkIDHigh == netHigh )
		{
			BackToList();
		}

		LoadVehicles();
	}

	void SetVehicleInfo( JMVehicleMetaData vehicle )
	{
		m_IsInVehicleInfo = true;
		m_CurrentVehicle  = vehicle;

		// Picking a vehicle moves to its actions; the markers stay on the map
		// tab either way, which is why LoadVehicles no longer skips building
		// them while a vehicle is selected.
		if ( m_Tabs && GetActiveTabIndex() == TAB_MAP )
			m_Tabs.SetSelection( TAB_ACTIONS );

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

	void ShowMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
			m_MapMarkers.Get( i ).ShowMarker();
	}

	void HideMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
			m_MapMarkers.Get( i ).HideMarker();
	}

	void BackToList()
	{
		m_IsInVehicleInfo = false;
		m_CurrentVehicle  = NULL;

		if ( m_Tabs )
			m_Tabs.SetSelection( TAB_MAP );

		ShowMapMarkers();

		UpdateUI();
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

	void OnClick_Return( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		BackToList();
	}

	void OnClick_CopyInfo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !action )
			return;

		UIActionText txt = m_CopyButtonMap.Get( action.GetLayoutRoot() );
		if ( txt )
			g_Game.CopyToClipboard( txt.GetText() );
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
			OnConfirmation_DeleteVehicle();
	}

	void OnClick_DeleteVehicleAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		OnConfirmation_DeleteVehicleAll();
	}

	void OnClick_DeleteVehicleDestroyed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		OnConfirmation_DeleteVehicleDestroyed();
	}

	void OnClick_DeleteVehicleUnclaimed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

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

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefreshVehicles();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );

		StartMarkerTick();
	}

	override void OnHide()
	{
		StopMarkerTick();
		super.OnHide();
	}

	protected void StartMarkerTick()
	{
		if ( !m_MarkerTickTimer )
			m_MarkerTickTimer = new Timer( CALL_CATEGORY_GUI );

		if ( !m_MarkerTickTimer.IsRunning() )
			m_MarkerTickTimer.Run( MARKER_TICK_INTERVAL, this, "TickMarkers", NULL, true );
	}

	protected void StopMarkerTick()
	{
		if ( m_MarkerTickTimer && m_MarkerTickTimer.IsRunning() )
			m_MarkerTickTimer.Stop();
	}

	void TickMarkers()
	{
		// Form-level tick: reposition every marker once per interval instead of
		// letting each marker run its own Timer.
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
		{
			JMVehiclesMapMarker m = m_MapMarkers[i];
			if ( m )
				m.UpdatePosition();
		}
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( !m_MapWidget )
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

			m_MapWidget.SetScale( scale );
		}

		m_MapWidget.SetMapPos( mapPosition );
	}
};
