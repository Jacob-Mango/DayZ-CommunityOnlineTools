// =============================================================================
//  JMEntityManagerForm
//
//  Generic entity-manager form. Delegates all data and per-action logic to a
//  JMEntityManagerAdapter supplied by the owning module. The UI layout is
//  shared across all concrete managers (vehicles, garage, territories, events).
//
//  Layout: archetype A (split-pane roster + tabbed detail). Left 35% is a
//  searchable roster with a toolbar block above it; right 65% is a tab strip
//  over Map / Actions / Info. Picking an entity moves to its Actions tab; the
//  map keeps its markers either way. Actions and Info are covered by the
//  disable overlay while nothing is selected.
// =============================================================================
class JMEntityManagerForm: JMFormBase
{
	protected JMEntityManagerModule m_Module;

	// Left panel
	protected UIActionScroller m_ListScroller;
	protected Widget m_ListContent;
	protected UIActionButton m_RefreshButton;
	protected UIActionSearchBox m_SearchBar;
	protected ref array<ref UIActionBase> m_BulkButtons;
	protected ref array<ref UIActionButton> m_FilterButtons;
	protected ref array<string> m_FilterLabels;

	protected UIActionTabs m_Tabs;

	static const int TAB_MAP     = 0;
	static const int TAB_ACTIONS = 1;
	static const int TAB_INFO    = 2;

	// Right panel
	protected Widget m_MapWidgetPanel;
	protected MapWidget m_MapWidget;

	protected Widget m_OptionsPanel;
	protected UIActionScroller m_OptionsScroller;
	protected ref map<string, UIActionBase> m_ActionButtons;
	protected UIActionButton m_ReturnButton;

	protected Widget m_InfoPanel;
	protected UIActionScroller m_InfoScroller;
	protected UIActionText m_InfoName;
	protected UIActionText m_InfoClassName;
	protected UIActionText m_InfoStatus;
	protected UIActionText m_InfoPosition;
	protected ref array<ref UIActionText> m_ExtraRows;
	protected ref array<Widget> m_DynamicWidgets;  // widgets added each SetEntityInfo call

	// Keep action descriptors alive so UserData weak refs stay valid.
	protected ref array<ref JMEntityAction> m_Actions;

	// State
	protected ref array<ref JMEntityManagerMapMarker> m_MapMarkers;
	protected ref array<ref JMEntityManagerListEntry> m_ListEntries;
	protected JMEntityMetaData m_CurrentEntity;
	protected bool   m_IsInEntityInfo = false;
	protected string m_SearchFilter;
	protected int    m_FilterIndex = 0;

	// Consolidated 20 Hz marker reposition tick (replaces per-marker timers).
	protected ref Timer m_MarkerTickTimer;

	static const float MARKER_TICK_INTERVAL = 0.05;

	void JMEntityManagerForm()
	{
		m_MapMarkers    = new array<ref JMEntityManagerMapMarker>;
		m_ListEntries   = new array<ref JMEntityManagerListEntry>;
		m_BulkButtons   = new array<ref UIActionBase>;
		m_ActionButtons = new map<string, UIActionBase>;
		m_ExtraRows     = new array<ref UIActionText>;
		m_Actions       = new array<ref JMEntityAction>;
		m_FilterButtons = new array<ref UIActionButton>;
		m_FilterLabels  = new array<string>;
		m_DynamicWidgets = new array<Widget>;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	protected JMEntityManagerAdapter GetAdapter()
	{
		if ( m_Module )
			return m_Module.GetAdapter();
		return null;
	}

	override void OnInit()
	{
		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip     = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );

		Widget listButtons = layoutRoot.FindAnyWidget( "panel_left_top" );

		JMEntityManagerAdapter adapter = GetAdapter();

		m_Actions.Clear();
		if ( adapter )
			adapter.GetActions( m_Actions );
		array<ref JMEntityAction> actions = m_Actions;

		m_FilterLabels.Clear();
		if ( adapter )
			adapter.GetFilterLabels( m_FilterLabels );

		// Count bulk actions for grid sizing
		int bulkCount = 0;
		foreach ( JMEntityAction a: actions )
		{
			if ( a.m_IsBulk ) bulkCount++;
		}

		// Filter row (one extra row if the adapter exposes filters)
		int filterRows = 0;
		if ( m_FilterLabels.Count() > 0 )
			filterRows = 1;

		// Grid: search + refresh + N bulk + optional filter row
		Widget grid = UIActionManager.CreateGridSpacer( listButtons, 2 + bulkCount + filterRows, 1 );
			m_SearchBar     = UIActionManager.CreateSearchBox( grid, this, "OnChange_Search", "Search" );
			if ( m_SearchBar )
				m_SearchBar.SetTooltip( "Filter the list by name" );
			m_RefreshButton = UIActionManager.CreateButton( grid, "#STR_COT_GENERIC_REFRESH", this, "OnClick_Refresh" );
			m_RefreshButton.SetIcon( JMConstants.Lucide( "refresh-cw" ) );
			m_RefreshButton.SetTooltip( "#STR_COT_GENERIC_REFRESH" );

			foreach ( JMEntityAction bulkAct: actions )
			{
				if ( !bulkAct.m_IsBulk )
					continue;

				if ( bulkAct.m_IsDestructive )
				{
					UIActionConfirmInline b = UIActionManager.CreateConfirmInline( grid, bulkAct.m_Label, this, "OnBulkClick" );
					UIActionIconGrid.ApplyDeletePreset( b );
					b.SetUserData( bulkAct );
					if ( bulkAct.m_Tooltip != "" )
						b.SetTooltip( bulkAct.m_Tooltip );
					m_BulkButtons.Insert( b );
				}
				else
				{
					UIActionButton b2 = UIActionManager.CreateButton( grid, bulkAct.m_Label, this, "OnBulkClick" );
					b2.SetUserData( bulkAct );
					if ( bulkAct.m_Tooltip != "" )
						b2.SetTooltip( bulkAct.m_Tooltip );
					m_BulkButtons.Insert( b2 );
				}
			}

			// Filter row - one button per adapter filter, plus an "All" entry at slot 0.
			if ( m_FilterLabels.Count() > 0 )
			{
				int filterCols = 1 + m_FilterLabels.Count();
				Widget filterRow = UIActionManager.CreateGridSpacer( grid, 1, filterCols );

				UIActionButton fAll = UIActionManager.CreateButton( filterRow, "All", this, "OnFilterClick" );
				fAll.SetUserData( new JMIntBox( 0 ) );
				m_FilterButtons.Insert( fAll );

				for ( int fi = 0; fi < m_FilterLabels.Count(); fi++ )
				{
					UIActionButton fb = UIActionManager.CreateButton( filterRow, m_FilterLabels[fi], this, "OnFilterClick" );
					fb.SetUserData( new JMIntBox( fi + 1 ) );
					m_FilterButtons.Insert( fb );
				}
			}

		// The roster itself. JMEntityManagerListEntry parents into whatever
		// widget it is handed, so the scroller's content widget is all it needs.
		m_ListScroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel_left_bottom" ) );
		m_ListContent  = m_ListScroller.GetContentWidget();

		InitWidgetsRight();
	}

	//! Right-hand panel: a tab strip over Map / Actions / Info. The map lives in
	//! the layout because a MapWidget cannot be created from script; the other
	//! two tabs are scrollers built on first activation.
	protected void InitWidgetsRight()
	{
		m_MapWidgetPanel = Widget.Cast( layoutRoot.FindAnyWidget( "entity_map_panel" ) );
		m_MapWidget      = MapWidget.Cast( layoutRoot.FindAnyWidget( "entity_map" ) );

		m_OptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "entity_options_panel" ) );
		m_InfoPanel    = Widget.Cast( layoutRoot.FindAnyWidget( "entity_info_panel" ) );

		ref array<string> tabLabels = { "#STR_COT_VEHICLE_TAB_MAP", "#STR_COT_VEHICLE_TAB_ACTIONS", "#STR_COT_VEHICLE_TAB_INFO" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "map" ), JMConstants.Lucide( "zap" ), JMConstants.Lucide( "info" ) };

		m_Tabs = UIActionManager.CreateTabs( m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_MapWidgetPanel );
		m_Tabs.AddContent( m_OptionsPanel );
		m_Tabs.AddContent( m_InfoPanel );

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

		// A tab built after an entity was already picked has missed the pass
		// that would have filled it in.
		if ( m_CurrentEntity )
			SetEntityInfo( m_CurrentEntity );
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	protected void InitWidgetsActions()
	{
		// m_Actions was filled in OnInit and the bulk buttons hold weak UserData
		// refs into it - do not clear or refetch it here.
		array<ref JMEntityAction> actions = m_Actions;

		m_OptionsScroller = UIActionManager.CreateScroller( m_OptionsPanel );
		Widget optContent = m_OptionsScroller.GetContentWidget();

		int perEntityCount = 0;
		foreach ( JMEntityAction ca: actions )
		{
			if ( !ca.m_IsBulk ) perEntityCount++;
		}

		UIActionCard optCard = UIActionManager.CreateCard( optContent, "#STR_COT_VEHICLE_OPTIONS" );
		Widget optGrid = UIActionManager.CreateGridSpacer( optCard.GetContent(), perEntityCount + 1, 1 );
		foreach ( JMEntityAction act: actions )
		{
			if ( act.m_IsBulk )
				continue;

			if ( act.m_IsDestructive )
			{
				UIActionConfirmInline btn = UIActionManager.CreateConfirmInline( optGrid, act.m_Label, this, "OnEntityClick" );
				UIActionIconGrid.ApplyDeletePreset( btn );
				btn.SetUserData( act );
				if ( act.m_Tooltip != "" )
					btn.SetTooltip( act.m_Tooltip );
				m_ActionButtons.Set( act.m_Id, btn );
			}
			else
			{
				UIActionButton btn2 = UIActionManager.CreateButton( optGrid, act.m_Label, this, "OnEntityClick" );
				btn2.SetUserData( act );
				if ( act.m_Tooltip != "" )
					btn2.SetTooltip( act.m_Tooltip );
				m_ActionButtons.Set( act.m_Id, btn2 );
			}
		}

		Widget retGrid = UIActionManager.CreateGridSpacer( optCard.GetContent(), 1, 1 );
		m_ReturnButton = UIActionManager.CreateButton( retGrid, "Return", this, "OnClick_Return" );
		m_ReturnButton.SetTooltip( "Go back to the map" );

		m_OptionsScroller.UpdateScroller();
	}

	protected void InitWidgetsInfo()
	{
		m_InfoScroller = UIActionManager.CreateScroller( m_InfoPanel );
		Widget infoContent = m_InfoScroller.GetContentWidget();

		UIActionCard infoCard = UIActionManager.CreateCard( infoContent, "#STR_COT_VEHICLE_INFORMATION_TITLE" );
		Widget infoGrid = UIActionManager.CreateGridSpacer( infoCard.GetContent(), 4, 1 );
			m_InfoName      = UIActionManager.CreateText( infoGrid, "Name:", "" );
			m_InfoClassName = UIActionManager.CreateText( infoGrid, "ClassName:", "" );
			m_InfoStatus    = UIActionManager.CreateText( infoGrid, "Status:", "" );
			m_InfoPosition  = UIActionManager.CreateText( infoGrid, "Position:", "" );

		m_InfoScroller.UpdateScroller();
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

		if ( !m_CurrentEntity && ( sel == TAB_ACTIONS || sel == TAB_INFO ) )
			HideUI();
		else
			ShowUI();
	}

	void LoadEntities()
	{
		// Don't churn the list while the user is interacting with one entity -
		// it leaves the action buttons in a weird state and makes follow-up
		// clicks miss until the user does a manual refresh.
		if ( m_IsInEntityInfo )
			return;

		m_MapMarkers.Clear();
		m_ListEntries.Clear();

		if ( !m_Module )
			return;

		JMEntityManagerAdapter adapter = GetAdapter();
		array<ref JMEntityMetaData> entities = m_Module.GetEntities();
		TStringArray entryLabels = {};

		foreach ( JMEntityMetaData e: entities )
		{
			if ( m_SearchFilter != "" )
			{
				string name = e.m_DisplayName;
				name.ToLower();
				string q = m_SearchFilter;
				q.ToLower();
				if ( name.IndexOf( q ) == -1 )
					continue;
			}

			if ( adapter && !adapter.MatchesFilter( e, m_FilterIndex ) )
				continue;

			if ( !m_IsInEntityInfo )
			{
				JMEntityManagerMapMarker m = new JMEntityManagerMapMarker( m_MapWidgetPanel, m_MapWidget, e, this );
				m_MapMarkers.Insert( m );
			}

			JMEntityManagerListEntry le = new JMEntityManagerListEntry( m_ListContent, this, e );
			m_ListEntries.Insert( le );

			int idx = 0;
			foreach ( string existing: entryLabels )
			{
				if ( JMStatics.StrCaseCmp( le.GetLabel(), existing ) < 0 )
					break;
				idx++;
			}
			entryLabels.InsertAt( le.GetLabel(), idx );
		}

		int sort;
		foreach ( auto listEntry: m_ListEntries )
		{
			sort = entryLabels.Find( listEntry.GetLabel() );
			if ( sort > -1 )
				listEntry.SetSort( sort, false );
		}
	}

	// -------------------------------------------------------------------------
	//  Delta hooks - called by JMEntityManagerModule when a single-entity
	//  upsert or remove arrives from the server after an admin action.
	//
	//  Implementation goal: avoid the full LoadEntities churn. If the user is
	//  currently inspecting an entity, refresh just the info panel in place;
	//  otherwise let LoadEntities run normally (it's gated behind
	//  m_IsInEntityInfo anyway, so the upsert arrives into the cached list
	//  and the next manual Refresh / filter click picks it up visually).
	// -------------------------------------------------------------------------

	void OnDeltaUpsert( JMEntityMetaData fresh )
	{
		if ( !fresh )
			return;

		// If the user is looking at this specific entity, refresh its info.
		if ( m_IsInEntityInfo && m_CurrentEntity && m_CurrentEntity.m_Id == fresh.m_Id )
		{
			// Swap pointer so future actions use the updated data.
			m_CurrentEntity = fresh;
			m_InfoName.SetText( fresh.m_DisplayName );
			m_InfoClassName.SetText( fresh.m_ClassName );
			m_InfoStatus.SetText( fresh.m_StatusText );
			m_InfoPosition.SetText( fresh.m_Position.ToString() );
			return;
		}

		// Otherwise rebuild the list view - cheap because m_Entities already
		// contains the updated entry (module.RPC_SendUpsert patched it in
		// place before calling us).
		if ( !m_IsInEntityInfo )
			LoadEntities();
	}

	void OnDeltaRemove( string id, int netLow, int netHigh )
	{
		if ( m_IsInEntityInfo && m_CurrentEntity && m_CurrentEntity.m_Id == id )
		{
			// Entity we were inspecting vanished - go back to the list.
			BackToList();
		}

		if ( !m_IsInEntityInfo )
			LoadEntities();
	}

	void SetEntityInfo( JMEntityMetaData entity )
	{
		m_IsInEntityInfo = true;
		m_CurrentEntity  = entity;

		// Picking an entity moves to its actions; the markers stay on the map
		// tab either way.
		if ( m_Tabs && GetActiveTabIndex() == TAB_MAP )
			m_Tabs.SetSelection( TAB_ACTIONS );

		UpdateUI();

		// The Info tab may not have been built yet - it fills itself in from
		// m_CurrentEntity when it is.
		if ( !m_InfoName || !m_InfoScroller )
			return;

		m_InfoName.SetText( entity.m_DisplayName );
		m_InfoClassName.SetText( entity.m_ClassName );
		m_InfoStatus.SetText( entity.m_StatusText );
		m_InfoPosition.SetText( entity.m_Position.ToString() );

		// Tear down any widgets we added on the previous entity click.
		ClearDynamicWidgets();

		Widget infoContent = m_InfoScroller.GetContentWidget();

		JMEntityManagerAdapter adapter = GetAdapter();
		array<string> keys = new array<string>;
		map<string, string> vals = new map<string, string>;
		if ( adapter )
			adapter.GetInfoRows( entity, keys, vals );

		if ( keys && keys.Count() > 0 )
		{
			Widget extraGrid = UIActionManager.CreateGridSpacer( infoContent, keys.Count(), 1 );
			m_DynamicWidgets.Insert( extraGrid );
			foreach ( string k: keys )
			{
				UIActionText t = UIActionManager.CreateText( extraGrid, k + ":", vals.Get( k ) );
				m_ExtraRows.Insert( t );
			}
		}

		if ( adapter )
			adapter.OnBuildInfoExtras( infoContent, entity, this );

		m_InfoScroller.UpdateScroller();
		m_OptionsScroller.UpdateScroller();
	}

	// Adapters can register widgets they create in OnBuildInfoExtras so they
	// get torn down when the user picks a different entity.
	void TrackDynamicWidget( Widget w )
	{
		if ( w )
			m_DynamicWidgets.Insert( w );
	}

	protected void ClearDynamicWidgets()
	{
		m_ExtraRows.Clear();
		foreach ( Widget w: m_DynamicWidgets )
		{
			if ( w )
				w.Unlink();
		}
		m_DynamicWidgets.Clear();
	}

	void SyncAndRefresh()
	{
		if ( m_Module )
			m_Module.RequestServerEntities();
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
		m_IsInEntityInfo = false;
		m_CurrentEntity  = NULL;

		ClearDynamicWidgets();

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

		LoadEntities();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_RefreshButton.TriggerSpin( 2 );
		SyncAndRefresh();
	}

	void OnClick_Return( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		BackToList();
	}

	void OnFilterClick( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		Class data;
		action.GetUserData( data );

		JMIntBox box;
		if ( !Class.CastTo( box, data ) )
			return;

		m_FilterIndex = box.Value;
		LoadEntities();
	}

	// Dispatched from every per-entity action button.
	void OnEntityClick( UIEvent eid, UIActionBase action )
	{
		Class data;
		action.GetUserData( data );

		JMEntityAction act;
		if ( !Class.CastTo( act, data ) )
			return;

		// Destructive = ConfirmInline fires CHANGE on confirm; others fire CLICK.
		if ( act.m_IsDestructive && eid != UIEvent.CHANGE )
			return;
		if ( !act.m_IsDestructive && eid != UIEvent.CLICK )
			return;

		if ( !m_CurrentEntity )
			return;

		m_Module.RequestAction( act.m_Id, m_CurrentEntity );

		// Destructive actions return to list by default.
		if ( act.m_IsDestructive )
			BackToList();
	}

	// Dispatched from every bulk action button (header row).
	void OnBulkClick( UIEvent eid, UIActionBase action )
	{
		Class data;
		action.GetUserData( data );

		JMEntityAction act;
		if ( !Class.CastTo( act, data ) )
			return;

		if ( act.m_IsDestructive && eid != UIEvent.CHANGE )
			return;
		if ( !act.m_IsDestructive && eid != UIEvent.CLICK )
			return;

		m_Module.RequestAction( act.m_Id, null );
	}

	override void OnResize( float w, float h )
	{
		PinRightPanelGeometry( h );

		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();

		if ( m_OptionsScroller )
			m_OptionsScroller.UpdateScroller();

		if ( m_InfoScroller )
			m_InfoScroller.UpdateScroller();
	}

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefresh();

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
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
		{
			JMEntityManagerMapMarker m = m_MapMarkers[i];
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
}
