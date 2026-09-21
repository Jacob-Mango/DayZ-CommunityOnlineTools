//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

class JMPlayerForm: JMFormBase
{
	UIActionScroller m_ActionListScroller;

	//! protected so modded-class fragments (EgoLand tab injection) can append
	//! new tab panels onto the scroller without a fragile widget-tree walk.
	protected Widget m_ActionsWrapper;

	//! The right-hand panel is a tab strip over a scroller, not one long
	//! scroller. The strip has to sit OUTSIDE the scroller or it scrolls away
	//! with the content, which is why player_form.layout splits panel_right
	//! into panel_right_tabs and panel_right_content.
	//! protected so modded-class fragments can call AddTab / AddPanel / SetSelection.
	protected UIActionTabs m_Tabs;

	//! Tab ids, exactly as the tab strip handed them out from AddTab() - nothing in this
	//! form numbers a tab itself, so a mod that appends or reorders tabs cannot collide
	//! with, or be off by one from, these. -1 until InitWidgetsRight has run.
	protected int m_TabIdGeneral;
	protected int m_TabIdPosition;
	protected int m_TabIdInventory;
	protected int m_TabIdStatistics;
	protected int m_TabIdActions;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline) re-sorts these widgets via
	//! GetLayoutRoot().SetSort() when injecting its faction/reputation rows.
	//! A modded class cannot touch a private member of the class it mods.
	protected UIActionImageButton m_ApplyStats;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm` uses
	//! m_Stamina.GetLayoutRoot().GetParent() as the anchor to inject its own rows.
	protected UIActionSlider m_Stamina;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerForm`
	//! (DayZExpansion_AI, DayZExpansion_Hardline, DayZExpansion_PersonalStorage)
	//! does, inside its InitActionWidgetsStats override:
	//!
	//!     m_ApplyStats.GetLayoutRoot().SetSort(sort + 1);
	//!     m_RefreshStats.GetLayoutRoot().SetSort(sort + 2);
	//!
	//! COT dropped this button at some point and never kept the name, so all
	//! three of those addons have failed to compile against COT ever since.
	//!
	//! It is a real button sitting beside Apply, NOT an alias of the Position
	//! tab's refresh icon: SetSort reorders a widget among ITS OWN siblings, so
	//! pointing this at the position toolbar would have Expansion silently
	//! shuffling the coordinate icons instead. Both now live in the variables
	//! card's title bar rather than in the slider grid, which makes Expansion's
	//! SetSort a harmless no-op - the pair is already last in reading order -
	//! and leaves the member names it compiles against intact.
	protected UIActionImageButton m_RefreshStats;

	// -- Statistics tab ----------------------------------------------------
	//! See JMPlayerFormTabStatistics.c - the whole tab lives there.
	protected ref JMPlayerFormTabStatistics m_TabStatisticsCtrl;

	// -- Tab controllers -----------------------------------------------------
	//! One class per tab, in its own file. Created the first time the tab is
	//! opened, so each is null until then - guard before use.
	ref JMPlayerFormTabGeneral   m_TabGeneralCtrl;
	ref JMPlayerFormTabPosition  m_TabPositionCtrl;
	ref JMPlayerFormTabInventory m_TabInventoryCtrl;
	ref JMPlayerFormTabActions   m_TabActionsCtrl;
	string m_LastSelectedGuid;

	//! public, not protected: DayZ-Expansion's `modded class JMPlayerForm` reads
	//! this to pull per-player faction/reputation for its injected rows, and the
	//! tab classes (composed, not inherited) need it too.
	JMPlayerInstance m_SelectedInstance;

	//! public, not protected: DayZ-Expansion's `modded class JMPlayerForm` calls
	//! m_Module.SetExpansionFaction() / SetExpansionReputation() on it, and the
	//! tab classes (composed, not inherited) need it too.
	JMPlayerModule m_Module;
	protected int m_LastChangeTime;
	protected bool m_AutoSelect = true;  //! Auto-select player when first shown

	//! The player list down the left of the form. See JMPlayerRoster.
	protected ref JMPlayerRoster m_Roster;

	//! The player list's right-click menu. See JMPlayerRowMenu.
	protected ref JMPlayerRowMenu m_RowMenu;

	//! protected, not private: modded forms (see Ex_ContextMenu.c) add entries to the
	//! menu after super.OnPlayerRow_RightClick(). JMPlayerRowMenu owns the menu and
	//! copies it here, together with the GUID it was opened on, each time it opens.
	protected UIActionContextMenu m_PlayerMenu;
	protected string              m_PlayerMenuGUID;

	void JMPlayerForm()
	{
		m_Roster  = new JMPlayerRoster( this );
		m_RowMenu = new JMPlayerRowMenu( this );
	}

	void ~JMPlayerForm()
	{
		if (!g_Game)
			return;

		// The tooltip is parented to the workspace, not to this form, so it
		// outlives the form unless it is taken down by hand.
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.ReleaseLocalEntities();
	}

	vector GetPosition(  )
	{
		if ( m_TabPositionCtrl )
			return m_TabPositionCtrl.GetPosition(  );

		return vector.Zero;
	}

	string GetPrimaryRole( JMPlayerInstance player )
	{
		return m_Roster.GetPrimaryRole( player );
	}

	bool IsRoleCollapsed( string role )
	{
		return m_Roster.IsRoleCollapsed( role );
	}

	//! DEPRECATED mirror - modded forms read these three members. The stats tab
	//! owns the real ones and hands them over once the card is built.
	void SetLegacyStatWidgets( UIActionSlider stamina, UIActionImageButton apply, UIActionImageButton refresh )
	{
		m_Stamina      = stamina;
		m_ApplyStats   = apply;
		m_RefreshStats = refresh;
	}

	//! Called by JMPlayerRowMenu each time it opens, so modded forms keep reading
	//! m_PlayerMenu and m_PlayerMenuGUID off the form.
	void SetPlayerMenu( UIActionContextMenu menu, string guid )
	{
		m_PlayerMenu     = menu;
		m_PlayerMenuGUID = guid;
	}

	void SetStatusRepairPermissions( UIActionContextMenu menu )
	{
		if ( m_RowMenu )
			m_RowMenu.SetStatusRepairPermissions( menu );
	}

	//! AddTab() itself lives on JMFormBase (JMFormBase.c in 5_Mission); these two
	//! tell it where this form's strip and tab panels are. Third-party
	//! AddTab(label, icon, "BuildFn") calls keep working unchanged.
	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	protected override Widget GetTabContentHost()
	{
		return m_ActionsWrapper;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		super.OnClientPermissionsUpdated();

		UpdateUI();
	}

	override void OnInit()
	{
		InitWidgetsLeft();
		InitWidgetsRight();

		RefreshStats(true);  //! Show correct state on reinit
	}

	override void OnResize( float w, float h )
	{
		PinRightPanelGeometry( h );

		if ( m_Roster ) m_Roster.UpdateScroller();
		if ( m_ActionListScroller ) m_ActionListScroller.UpdateScroller();

		//! Each tab re-fits itself (the inventory cell grid is positioned by hand).
		ResizeTabs( w, h );
	}

	protected void InitWidgetsRight()
	{
		m_RightPanel = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightTabStrip = layoutRoot.FindAnyWidget( "panel_right_tabs" );
		m_RightContent = layoutRoot.FindAnyWidget( "panel_right_content" );

		// Captions are one word each on purpose - UIActionTabs centres the label
		// across the whole button while the icon sits at a fixed x=10, so a wide
		// caption grows leftwards into its own icon.

		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );

		m_ActionListScroller = UIActionManager.CreateScroller( m_RightContent );

		// The tab panels are direct children of the scroller content, with no
		// grid in between: a GridSpacer reserves a row for a hidden child, so
		// four hidden tabs would pad the visible one down the page.
		m_ActionsWrapper = m_ActionListScroller.GetContentWidget();

		// Each tab declares its own PANEL_ROWS - how many direct children its builder adds.
		Widget panelGeneral = UIActionManager.CreateGridSpacer( m_ActionsWrapper, JMPlayerFormTabGeneral.PANEL_ROWS, 1 );
		Widget panelPosition = UIActionManager.CreateGridSpacer( m_ActionsWrapper, JMPlayerFormTabPosition.PANEL_ROWS, 1 );
		Widget panelInventory = UIActionManager.CreateGridSpacer( m_ActionsWrapper, JMPlayerFormTabInventory.PANEL_ROWS, 1 );
		Widget panelStatistics = UIActionManager.CreateGridSpacer( m_ActionsWrapper, JMPlayerFormTabStatistics.PANEL_ROWS, 1 );
		Widget panelActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, JMPlayerFormTabActions.PANEL_ROWS, 1 );

		m_TabIdGeneral = m_Tabs.AddTab( "#STR_COT_PLAYER_MODULE_TAB_GENERAL", JMConstants.Lucide( "user" ), panelGeneral );
		m_TabIdPosition = m_Tabs.AddTab( "#STR_COT_PLAYER_MODULE_TAB_POSITION", JMConstants.Lucide( "map-pin" ), panelPosition );
		m_TabIdInventory = m_Tabs.AddTab( "#STR_COT_PLAYER_MODULE_TAB_INVENTORY", JMConstants.Lucide( "backpack" ), panelInventory );
		m_TabIdStatistics = m_Tabs.AddTab( "#STR_COT_PLAYER_MODULE_TAB_STATISTICS", JMConstants.Lucide( "chart-column" ), panelStatistics );
		m_TabIdActions = m_Tabs.AddTab( "#STR_COT_PLAYER_MODULE_TAB_ACTIONS", JMConstants.Lucide( "zap" ), panelActions );

		DeclareTabs( 5 );

		// sendEvent = false: OnChange_Tab pokes at widgets no builder has run
		// for yet on the very first pass.
		m_Tabs.SetSelection( m_TabIdGeneral, false );

		// Only the tab that is about to be on screen gets built. The other four
		// cost nothing until the user asks for them, which is most of what the
		// form used to spend on being opened. Everything that touches a widget
		// from a tab it does not own null-guards it - that convention is what
		// makes deferring safe, so keep it when adding to this form.
		InitTabFocus( m_TabIdGeneral );

		// No geometry pin here: the true content height is only known once
		// JMWindowBase calls OnResize. Until then the layout's own 0.06 / 0.94
		// split holds, which is close enough to be invisible.
		m_ActionListScroller.UpdateScroller();
	}

	//! Build a tab's contents; JMFormBase calls this once per tab, the first time
	//! it is selected. Only the cases are ours. A modded-class fragment that
	//! injects extra tabs can just use AddTab(), or override this, call
	//! super.OnTabCreate() for the base cases and handle its own indices.
	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdGeneral )
		{
			m_TabGeneralCtrl = new JMPlayerFormTabGeneral( this );
			RegisterTab( m_TabIdGeneral, m_TabGeneralCtrl );
			m_TabGeneralCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdPosition )
		{
			m_TabPositionCtrl = new JMPlayerFormTabPosition( this );
			RegisterTab( m_TabIdPosition, m_TabPositionCtrl );
			m_TabPositionCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdInventory )
		{
			m_TabInventoryCtrl = new JMPlayerFormTabInventory( this );
			RegisterTab( m_TabIdInventory, m_TabInventoryCtrl );
			m_TabInventoryCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdActions )
		{
			m_TabActionsCtrl = new JMPlayerFormTabActions( this );
			RegisterTab( m_TabIdActions, m_TabActionsCtrl );
			m_TabActionsCtrl.OnCreate( panel );
		}
		else if ( tab == m_TabIdStatistics )
		{
			if ( !m_TabStatisticsCtrl )
			{
				m_TabStatisticsCtrl = new JMPlayerFormTabStatistics( this );
				RegisterTab( m_TabIdStatistics, m_TabStatisticsCtrl );
			}
			m_TabStatisticsCtrl.OnCreate( panel );
		}

		// A tab built after the form already has a selection has missed every
		// UpdateUI/RefreshStats pass so far, so it would come up blank and
		// unpermissioned. Catch it up before it is shown.
		if ( m_SelectedInstance )
		{
			UpdateUI();
			RefreshStats( true );
		}
	}

	//! DEPRECATED - use EnsureTabCreated and override OnTabCreate. Kept so a
	//! third-party modded form that still names it keeps compiling; an override
	//! of it is no longer called.
	protected void BuildTabIfNeeded( int tabIdx )
	{
		JMDeprecated.WarnOnce( this, "BuildTabIfNeeded() is deprecated. Please use EnsureTabCreated() and override OnTabCreate()." );

		EnsureTabCreated( tabIdx );
	}

	//! Thin forwarders - JMPlayerModule's RPC handlers call GetForm() and expect
	//! to reach these directly, but the Statistics tab (and its fields) now
	//! lives in JMPlayerFormTabStatistics.c, not on the form itself.
	void OnPlayerStatsUpdated( string guid, JMPlayerStats stats )
	{
		if ( m_TabStatisticsCtrl )
			m_TabStatisticsCtrl.OnPlayerStatsUpdated( guid, stats );
	}

	void OnExpansionInfoUpdated( string guid, array< string > ids, array< string > values )
	{
		if ( m_TabStatisticsCtrl )
			m_TabStatisticsCtrl.OnExpansionInfoUpdated( guid, ids, values );
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		//! CloseAllOverlays, OnTabUnfocus( previous ), OnTabCreate, OnTabFocus, OnTabUpdate.
		//! Every popup here anchors to the window root rather than to the tab panel,
		//! so UIActionTabs' Show(false) never reaches them - HandleTabChange()
		//! closes the registered ones so an open list cannot float over the next tab.
		HandleTabChange();

		if ( m_ActionListScroller )
			m_ActionListScroller.UpdateScroller();
	}

	//! Turn the glyph of whichever refresh button was clicked.
	//!
	//! Every one of these actions can complete with nothing on screen changing -
	//! a listing that came back identical, a stat block that has not moved - and
	//! a button that produces no feedback reads as a button that did not work.
	//! Two revolutions, matching every other refresh button in COT.
	void SpinRefreshIcon( UIActionBase action )
	{
		UIActionImageButton button;
		if ( Class.CastTo( button, action ) )
			button.TriggerSpin( 2 );
	}

	//! The player list, its filter and the row menu now live in JMPlayerRoster,
	//! JMPlayerRosterFilter and JMPlayerRowMenu. What follows is the surface the rest of
	//! the mod and sub-mods already call on the form - each is a thin hand-off - plus the
	//! confirmation callbacks, which JMFormBase resolves by name on the form itself.
	protected void InitWidgetsLeft()
	{
		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		m_Roster.Build( layoutRoot );
	}

	//! Extension point, unchanged: override, call super, then add to m_PlayerMenu.
	void OnPlayerRow_RightClick( string guid, int x, int y )
	{
		if ( m_RowMenu )
			m_RowMenu.Open( guid, x, y );
	}

	void OnRowMessage_Confirm( JMConfirmation confirmation = NULL )
	{
		if ( m_RowMenu )
			m_RowMenu.OnRowMessage_Confirm( confirmation );
	}

	void AddStatusRepairItem( UIActionContextMenu menu, string id )
	{
		if ( m_RowMenu )
			m_RowMenu.AddStatusRepairItem( menu, id );
	}

	bool RunStatusRepair( string id, string guid )
	{
		if ( m_RowMenu )
			return m_RowMenu.RunStatusRepair( id, guid );

		return false;
	}

	string RoleDisplayName( string role )
	{
		return m_Roster.RoleDisplayName( role );
	}

	//! force=true rebuilds unconditionally - see JMPlayerRoster.UpdatePlayerList.
	void UpdatePlayerList( bool force = false )
	{
		if ( m_Roster )
			m_Roster.UpdatePlayerList( force );
	}

	void OnRoleHeader_Toggled( string role )
	{
		m_Roster.OnRoleHeader_Toggled( role );
	}

	void OnRoleHeader_Checked( string role, bool checked )
	{
		m_Roster.OnRoleHeader_Checked( role, checked );
	}

	//! Extension point, unchanged: override, call super, and return false to hide a
	//! player from the list (see Ex_FilterPlayerList.c).
	protected bool PassesListFilters( JMPlayerInstance player )
	{
		return m_Roster.GetFilter().Passes( player );
	}

	//! What the roster asks. Public so it can reach the protected, overridable
	//! PassesListFilters above - a class outside the form cannot call that directly.
	bool FilterPlayer( JMPlayerInstance player )
	{
		return PassesListFilters( player );
	}

	void OnClick_SavePlayerListPref01()
	{
		m_Roster.OnClick_SavePlayerListPref01();
	}

	void OnClick_SavePlayerListPref02()
	{
		m_Roster.OnClick_SavePlayerListPref02();
	}

	void OnClick_LoadPlayerListPref01()
	{
		m_Roster.OnClick_LoadPlayerListPref01();
	}

	void OnClick_LoadPlayerListPref02()
	{
		m_Roster.OnClick_LoadPlayerListPref02();
	}

	//! Extension point: modded forms override this, call super and repaint their own
	//! rows. Runs ten times a second while the form is up.
	void RefreshStats(bool force = false)
	{
		if ( !m_SelectedInstance )
			return;

		if (g_Game.IsClient() && m_SelectedInstance.GetDataLastUpdatedTime() < m_LastChangeTime)
			return;

		// The coordinate fields only exist on the Position tab, and this runs ten
		// times a second. Everything below is a cheap SetCurrent / SetChecked and
		// stays ungated.
		if ( m_TabPositionCtrl && IsTabActive( m_TabPositionCtrl.GetTabId() ) )
			m_TabPositionCtrl.RefreshPosition( force );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Refresh( force );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.RefreshFlags();
	}

	override void SetPanelEnabled( bool enabled )
	{
		if ( enabled )
		{
			#ifdef COT_DEBUGLOGS
			Print( "[COT_DBG] JMPlayerForm::SetPanelEnabled true enter" );
			#endif

			m_RightPanelDisable.Show( false );

			#ifdef COT_DEBUGLOGS
			Print( "[COT_DBG] JMPlayerForm::SetPanelEnabled true exit" );
			#endif

			return;
		}

		//! Do NOT concatenate "this" into a Print() here - that implicit
		//! object-to-string conversion inside this override crashes the real
		//! DayZ Tools script compiler natively at module load (confirmed by
		//! bisection: this file alone, with only this pattern active,
		//! reproduces the crash). The custom lint tool does not catch it.
		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::SetPanelEnabled false enter" );
		#endif

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.ShowIdentityWidgets();

		m_RightPanelDisable.Show( true );

		m_SelectedInstance = NULL;

		//! Nothing in the right-hand panel means no row owns it either.
		m_LastSelectedGuid = "";
		m_Roster.ApplyListFocus();

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] JMPlayerForm::SetPanelEnabled false exit" );
		#endif
	}

	void UpdateUI()
	{
		if ( JM_GetSelected().NumPlayers(m_AutoSelect) == 0 )
		{
			SetPanelEnabled( false );
			return;
		}

		m_AutoSelect = false;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers(false)[0] );
		
		if ( !instance )
		{
			SetPanelEnabled( false );
			return;
		}

		if ( IsMissionOffline() )
		{
			instance.Update();
		}

		SetPanelEnabled( true );

		if ( m_TabGeneralCtrl )   m_TabGeneralCtrl.BindPermissions();
		if ( m_TabPositionCtrl )  m_TabPositionCtrl.BindPermissions();
		if ( m_TabInventoryCtrl ) m_TabInventoryCtrl.BindPermissions();
		if ( m_TabActionsCtrl )   m_TabActionsCtrl.BindPermissions();

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.ShowInstance( instance );

		if ( IsMissionOffline() )
		{
			if ( m_TabPositionCtrl )
				m_TabPositionCtrl.DisableForOffline();
			if ( m_TabActionsCtrl )
				m_TabActionsCtrl.DisableForOffline();
		}

		if ( m_SelectedInstance != instance )
		{
			m_SelectedInstance = instance;
		}

		// Push disease mask + bleeding state sync on selection change so dropdown
		// icons update for both panels.
		// UpdateUI is called from a lot more than a selection change -
		// OnClientPermissionsUpdated runs it every time the server pushes
		// permissions - so nothing below may fire an RPC unconditionally.
		// Unguarded, the Statistics panel rebuilt roughly once a second and read
		// as flashing, and the disease/bleeding requests went out just as often.
		string selectedGuid = instance.GetGUID();
		bool selectionChanged = ( selectedGuid != m_LastSelectedGuid );
		m_LastSelectedGuid = selectedGuid;

		if ( selectionChanged )
		{
			m_Module.RequestDiseaseMask( m_LastSelectedGuid );
			m_Module.RequestBleedingState( m_LastSelectedGuid );

			if ( m_TabPositionCtrl )
				m_TabPositionCtrl.OnSelectionChanged();

			if ( m_TabInventoryCtrl )
				m_TabInventoryCtrl.OnSelectionChanged();

			// The history belongs to the player, not to the character, so it
			// has to be dropped rather than left showing the previous numbers.
			if ( m_TabStatisticsCtrl )
				m_TabStatisticsCtrl.OnSelectionChanged();
		}

		m_Roster.ApplyListFocus();

		RefreshStats();
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayerList, 1500, true );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RefreshStats, 100, true, false );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapMarkers, 1000, true );
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( m_Roster.UpdateGroupAnimation );

		UpdateUI();

		UpdatePlayerList( true );
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePlayerList );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( RefreshStats );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateMapMarkers );
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( m_Roster.UpdateGroupAnimation );

		// A MapWidget left visible behind a hidden form keeps rendering.
		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.OnFormHide();

		// Local preview entities are real objects in the world, invisible and
		// simulation-disabled but real. Leaking one per form open is not
		// acceptable.
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.OnFormHide();

		// The overlays that float outside the form - the prompt holds the game's
		// inputs down while it is up - are closed by super.OnHide() through the
		// registry, above.

		//! A form hidden mid-fold would come back with half-height rows.
		m_Roster.FinishGroupAnimation();
	}

	//! The position map's marker widgets need reprojecting onto its current
	//! pan/zoom every frame the tab is up - see UIActionMap.TickMarkers().
	override void Update()
	{
		super.Update();

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Tick();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( m_TabInventoryCtrl )
			return m_TabInventoryCtrl.OnInventoryCellSelect( w );

		return false;
	}

	//! Both buttons arrive here: OnClick never fires for the right one, and it
	//! is not guaranteed to fire for a plain panel on the left one either.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( m_TabInventoryCtrl && m_TabInventoryCtrl.OnInventoryCellPressed( w, button ) )
			return true;

		return super.OnMouseButtonDown( w, x, y, button );
	}

	//! The container toggle lands here rather than on the press - see
	//! OnInventoryCellPressed.
	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( m_TabInventoryCtrl && m_TabInventoryCtrl.OnInventoryCellReleased( w, button ) )
			return true;

		return super.OnMouseButtonUp( w, x, y, button );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( m_TabInventoryCtrl && m_TabInventoryCtrl.OnInventoryCellHover( w ) )
			return true;

		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		// A cell is several widgets, so the pointer crosses a boundary moving
		// within one. Only drop the tooltip when it has actually left the cell.
		if ( m_TabInventoryCtrl && m_TabInventoryCtrl.OnMouseLeaveCell( enterW ) )
			return true;

		return super.OnMouseLeave( w, enterW, x, y );
	}

	//! The position map's own double-click (teleport-to-cursor) is handled by
	//! OnClick_PositionMap via UIActionMap's UIEvent.DOUBLE_CLICK now, not here.
	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
			return false;

		return super.OnDoubleClick( w, x, y, button );
	}

	override void OnFocus()
	{
		//CF_DumpWidgets( m_LeftPanel );
	}

	override void OnUnfocus()
	{

	}

	void UpdateLastChangeTime()
	{
		m_LastChangeTime = g_Game.GetTime();
	}

	// -------------------------------------------------------------------------
	//  Extension points
	//
	//  Modded classes (DayZ-Expansion AI / Hardline / PersonalStorage, and the
	//  ExampleScriptOverride samples) override these exact shapes, call super and
	//  inject their own rows. The widgets themselves are built by the tab
	//  classes; these forward to them, and the tabs call them through the
	//  CallInit* wrappers so an override always wraps the build.
	// -------------------------------------------------------------------------

	protected Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		if ( m_TabGeneralCtrl )
			return m_TabGeneralCtrl.InitActionWidgetsIdentity( actionsParent );

		return actionsParent;
	}

	//! Name, signature and return type are frozen: DayZ-Expansion modded forms
	//! override this exact shape, call super, then inject their own rows into
	//! m_Stamina.GetLayoutRoot().GetParent() - the slider grid - and re-sort them
	//! against m_ApplyStats and m_RefreshStats.
	protected Widget InitActionWidgetsStats( Widget actionsParent )
	{
		if ( m_TabGeneralCtrl )
			return m_TabGeneralCtrl.InitActionWidgetsStats( actionsParent );

		return actionsParent;
	}

	void Click_ApplyStats( UIEvent eid, UIActionBase action )
	{
		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_ApplyStats( eid, action );
	}

	//! Called by JMPlayerFormTabGeneral.OnCreate. Public so the tab can reach the
	//! virtual hooks above, which may be private / protected.
	Widget CallInitActionWidgetsIdentity( Widget actionsParent )
	{
		return InitActionWidgetsIdentity( actionsParent );
	}

	Widget CallInitActionWidgetsStats( Widget actionsParent )
	{
		return InitActionWidgetsStats( actionsParent );
	}

	// -------------------------------------------------------------------------
	//  Callbacks looked up by NAME on this form (JMConfirmation, CallLater, module RPC
	//  handlers). The logic lives in the tab classes; these only forward.
	// -------------------------------------------------------------------------

	void OnInventoryUpdated( string guid, array< ref JMPlayerInventoryItem > items, bool truncated )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.OnInventoryUpdated( guid, items, truncated );
	}

	void RunInventoryGroupAction( int index, int op )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.RunInventoryGroupAction( index, op );
	}

	void InventoryGroupDeleteConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.InventoryGroupDeleteConfirm( confirmation );
	}

	void Click_ClearCargoOnly( UIEvent eid, UIActionBase action )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.Click_ClearCargoOnly( eid, action );
	}

	void ClearCargoOnlyConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.ClearCargoOnlyConfirm( confirmation );
	}

	void OnClick_InventoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.OnClick_InventoryMenu( eid, action );
	}

	void InventoryDeleteConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.InventoryDeleteConfirm( confirmation );
	}

	void UpdateMapMarkers(  )
	{
		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.UpdateMapMarkers(  );
	}

	void Click_StripPlayer( UIEvent eid, UIActionBase action )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_StripPlayer( eid, action );
	}

	void StripConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.StripConfirm( confirmation );
	}

	void ClearCargoConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.ClearCargoConfirm( confirmation );
	}

	void VomitPlayerMultiConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerMultiConfirm(  );
	}

	void VomitPlayerSingleConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerSingleConfirm(  );
	}

	void VomitPlayerSelfConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerSelfConfirm(  );
	}

	void VomitPlayerMulti( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerMulti( confirmation );
	}

	void VomitPlayerSingle( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerSingle( confirmation );
	}

	void VomitPlayerSelf( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.VomitPlayerSelf( confirmation );
	}

	void SendMessageMultiConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageMultiConfirm(  );
	}

	void SendMessageSingleConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageSingleConfirm(  );
	}

	void SendMessageSelfConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageSelfConfirm(  );
	}

	void SendMessageMulti( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageMulti( confirmation );
	}

	void SendMessageSingle( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageSingle( confirmation );
	}

	void SendMessageSelf( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendMessageSelf( confirmation );
	}

	void SendNotifMulti( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendNotifMulti( confirmation );
	}

	void SendNotifSingle( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendNotifSingle( confirmation );
	}

	void SendNotifSelf( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.SendNotifSelf( confirmation );
	}

	void KickPlayerMultiConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.KickPlayerMultiConfirm( confirmation );
	}

	void KickPlayerSingleConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.KickPlayerSingleConfirm( confirmation );
	}

	void KickPlayerSelfConfirm( JMConfirmation confirmation = NULL )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.KickPlayerSelfConfirm( confirmation );
	}

	void KickPlayerMulti( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.KickPlayerMulti( confirmation );
	}

	void KickPlayerSingle( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.KickPlayerSingle( confirmation );
	}

	void BanPlayerMultiConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.BanPlayerMultiConfirm(  );
	}

	void BanPlayerSingleConfirm(  )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.BanPlayerSingleConfirm(  );
	}

	void BanPlayerSelfConfirm( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.BanPlayerSelfConfirm( confirmation );
	}

	void BanPlayerMulti( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.BanPlayerMulti( confirmation );
	}

	void BanPlayerSingle( JMConfirmation confirmation )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.BanPlayerSingle( confirmation );
	}

	void OnDiseaseMaskUpdated( string guid, int cholera, int influenza, int salmonella, int brain, int foodPoison, int chemPoison, int wound, int nerve, int heavyMetal )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.OnDiseaseMaskUpdated( guid, cholera, influenza, salmonella, brain, foodPoison, chemPoison, wound, nerve, heavyMetal );
	}

	void OnBleedingStateUpdated( string guid, array< string > names, array< int > bits, int bleedingBits, array< int > activePartIndices )
	{
		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.OnBleedingStateUpdated( guid, names, bits, bleedingBits, activePartIndices );
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Health );
	}

	// -------------------------------------------------------------------------
	//  DEPRECATED - the members below moved into the tab classes. They keep
	//  working for third-party modded forms, warn once, and forward.
	// -------------------------------------------------------------------------

	void Click_StatusUnconscious( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_StatusUnconscious() is deprecated. It moved to JMPlayerFormTabGeneral.Click_StatusUnconscious()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_StatusUnconscious( eid, action );
	}

	void Click_StatusBrokenLegs( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_StatusBrokenLegs() is deprecated. It moved to JMPlayerFormTabGeneral.Click_StatusBrokenLegs()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_StatusBrokenLegs( eid, action );
	}

	void Click_StatusBleeding( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_StatusBleeding() is deprecated. It moved to JMPlayerFormTabGeneral.Click_StatusBleeding()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_StatusBleeding( eid, action );
	}

	void Click_StatusSick( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_StatusSick() is deprecated. It moved to JMPlayerFormTabGeneral.Click_StatusSick()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_StatusSick( eid, action );
	}

	void OnClick_StatusMenu( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnClick_StatusMenu() is deprecated. It moved to JMPlayerFormTabGeneral.OnClick_StatusMenu()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.OnClick_StatusMenu( eid, action );
	}

	void Click_BadgeGodMode( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BadgeGodMode() is deprecated. It moved to JMPlayerFormTabGeneral.Click_BadgeGodMode()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_BadgeGodMode( eid, action );
	}

	void Click_BadgeFrozen( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BadgeFrozen() is deprecated. It moved to JMPlayerFormTabGeneral.Click_BadgeFrozen()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_BadgeFrozen( eid, action );
	}

	void Click_BadgeInvisible( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BadgeInvisible() is deprecated. It moved to JMPlayerFormTabGeneral.Click_BadgeInvisible()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_BadgeInvisible( eid, action );
	}

	void Click_BadgeUnconscious( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BadgeUnconscious() is deprecated. It moved to JMPlayerFormTabGeneral.Click_BadgeUnconscious()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_BadgeUnconscious( eid, action );
	}

	void RebuildTeleportHistory(  )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.RebuildTeleportHistory() is deprecated. It moved to JMPlayerFormTabPosition.RebuildTeleportHistory()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.RebuildTeleportHistory(  );
	}

	JMTeleportHistoryEntry TeleportHistoryEntryAt( int index )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.TeleportHistoryEntryAt() is deprecated. It moved to JMPlayerFormTabPosition.TeleportHistoryEntryAt()." );

		if ( m_TabPositionCtrl )
			return m_TabPositionCtrl.TeleportHistoryEntryAt( index );

		return NULL;
	}

	void SyncTeleportHistory(  )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.SyncTeleportHistory() is deprecated. It moved to JMPlayerFormTabPosition.SyncTeleportHistory()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.SyncTeleportHistory(  );
	}

	void OnClick_TeleportHistory( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnClick_TeleportHistory() is deprecated. It moved to JMPlayerFormTabPosition.OnClick_TeleportHistory()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.OnClick_TeleportHistory( eid, action );
	}

	void OnClick_TeleportHistoryCopy( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnClick_TeleportHistoryCopy() is deprecated. It moved to JMPlayerFormTabPosition.OnClick_TeleportHistoryCopy()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.OnClick_TeleportHistoryCopy( eid, action );
	}

	void OnClick_TeleportHistoryFocus( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnClick_TeleportHistoryFocus() is deprecated. It moved to JMPlayerFormTabPosition.OnClick_TeleportHistoryFocus()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.OnClick_TeleportHistoryFocus( eid, action );
	}

	void Click_RedoTeleport( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RedoTeleport() is deprecated. It moved to JMPlayerFormTabPosition.Click_RedoTeleport()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_RedoTeleport( eid, action );
	}

	void Click_ClearTeleportHistory( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_ClearTeleportHistory() is deprecated. It moved to JMPlayerFormTabPosition.Click_ClearTeleportHistory()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_ClearTeleportHistory( eid, action );
	}

	void Click_BandTakeAll( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BandTakeAll() is deprecated. It moved to JMPlayerFormTabInventory.Click_BandTakeAll()." );

		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.Click_BandTakeAll( eid, action );
	}

	void Click_BandRepairAll( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BandRepairAll() is deprecated. It moved to JMPlayerFormTabInventory.Click_BandRepairAll()." );

		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.Click_BandRepairAll( eid, action );
	}

	void Click_BandDeleteAll( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BandDeleteAll() is deprecated. It moved to JMPlayerFormTabInventory.Click_BandDeleteAll()." );

		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.Click_BandDeleteAll( eid, action );
	}

	void Click_RefreshInventory( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RefreshInventory() is deprecated. It moved to JMPlayerFormTabInventory.Click_RefreshInventory()." );

		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.Click_RefreshInventory( eid, action );
	}

	void Click_RefreshVitals( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RefreshVitals() is deprecated. It moved to JMPlayerFormTabGeneral.Click_RefreshVitals()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_RefreshVitals( eid, action );
	}

	void OnConfirm_InventoryPrompt( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnConfirm_InventoryPrompt() is deprecated. It moved to JMPlayerFormTabInventory.OnConfirm_InventoryPrompt()." );

		if ( m_TabInventoryCtrl )
			m_TabInventoryCtrl.OnConfirm_InventoryPrompt( eid, action );
	}

	void OnClick_PositionMap( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnClick_PositionMap() is deprecated. It moved to JMPlayerFormTabPosition.OnClick_PositionMap()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.OnClick_PositionMap( eid, action );
	}

	void Click_RecenterMap( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RecenterMap() is deprecated. It moved to JMPlayerFormTabPosition.Click_RecenterMap()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_RecenterMap( eid, action );
	}

	void Click_DryPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_DryPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_DryPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_DryPlayer( eid, action );
	}

	void Click_KillPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_KillPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_KillPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_KillPlayer( eid, action );
	}

	void Click_VomitPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_VomitPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_VomitPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_VomitPlayer( eid, action );
	}

	void Click_ScalePlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_ScalePlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_ScalePlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_ScalePlayer( eid, action );
	}

	void ScalePlayerMultiConfirm(  )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.ScalePlayerMultiConfirm() is deprecated. It moved to JMPlayerFormTabActions.ScalePlayerMultiConfirm()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.ScalePlayerMultiConfirm(  );
	}

	void ScalePlayerSingleConfirm(  )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.ScalePlayerSingleConfirm() is deprecated. It moved to JMPlayerFormTabActions.ScalePlayerSingleConfirm()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.ScalePlayerSingleConfirm(  );
	}

	void ScalePlayerSelfConfirm(  )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.ScalePlayerSelfConfirm() is deprecated. It moved to JMPlayerFormTabActions.ScalePlayerSelfConfirm()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.ScalePlayerSelfConfirm(  );
	}

	void OnConfirm_ScalePrompt( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.OnConfirm_ScalePrompt() is deprecated. It moved to JMPlayerFormTabActions.OnConfirm_ScalePrompt()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.OnConfirm_ScalePrompt( eid, action );
	}

	void Click_SendMessage( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SendMessage() is deprecated. It moved to JMPlayerFormTabActions.Click_SendMessage()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_SendMessage( eid, action );
	}

	void Click_CopyExpLoadout( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyExpLoadout() is deprecated. It moved to JMPlayerFormTabActions.Click_CopyExpLoadout()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_CopyExpLoadout( eid, action );
	}

	void Click_KickPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_KickPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_KickPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_KickPlayer( eid, action );
	}

	void Click_BanPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BanPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_BanPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_BanPlayer( eid, action );
	}

	void Click_StopBleeding( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_StopBleeding() is deprecated. It moved to JMPlayerFormTabActions.Click_StopBleeding()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_StopBleeding( eid, action );
	}

	void Click_AddDisease( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_AddDisease() is deprecated. It moved to JMPlayerFormTabActions.Click_AddDisease()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_AddDisease( eid, action );
	}

	void Click_RemoveDisease( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RemoveDisease() is deprecated. It moved to JMPlayerFormTabActions.Click_RemoveDisease()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_RemoveDisease( eid, action );
	}

	void Click_ApplyBleed( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_ApplyBleed() is deprecated. It moved to JMPlayerFormTabActions.Click_ApplyBleed()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_ApplyBleed( eid, action );
	}

	void Click_ClearAllDiseases( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_ClearAllDiseases() is deprecated. It moved to JMPlayerFormTabActions.Click_ClearAllDiseases()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_ClearAllDiseases( eid, action );
	}

	void Click_HealPlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_HealPlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_HealPlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_HealPlayer( eid, action );
	}

	void Click_SpectatePlayer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SpectatePlayer() is deprecated. It moved to JMPlayerFormTabActions.Click_SpectatePlayer()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_SpectatePlayer( eid, action );
	}

	void Click_RepairTransport( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RepairTransport() is deprecated. It moved to JMPlayerFormTabActions.Click_RepairTransport()." );

		if ( m_TabActionsCtrl )
			m_TabActionsCtrl.Click_RepairTransport( eid, action );
	}

	void Click_SetPosition( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetPosition() is deprecated. It moved to JMPlayerFormTabPosition.Click_SetPosition()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_SetPosition( eid, action );
	}

	void Click_TeleportToMe( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_TeleportToMe() is deprecated. It moved to JMPlayerFormTabPosition.Click_TeleportToMe()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_TeleportToMe( eid, action );
	}

	void Click_CopyPlayerName( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyPlayerName() is deprecated. It moved to JMPlayerFormTabGeneral.Click_CopyPlayerName()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_CopyPlayerName( eid, action );
	}

	void Click_CopyPlayerGUID( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyPlayerGUID() is deprecated. It moved to JMPlayerFormTabGeneral.Click_CopyPlayerGUID()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_CopyPlayerGUID( eid, action );
	}

	void Click_CopyPlayerSteam64ID( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyPlayerSteam64ID() is deprecated. It moved to JMPlayerFormTabGeneral.Click_CopyPlayerSteam64ID()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_CopyPlayerSteam64ID( eid, action );
	}

	void Click_OpenPlayerSteamProfile( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_OpenPlayerSteamProfile() is deprecated. It moved to JMPlayerFormTabGeneral.Click_OpenPlayerSteamProfile()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_OpenPlayerSteamProfile( eid, action );
	}

#ifdef GAMELABS
	void Click_CopyCFToolsID( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyCFToolsID() is deprecated. It moved to JMPlayerFormTabGeneral.Click_CopyCFToolsID()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_CopyCFToolsID( eid, action );
	}
#endif

#ifdef GAMELABS
	void Click_OpenPlayerCFProfile( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_OpenPlayerCFProfile() is deprecated. It moved to JMPlayerFormTabGeneral.Click_OpenPlayerCFProfile()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_OpenPlayerCFProfile( eid, action );
	}
#endif

	void Click_CopyPlayerPostion( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyPlayerPostion() is deprecated. It moved to JMPlayerFormTabPosition.Click_CopyPlayerPostion()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_CopyPlayerPostion( eid, action );
	}

	void Click_CopyPlayerRotation( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CopyPlayerRotation() is deprecated. It moved to JMPlayerFormTabPosition.Click_CopyPlayerRotation()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_CopyPlayerRotation( eid, action );
	}

	void Click_PastePlayerPostion( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_PastePlayerPostion() is deprecated. It moved to JMPlayerFormTabPosition.Click_PastePlayerPostion()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_PastePlayerPostion( eid, action );
	}

	void Click_TeleportMeTo( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_TeleportMeTo() is deprecated. It moved to JMPlayerFormTabPosition.Click_TeleportMeTo()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_TeleportMeTo( eid, action );
	}

	void Click_RefreshTeleports( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RefreshTeleports() is deprecated. It moved to JMPlayerFormTabPosition.Click_RefreshTeleports()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Click_RefreshTeleports( eid, action );
	}

	void Change_PositionX( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Change_PositionX() is deprecated. It moved to JMPlayerFormTabPosition.Change_PositionX()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Change_PositionX( eid, action );
	}

	void Change_PositionY( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Change_PositionY() is deprecated. It moved to JMPlayerFormTabPosition.Change_PositionY()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Change_PositionY( eid, action );
	}

	void Change_PositionZ( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Change_PositionZ() is deprecated. It moved to JMPlayerFormTabPosition.Change_PositionZ()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.Change_PositionZ( eid, action );
	}

	void RefreshTeleports( bool force = false )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.RefreshTeleports() is deprecated. It moved to JMPlayerFormTabPosition.RefreshTeleports()." );

		if ( m_TabPositionCtrl )
			m_TabPositionCtrl.RefreshTeleports( force );
	}

	void ApplyStats( TStringArray guids )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.ApplyStats() is deprecated. It moved to JMPlayerFormTabGeneral.ApplyStats()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.ApplyStats( guids );
	}

	void Click_SetBlood( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetBlood() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Blood );
	}

	void Click_SetEnergy( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetEnergy() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Energy );
	}

	void Click_SetWater( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetWater() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Water );
	}

	void Click_SetShock( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetShock() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Shock );
	}

	void Click_SetStamina( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetStamina() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.Stamina );
	}

	void Click_SetHeatBuffer( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetHeatBuffer() is deprecated. It was replaced by JMPlayerFormTabGeneral.OnChange_Stat() and ChangeStat()." );

		if ( m_TabGeneralCtrl && eid == UIEvent.CHANGE )
			m_TabGeneralCtrl.ChangeStat( JMStatType.HeatBuffer );
	}

	void Click_BloodyHands( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_BloodyHands() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.BLOODYHANDS );
	}

	void Click_GodMode( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_GodMode() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.GODMODE );
	}

	void Click_Freeze( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_Freeze() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.FREEZE );
	}

	void Click_SetBrokenLegs( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetBrokenLegs() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.BROKENLEGS );
	}

	void Click_SetReceiveDamageDealt( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_SetReceiveDamageDealt() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.RECEIVEDAMAGEDEALT );
	}

	void Click_CannotBeTargetedByAI( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_CannotBeTargetedByAI() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.CANNOTBETARGETEDBYAI );
	}

	void Click_Invisible( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_Invisible() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.INVISIBILITY );
	}

	void Click_RemoveCollision( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_RemoveCollision() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.REMOVECOLLISION );
	}

	void Click_UnlimitedAmmo( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_UnlimitedAmmo() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.UNLIMITEDAMMO );
	}

	void Click_AdminNVG( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_AdminNVG() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.ADMINNVG );
	}

	void Click_UnlimitedStamina( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_UnlimitedStamina() is deprecated. It was replaced by JMPlayerFormTabActions.OnClick_Toggle() and ClickToggle()." );

		if ( m_TabActionsCtrl && eid == UIEvent.CLICK )
			m_TabActionsCtrl.ClickToggle( JMPlayerToggle.UNLIMITEDSTAMINA );
	}

	void Click_EditPlayerRoles( UIEvent eid, UIActionBase action )
	{
		JMDeprecated.WarnOnce( this, "JMPlayerForm.Click_EditPlayerRoles() is deprecated. It moved to JMPlayerFormTabGeneral.Click_EditPlayerRoles()." );

		if ( m_TabGeneralCtrl )
			m_TabGeneralCtrl.Click_EditPlayerRoles( eid, action );
	}

}
