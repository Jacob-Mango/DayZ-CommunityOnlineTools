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
	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdFlagged;
	protected int m_TabIdKillStats;

	//! protected, not private: sub-mods reach for the module through the form.
	//! Also public enough (no modifier) for JMAntiCheatFormTabFlagged /
	//! JMAntiCheatFormTabKillStats to reach it through their back-reference.
	JMAntiCheatModule m_Module;
	protected UIActionTabs m_Tabs;
	protected Widget m_TabFlaggedPanel;
	protected Widget m_TabKillStatsPanel;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them.
	protected ref JMAntiCheatFormTabFlagged   m_TabFlagged;
	protected ref JMAntiCheatFormTabKillStats m_TabKillStats;
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

	override void OnCreate()
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
		m_SortToggle.SetTooltip( "#STR_COT_ANTICHEAT_SORT_BY_SCORE_HIGHEST_FIRST_LOWEST" );
	}

	protected void InitWidgetsBottom()
	{
		m_TabFlaggedPanel   = layoutRoot.FindAnyWidget( "panel_flagged_players" );
		m_TabKillStatsPanel = layoutRoot.FindAnyWidget( "panel_kill_stats" );


		m_BottomTabStrip = layoutRoot.FindAnyWidget( "panel_bottom_tabs" );
		m_BottomContent  = layoutRoot.FindAnyWidget( "panel_bottom_content" );

		m_Tabs = UIActionManager.CreateTabStrip( m_BottomTabStrip, this, "OnChange_Tab" );

		m_TabIdFlagged = m_Tabs.AddTab( "Active Flagged Players", JMConstants.Lucide( "shield-alert" ), m_TabFlaggedPanel );
		m_TabIdKillStats = m_Tabs.AddTab( "Combat & Precision Stats", JMConstants.Lucide( "crosshair" ), m_TabKillStatsPanel );

		DeclareTabs( 2 );

		m_Tabs.SetSelection( m_TabIdFlagged, false );

		InitTabFocus( m_TabIdFlagged );
	}

	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdFlagged )
		{
			m_TabFlagged = new JMAntiCheatFormTabFlagged( this );
			RegisterTab( m_TabIdFlagged, m_TabFlagged );
			m_TabFlagged.OnCreate( panel );
		}
		else if ( tab == m_TabIdKillStats )
		{
			m_TabKillStats = new JMAntiCheatFormTabKillStats( this );
			RegisterTab( m_TabIdKillStats, m_TabKillStats );
			m_TabKillStats.OnCreate( panel );
		}

		RebuildList();
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

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinBottomPanelGeometry( h * 0.88 );

		ResizeTabs( w, h );
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

	protected void RebuildList()
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
