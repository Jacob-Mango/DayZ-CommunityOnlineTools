// =============================================================================
//  JMAntiCheatForm.c
//
//  Anti-cheat flag monitoring view with quick actions, detection sorting,
//  human-friendly detection descriptions, context menu, and combat metrics.
//
//  Per-tab widget building/rendering lives in JMAntiCheatFormTabFlagged.c and
//  JMAntiCheatFormTabKillStats.c. What stays here: the shared top toolbar
//  (name filter + sort, which filters BOTH tabs), tab-strip mechanics, the
//  module's OnFlagsUpdated/OnKillStatsUpdated contract (module calls back on
//  the form, so these can't move), OnMouseButtonDown (a JMFormBase virtual
//  override), and the three Confirm* methods - JMConfirmation dispatches its
//  named callbacks against whatever object its window was Init()'d with, not
//  against whichever tab raised the popup, so moving them would silently
//  break the confirm flow.
// =============================================================================

class JMAntiCheatForm : JMFormBase
{
	static const int TAB_FLAGGED   = 0;
	static const int TAB_KILLSTATS = 1;

	//! protected, not private: sub-mods reach for the module through the form.
	//! Also public enough (no modifier) for JMAntiCheatFormTabFlagged /
	//! JMAntiCheatFormTabKillStats to reach it through their back-reference.
	JMAntiCheatModule m_Module;

	protected UIActionTabs m_Tabs;
	protected Widget m_TabFlaggedPanel;
	protected Widget m_TabKillStatsPanel;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them.
	private ref JMAntiCheatFormTabFlagged   m_TabFlagged;
	private ref JMAntiCheatFormTabKillStats m_TabKillStats;

	protected UIActionSearchBox m_NameFilter;

	//! Score order, as the same icon toggle the player list uses for its sort
	//! rather than a dropdown: there are two states, and a control that spends a
	//! quarter of the toolbar spelling out the one it is already in is a poor
	//! trade for a 32px button that shows it.
	protected UIActionImageButtonToggle m_SortToggle;

	//! Target of a destructive quick action while its confirmation is up. The
	//! dialog answers with a named method that takes no argument, so the guid
	//! cannot ride along with it. Public: JMAntiCheatFormTabFlagged sets this
	//! before raising the confirmation.
	string m_PendingGuid;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		InitWidgetsTop();
		InitWidgetsBottom();
	}

	protected void InitWidgetsTop()
	{
		Widget top = layoutRoot.FindAnyWidget( "panel_top" );
		if ( !top )
			return;

		Widget topRow = UIActionManager.CreateWrapSpacer( top, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		//! No refresh button here and no flagged/tracked readout: the card below
		//! carries its own refresh, and the counts it summarised are the rows an
		//! admin is already looking at.
		m_NameFilter = UIActionManager.CreateSearchBox( topRow, this, "OnChange_Filter", "Filter by name / GUID..." );
		m_NameFilter.SetWidth( 0.80 );

		//! Untoggled = highest score first, which is the order that matters: the
		//! worst offender is the one an admin opened this view for.
		m_SortToggle = UIActionManager.CreateImageButtonToggle( topRow, JMConstants.Lucide( "arrow-down-1-0" ), JMConstants.Lucide( "arrow-down-0-1" ), this, "OnClick_Sort" );
		m_SortToggle.SetFixedSize( 30, 30 );
		m_SortToggle.SetTooltip( "Sort by score - highest first / lowest first" );
	}

	protected void InitWidgetsBottom()
	{
		m_TabFlaggedPanel   = layoutRoot.FindAnyWidget( "panel_flagged_players" );
		m_TabKillStatsPanel = layoutRoot.FindAnyWidget( "panel_kill_stats" );

		ref array<string> tabLabels = { "Active Flagged Players", "Combat & Precision Stats" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "shield-alert" ), JMConstants.Lucide( "crosshair" ) };

		m_Tabs = UIActionManager.CreateTabs( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_TabFlaggedPanel );
		m_Tabs.AddContent( m_TabKillStatsPanel );

		InitTabState( 2 );

		m_Tabs.SetSelection( TAB_FLAGGED, false );

		BuildTabIfNeeded( TAB_FLAGGED );
	}

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_FLAGGED:
				m_TabFlagged = new JMAntiCheatFormTabFlagged( this );
				m_TabFlagged.Build( m_TabFlaggedPanel );
				break;

			case TAB_KILLSTATS:
				m_TabKillStats = new JMAntiCheatFormTabKillStats( this );
				m_TabKillStats.Build( m_TabKillStatsPanel );
				break;
		}

		RebuildList();
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		BuildTabIfNeeded( GetActiveTabIndex() );
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinStripGeometry( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), layoutRoot.FindAnyWidget( "panel_bottom_content" ), h * 0.88, TAB_STRIP_HEIGHT );

		if ( m_TabFlagged )
			m_TabFlagged.OnResize();

		if ( m_TabKillStats )
			m_TabKillStats.OnResize();
	}

	override void OnShow()
	{
		super.OnShow();

		if ( m_Module )
			m_Module.RequestFlags();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		RebuildList();
	}

	// -----------------------------------------------------------------------
	//  Shared top toolbar - filters/sorts BOTH tabs.
	// -----------------------------------------------------------------------

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

		if ( m_Module )
			m_Module.RequestFlags();
	}

	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RebuildList();
	}

	//! The sort toggle answers with CLICK, not CHANGE - it is an image button
	//! that remembers a state, not an editor - so it cannot share OnChange_Filter.
	void OnClick_Sort( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RebuildList();
	}

	private void RebuildList()
	{
		if ( !m_Module )
			return;

		array< string > order = m_Module.GetClientGuidOrder();
		map< string, ref JMAntiCheatFlag > flags = m_Module.GetClientFlags();

		string filter = "";
		if ( m_NameFilter )
			filter = m_NameFilter.GetText();
		filter.ToLower();

		bool lowestFirst = false;
		if ( m_SortToggle )
			lowestFirst = m_SortToggle.IsToggled();

		if ( m_TabFlagged )
			m_TabFlagged.Rebuild( order, flags, filter, lowestFirst );

		if ( m_TabKillStats )
			m_TabKillStats.Rebuild( filter );
	}

	// -----------------------------------------------------------------------
	//  Right-click context menu - JMAntiCheatFormTabFlagged owns the menu
	//  itself, but the widget-tree walk needs a JMFormBase virtual override.
	// -----------------------------------------------------------------------

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.RIGHT && w && m_TabFlagged )
		{
			Widget curr = w;
			JMAntiCheatClearData data;

			// Widget.GetUserData is an out-parameter call, not a getter that
			// returns the object - it cannot be passed straight to Class.CastTo.
			Class userData;

			while ( curr )
			{
				userData = NULL;
				curr.GetUserData( userData );

				if ( Class.CastTo( data, userData ) )
				{
					m_TabFlagged.ShowPlayerContextMenu( data.Guid, x, y );
					return true;
				}
				curr = curr.GetParent();
			}
		}

		return super.OnMouseButtonDown( w, x, y, button );
	}

	// -----------------------------------------------------------------------
	//  Destructive confirmations - stay here, see file header.
	// -----------------------------------------------------------------------

	void ConfirmClear( JMConfirmation confirmation = NULL )
	{
		if ( m_PendingGuid == "" || !m_Module )
			return;

		m_Module.ClearFlag( m_PendingGuid );
		m_Module.RequestFlags();
	}

	void ConfirmKick( JMConfirmation confirmation = NULL )
	{
		if ( m_PendingGuid == "" )
			return;

		JMPlayerModule playerMod = JMPlayerModule.Cast( GetModuleManager().GetModule( JMPlayerModule ) );
		if ( playerMod )
			playerMod.Kick( { m_PendingGuid }, "Kicked by Anti-Cheat" );
	}

	void ConfirmBan( JMConfirmation confirmation = NULL )
	{
		if ( m_PendingGuid == "" )
			return;

		JMBanModule banMod = JMBanModule.Cast( GetModuleManager().GetModule( JMBanModule ) );
		if ( banMod )
			banMod.Ban( m_PendingGuid, "", "Banned by Anti-Cheat", -1, "", "" );
	}

	// -----------------------------------------------------------------------
	//  Server update hooks - the module calls back on the form, so these stay
	//  here and forward the data into whichever tab owns it.
	// -----------------------------------------------------------------------

	void OnFlagsUpdated( array< string > guidOrder, map< string, ref JMAntiCheatFlag > flags )
	{
		RebuildList();
	}

	void OnKillStatsUpdated( array< string > guidOrder, map< string, ref JMAntiCheatKillStats > stats )
	{
		if ( m_TabKillStats )
			m_TabKillStats.SetStats( guidOrder, stats );

		RebuildList();
	}
}

class JMAntiCheatClearData : UIActionData
{
	string Guid;

	void JMAntiCheatClearData( string guid )
	{
		Guid = guid;
	}
}

class JMAntiCheatGroupEntry
{
	string Key;
	string HumanTitle;
	int Count;
	int TotalPoints;
	string LatestTechnical;
	ref TStringArray Occurrences;

	void JMAntiCheatGroupEntry( string title )
	{
		Key = title;
		HumanTitle = title;
		Count = 0;
		TotalPoints = 0;
		LatestTechnical = "";
		Occurrences = new TStringArray;
	}
}

class JMAntiCheatSubGroupData : UIActionData
{
	string Guid;
	string GroupKey;

	void JMAntiCheatSubGroupData( string guid, string groupKey )
	{
		Guid = guid;
		GroupKey = groupKey;
	}
}
