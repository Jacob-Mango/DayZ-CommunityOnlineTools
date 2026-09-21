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
	protected UIActionImageButton m_RefreshButton;
	protected ref UIActionFlexRow m_SearchRow;
	protected UIActionSearchBox m_SearchBar;
	protected ref array<ref UIActionBase> m_BulkButtons;
	protected ref array<ref UIActionButton> m_FilterButtons;
	protected ref array<string> m_FilterLabels;
	protected UIActionTabs m_Tabs;

	//! Tab ids as the tab strip handed them out from AddTab() - never numbered here.
	protected int m_TabIdMap     = -1;
	protected int m_TabIdActions = -1;
	protected int m_TabIdInfo    = -1;

	// Right panel - the layout panels the three tabs live in. Each tab's widgets
	// are owned by its own class: JMEntityManagerFormTabMap / Actions / Info.
	protected Widget m_MapWidgetPanel;
	protected Widget m_OptionsPanel;
	protected Widget m_InfoPanel;
	ref JMEntityManagerFormTabMap     m_TabMap;
	ref JMEntityManagerFormTabActions m_TabActions;
	ref JMEntityManagerFormTabInfo    m_TabInfo;

	// Keep action descriptors alive so UserData weak refs stay valid.
	protected ref array<ref JMEntityAction> m_Actions;

	// State
	protected ref array<ref JMEntityManagerListEntry> m_ListEntries;
	protected JMEntityMetaData m_CurrentEntity;
	protected bool   m_IsInEntityInfo = false;
	protected string m_SearchFilter;
	protected int    m_FilterIndex = 0;

	void JMEntityManagerForm()
	{
		m_ListEntries   = new array<ref JMEntityManagerListEntry>;
		m_BulkButtons   = new array<ref UIActionBase>;
		m_Actions       = new array<ref JMEntityAction>;
		m_FilterButtons = new array<ref UIActionButton>;
		m_FilterLabels  = new array<string>;
	}

	//! The adapter's action descriptors. Public for the Actions tab.
	array<ref JMEntityAction> GetActions()
	{
		return m_Actions;
	}

	//! Public for the tab classes.
	JMEntityManagerAdapter GetAdapter()
	{
		if ( m_Module )
			return m_Module.GetAdapter();
		return null;
	}

	JMEntityMetaData GetCurrentEntity()
	{
		return m_CurrentEntity;
	}

	//! Public for the tab classes, which run their commands through the module.
	JMEntityManagerModule GetModule()
	{
		return m_Module;
	}

	void SetEntityInfo( JMEntityMetaData entity )
	{
		m_IsInEntityInfo = true;
		m_CurrentEntity  = entity;

		// Picking an entity moves to its actions; the markers stay on the map
		// tab either way.
		if ( m_Tabs && GetActiveTabIndex() == m_TabIdMap )
			m_Tabs.SetSelection( m_TabIdActions );

		UpdateUI();

		// The Info tab may not have been built yet - it fills itself in from
		// m_CurrentEntity when it is.
		if ( !m_TabInfo || !m_TabInfo.IsBuilt() )
			return;

		m_TabInfo.ShowEntity( entity );

		if ( m_TabActions )
			m_TabActions.UpdateScroller();
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnCreate()
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

		// Grid: search/refresh row + N bulk + optional filter row
		Widget grid = UIActionManager.CreateGridSpacer( listButtons, 1 + bulkCount + filterRows, 1 );
			JMSearchRow searchToolbar = UIActionManager.CreateSearchFlexRow( grid, "Search", this, "OnChange_Search", "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH", 30 );

			m_SearchRow = searchToolbar.Row;
			m_RefreshButton = searchToolbar.Refresh;

			m_SearchBar = searchToolbar.Search;
			if ( m_SearchBar )
				m_SearchBar.SetTooltip( "#STR_COT_ENTITYMANAGER_FILTER_THE_LIST_BY_NAME" );

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
					if ( bulkAct.m_Permission != "" )
						UpdatePermission( b, bulkAct.m_Permission );
					m_BulkButtons.Insert( b );
				}
				else
				{
					UIActionButton b2 = UIActionManager.CreateButton( grid, bulkAct.m_Label, this, "OnBulkClick" );
					b2.SetUserData( bulkAct );
					if ( bulkAct.m_Tooltip != "" )
						b2.SetTooltip( bulkAct.m_Tooltip );
					if ( bulkAct.m_Permission != "" )
						UpdatePermission( b2, bulkAct.m_Permission );
					m_BulkButtons.Insert( b2 );
				}
			}

			// Filter row - one button per adapter filter, plus an "All" entry at slot 0.
			if ( m_FilterLabels.Count() > 0 )
			{
				int filterCols = 1 + m_FilterLabels.Count();
				Widget filterRow = UIActionManager.CreateGridSpacer( grid, 1, filterCols );

				UIActionButton fAll = UIActionManager.CreateButton( filterRow, "#STR_COT_GENERIC_ALL", this, "OnFilterClick" );
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

		m_OptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "entity_options_panel" ) );
		m_InfoPanel    = Widget.Cast( layoutRoot.FindAnyWidget( "entity_info_panel" ) );


		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );

		m_TabIdMap = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_MAP", JMConstants.Lucide( "map" ), m_MapWidgetPanel );
		m_TabIdActions = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_ACTIONS", JMConstants.Lucide( "zap" ), m_OptionsPanel );
		m_TabIdInfo = m_Tabs.AddTab( "#STR_COT_VEHICLE_TAB_INFO", JMConstants.Lucide( "info" ), m_InfoPanel );

		DeclareTabs( 3 );

		m_Tabs.SetSelection( m_TabIdMap, false );

		InitTabFocus( m_TabIdMap );

		UpdateUI();
	}

	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdMap )
		{
			m_TabMap = new JMEntityManagerFormTabMap( this );
			RegisterTab( m_TabIdMap, m_TabMap );
			m_TabMap.OnCreate( panel );
		}
		else if ( tab == m_TabIdActions )
		{
			m_TabActions = new JMEntityManagerFormTabActions( this );
			RegisterTab( m_TabIdActions, m_TabActions );
			m_TabActions.OnCreate( panel );
		}
		else if ( tab == m_TabIdInfo )
		{
			m_TabInfo = new JMEntityManagerFormTabInfo( this );
			RegisterTab( m_TabIdInfo, m_TabInfo );
			m_TabInfo.OnCreate( panel );
		}

		// A tab built after an entity was already picked has missed the pass
		// that would have filled it in.
		if ( m_CurrentEntity )
			SetEntityInfo( m_CurrentEntity );
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
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

		if ( !m_CurrentEntity && ( sel == m_TabIdActions || sel == m_TabIdInfo ) )
			SetPanelEnabled( false );
		else
			SetPanelEnabled( true );
	}

	void LoadEntities()
	{
		// Don't churn the list while the user is interacting with one entity -
		// it leaves the action buttons in a weird state and makes follow-up
		// clicks miss until the user does a manual refresh.
		if ( m_IsInEntityInfo )
			return;

		if ( m_TabMap )
			m_TabMap.ClearMarkers();

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

			if ( !m_IsInEntityInfo && m_TabMap )
				m_TabMap.AddEntityMarker( e );

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
			if ( m_TabInfo )
				m_TabInfo.UpdateFields( fresh );
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

	// Adapters can register widgets they create in OnBuildInfoExtras so they
	// get torn down when the user picks a different entity.
	void TrackDynamicWidget( Widget w )
	{
		if ( m_TabInfo )
			m_TabInfo.TrackDynamicWidget( w );
	}

	void SyncAndRefresh()
	{
		if ( m_Module )
			m_Module.RequestServerEntities();
	}

	void ShowMapMarkers()
	{
		if ( m_TabMap )
			m_TabMap.ShowMapMarkers();
	}

	void HideMapMarkers()
	{
		if ( m_TabMap )
			m_TabMap.HideMapMarkers();
	}

	void BackToList()
	{
		m_IsInEntityInfo = false;
		m_CurrentEntity  = NULL;

		if ( m_TabInfo )
			m_TabInfo.ClearDynamicWidgets();

		if ( m_Tabs )
			m_Tabs.SetSelection( m_TabIdMap );

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

		SyncAndRefresh();
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

		ResizeTabs( w, h );
	}

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefresh();

		DeferCall( "UpdateMapPosition", 34, false, new Param2< bool, vector >( true, vector.Zero ) );

		StartMarkerTick();
	}

	override void OnHide()
	{
		StopMarkerTick();
		super.OnHide();
	}

	protected void StartMarkerTick()
	{
		if ( m_TabMap )
			m_TabMap.StartMarkerTick();
	}

	protected void StopMarkerTick()
	{
		if ( m_TabMap )
			m_TabMap.StopMarkerTick();
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( m_TabMap )
			m_TabMap.UpdateMapPosition( usePlayerPosition, mapPosition );
	}
}
