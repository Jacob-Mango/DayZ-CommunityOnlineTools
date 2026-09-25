//! "Statistics" tab of JMPlayerForm - session, combat and (Expansion-only)
//! wallet/faction/reputation/group readouts for the selected player.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
//! Confirmation-bound methods stay off this file entirely - the Statistics
//! tab has none - see JMPlayerForm.c's remaining General/Position/Inventory/
//! Actions code for why those tabs could not be split the same way.
class JMPlayerFormTabStatistics: JMFormTab
{
	protected JMPlayerForm m_Form;
	protected UIActionKeyValueList m_StatsSession;
	protected UIActionKeyValueList m_StatsCombat;
	protected UIActionProgressBar m_StatsHeadshotBar;
	protected UIActionText m_StatsCombatNotice;
	protected UIActionKeyValueList m_StatsExpansion;
	protected Widget m_StatsExpansionRoot;
	protected UIActionImageButton m_StatsRefreshButton;
	protected ref JMPlayerStats m_PlayerStats;
	protected JMAntiCheatModule m_AntiCheatModule;

	//! Direct children this tab's builder adds to its panel, with headroom. Over-
	//! provisioning is free under Size-To-Content-V; running short silently clips the
	//! last rows with no error anywhere. The form sizes the panel from this.
	static const int PANEL_ROWS = 8;

	void JMPlayerFormTabStatistics( JMPlayerForm form )
	{
		m_Form = form;
	}

	//! Kill stats for the selected player, from whichever side of the wire has
	//! them. RequestFlags never fills the client cache on a listen host.
	protected JMAntiCheatKillStats GetSelectedKillStats()
	{
		if ( !m_AntiCheatModule || !m_Form.m_SelectedInstance )
			return NULL;

		string guid = m_Form.m_SelectedInstance.GetGUID();

		map< string, ref JMAntiCheatKillStats > source = m_AntiCheatModule.GetClientKillStats();
		if ( IsMissionHost() )
			source = m_AntiCheatModule.GetServerKillStats();

		if ( !source || !source.Contains( guid ) )
			return NULL;

		return source.Get( guid );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		Widget parent = UIActionManager.CreateGridSpacer( panel, 12, 1 );

		// Refresh acts on both cards below it, so it sits in the session card
		// title bar - the same place the Position and Inventory cards put
		// theirs, spelled with the same glyph. It used to be a loose icon inside
		// the card body nudged to x=0.9, which cost a row of height and drifted
		// against the values column as the form was resized.
		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_SESSION" );
		Widget section0 = section0Card.GetContent();
		m_StatsRefreshButton = section0Card.AddRefreshButton( this, "" );
		if ( m_StatsRefreshButton ) m_StatsRefreshButton.SetOnClick( this, "Click_RefreshStatistics" );
		m_StatsRefreshButton.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_REFRESH_STATISTICS" );

		m_StatsSession = UIActionManager.CreateKeyValueList( section0 );

		UIActionManager.CreateSpacerPx( section0, 10 );
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_COMBAT" );
		Widget section1 = section1Card.GetContent();
		m_StatsCombat = UIActionManager.CreateKeyValueList( section1 );

		// The bar is a summary of the two rows above it, so it gets its own
		// breathing room instead of reading as a fifth table row.
		UIActionManager.CreateSpacerPx( section1, 8 );
		m_StatsHeadshotBar = UIActionManager.CreateProgressBar( section1, "#STR_COT_PLAYER_MODULE_STAT_HEADSHOT_RATIO", 0 );

		// Shown instead of the combat rows when the admin lacks the anti-cheat
		// permission. A silently empty section reads as "this player has never
		// fired a shot", which is a different and wrong answer.
		m_StatsCombatNotice = UIActionManager.CreateText( section1, "", "" );

	#ifdef DZ_Expansion_Core
		UIActionManager.CreateSpacerPx( parent, 10 );
		UIActionCard section2Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_EXPANSION" );
		m_StatsExpansion = UIActionManager.CreateKeyValueList( section2Card.GetContent() );

		// Hidden until the server answers with at least one row: Expansion being
		// COMPILED against is not the same as any of its features being on, and
		// an empty card would claim the player has no money, faction, reputation
		// or group when the truth is that nothing asked.
		m_StatsExpansionRoot = section2Card.GetLayoutRoot();
		if ( m_StatsExpansionRoot )
			m_StatsExpansionRoot.Show( false );
	#endif

		JMPlayerStatRegistry.Populate( parent );
	}

	//! Tab entered: pull both halves. Never on a timer.
	override void OnFocus()
	{
		RequestStatistics();
	}

	override void OnUpdate()
	{
		RefreshStatisticsPanel();
	}

	void Click_RefreshStatistics( UIActionBase action )
	{
		m_Form.SpinRefreshIcon( action );
		RequestStatistics();
	}

	//! Selection changed while the tab is up, or a fresh player was just
	//! selected - either way the old numbers cannot mean anything here.
	void OnSelectionChanged()
	{
		m_PlayerStats = NULL;

		if ( IsFocused() )
			RequestStatistics();
		else
			RefreshStatisticsPanel();
	}

	//! Pull both halves. Called from OnFocus, on a selection change while the tab
	//! is up, and never on a timer.
	void RequestStatistics()
	{
		if ( !m_Form.m_Module || !m_Form.m_SelectedInstance )
			return;

		if ( JMPermissions.Has( JMConstants.PERM_PLAYER_STATISTICS_VIEW ) )
			m_Form.m_Module.RequestPlayerStats( m_Form.m_SelectedInstance.GetGUID() );

	#ifdef DZ_Expansion_Core
		//! Same permission and the same one-shot request as the session block -
		//! none of it is on a timer.
		if ( JMPermissions.Has( JMConstants.PERM_PLAYER_STATISTICS_VIEW ) )
			m_Form.m_Module.RequestExpansionInfo( m_Form.m_SelectedInstance.GetGUID() );
	#endif

		RequestKillStats();
		RefreshStatisticsPanel();
	}

	//! Ask the anti-cheat module for its kill table.
	//!
	//! On a listen host RequestFlags() returns without sending anything - there
	//! is no wire to send over - so the server-side map is read directly instead.
	//! Only the request differs; both paths end up in RefreshStatisticsPanel.
	protected void RequestKillStats()
	{
		if ( !m_AntiCheatModule )
			Class.CastTo( m_AntiCheatModule, GetModuleManager().GetModule( JMAntiCheatModule ) );

		if ( !m_AntiCheatModule )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ANTICHEAT_VIEW ) )
			return;

		if ( !IsMissionHost() )
			m_AntiCheatModule.RequestFlags();
	}

	//! The Expansion block came back. Declared unconditionally - the module
	//! calls it from an RPC handler that is compiled either way - but it only
	//! has anywhere to put the rows on an Expansion build.
	void OnExpansionInfoUpdated( string guid, array< string > ids, array< string > values )
	{
	#ifdef DZ_Expansion_Core
		if ( !m_Form.m_SelectedInstance || guid != m_Form.m_SelectedInstance.GetGUID() )
			return;

		if ( !m_StatsExpansion || !ids || !values )
			return;

		m_StatsExpansion.Clear();

		int count = ids.Count();
		if ( values.Count() < count )
			count = values.Count();

		for ( int i = 0; i < count; i++ )
			m_StatsExpansion.SetValue( ExpansionInfoLabel( ids[i] ), values[i], ExpansionInfoIcon( ids[i] ) );

		if ( m_StatsExpansionRoot )
			m_StatsExpansionRoot.Show( count > 0 );

		if ( m_Form.m_ActionListScroller )
			m_Form.m_ActionListScroller.UpdateScroller();
	#endif
	}

#ifdef DZ_Expansion_Core
	//! Row ids are resolved to a label and a glyph HERE rather than sent as
	//! text, so the server never decides what language the admin reads and an
	//! id the client does not recognise still renders as itself.
	protected string ExpansionInfoLabel( string id )
	{
		if ( id == JMPlayerModule.EXP_INFO_MONEY )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_MONEY";

		if ( id == JMPlayerModule.EXP_INFO_FACTION )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_FACTION";

		if ( id == JMPlayerModule.EXP_INFO_REPUTATION )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_REPUTATION";

		if ( id == JMPlayerModule.EXP_INFO_GROUP )
			return "#STR_COT_PLAYER_MODULE_STAT_EXP_GROUP";

		return id;
	}

	protected string ExpansionInfoIcon( string id )
	{
		if ( id == JMPlayerModule.EXP_INFO_MONEY )
			return JMConstants.Lucide( "banknote" );

		if ( id == JMPlayerModule.EXP_INFO_FACTION )
			return JMConstants.Lucide( "flag" );

		if ( id == JMPlayerModule.EXP_INFO_REPUTATION )
			return JMConstants.Lucide( "star" );

		if ( id == JMPlayerModule.EXP_INFO_GROUP )
			return JMConstants.Lucide( "users" );

		return JMConstants.Lucide( "info" );
	}
#endif

	void OnPlayerStatsUpdated( string guid, JMPlayerStats stats )
	{
		if ( !m_Form.m_SelectedInstance || guid != m_Form.m_SelectedInstance.GetGUID() )
			return;

		m_PlayerStats = stats;
		RefreshStatisticsPanel();
	}

	void RefreshStatisticsPanel()
	{
		RefreshSessionStats();
		RefreshCombatStats();
	}

	protected void RefreshSessionStats()
	{
		if ( !m_StatsSession )
			return;

		// Every row is written whether or not there is data behind it. A section
		// that renders a different NUMBER of rows depending on what arrived reads
		// as a broken panel; one that renders "-" reads as "nothing recorded",
		// which is the actual answer for a player whose history starts today.
		string playtime    = "-";
		string sessions    = "-";
		string firstSeen   = "-";
		string lastSeen    = "-";
		string deaths      = "-";
		string longestLife = "-";
		string currentLife = "-";

		if ( m_PlayerStats )
		{
			playtime    = JMPlayerStats.FormatDuration( m_PlayerStats.TotalPlaytimeSec );
			sessions    = m_PlayerStats.SessionCount.ToString();
			firstSeen   = JMPlayerStats.FormatSince( m_PlayerStats.FirstSeenUnix );
			lastSeen    = JMPlayerStats.FormatSince( m_PlayerStats.LastSeenUnix );
			deaths      = m_PlayerStats.Deaths.ToString();
			longestLife = JMPlayerStats.FormatDuration( m_PlayerStats.LongestLifeSec );
			currentLife = JMPlayerStats.FormatDuration( m_PlayerStats.GetCurrentLifeSeconds() );
		}

		// The icon is only read the first time a key is seen, so passing it on
		// every refresh costs nothing.
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_PLAYTIME", playtime, JMConstants.Lucide( "hourglass" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_SESSIONS", sessions, JMConstants.Lucide( "log-in" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_FIRSTSEEN", firstSeen, JMConstants.Lucide( "calendar-plus" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_LASTSEEN", lastSeen, JMConstants.Lucide( "calendar-check" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_DEATHS", deaths, JMConstants.Lucide( "skull" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_LONGEST_LIFE", longestLife, JMConstants.Lucide( "heart-pulse" ) );
		m_StatsSession.SetValue( "#STR_COT_PLAYER_MODULE_STAT_CURRENT_LIFE", currentLife, JMConstants.Lucide( "heart" ) );
	}

	protected void RefreshCombatStats()
	{
		if ( !m_StatsCombat || !m_Form.m_SelectedInstance )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ANTICHEAT_VIEW ) )
		{
			if ( m_StatsCombatNotice )
				m_StatsCombatNotice.SetText( "#STR_COT_PLAYER_MODULE_STAT_NO_PERMISSION" );
			return;
		}

		JMAntiCheatKillStats stats = GetSelectedKillStats();

		string kills   = "-";
		string hitStr  = "-";
		string zones   = "-";
		string avgStr  = "-";
		string maxStr  = "-";
		float headshotRatio = 0;

		if ( stats )
		{
			kills = stats.TotalKills.ToString();

			// Math.Round returns a float, so rounding straight into ToString
			// gives "50.0%". Land it in an int first.
			float hit = stats.GetHitRatio();
			if ( hit >= 0 )
			{
				int hitPercent = Math.Round( hit * 100 );
				hitStr = hitPercent.ToString() + "%";
			}

			zones = stats.BodyHead.ToString() + " / " + stats.BodyTorso.ToString();

			int avgDist = Math.Round( stats.GetAvgDistance() );
			int maxDist = Math.Round( stats.DistanceMax );
			avgStr = avgDist.ToString() + "m";
			maxStr = maxDist.ToString() + "m";

			if ( stats.TotalKills > 0 )
				headshotRatio = stats.BodyHead / (float)stats.TotalKills;
		}

		if ( m_StatsCombatNotice )
		{
			if ( stats )
				m_StatsCombatNotice.SetText( "" );
			else
				m_StatsCombatNotice.SetText( "#STR_COT_PLAYER_MODULE_STAT_NO_COMBAT" );
		}

		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_KILLS", kills, JMConstants.Lucide( "swords" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_HITRATIO", hitStr, JMConstants.Lucide( "target" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_HEADSHOTS", zones, JMConstants.Lucide( "scan-face" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_AVGDIST", avgStr, JMConstants.Lucide( "ruler" ) );
		m_StatsCombat.SetValue( "#STR_COT_PLAYER_MODULE_STAT_MAXDIST", maxStr, JMConstants.Lucide( "ruler-dimension-line" ) );

		if ( m_StatsHeadshotBar )
			m_StatsHeadshotBar.SetProgress( headshotRatio );
	}
}
