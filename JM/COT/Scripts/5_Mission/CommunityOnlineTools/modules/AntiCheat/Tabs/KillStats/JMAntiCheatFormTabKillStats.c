//! "Combat & Precision Stats" tab of JMAntiCheatForm - kill/accuracy table.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
class JMAntiCheatFormTabKillStats: JMFormTab
{
	protected JMAntiCheatForm m_Form;
	protected UIActionScroller m_KillStatsScroller;
	protected Widget m_KillStatsContent;
	protected ref array< string > m_KillGuidOrder = new array< string >;
	protected ref map< string, ref JMAntiCheatKillStats > m_KillStats = new map< string, ref JMAntiCheatKillStats >;

	void JMAntiCheatFormTabKillStats( JMAntiCheatForm form )
	{
		m_Form = form;
	}

	void SetStats( array< string > guidOrder, map< string, ref JMAntiCheatKillStats > stats )
	{
		m_KillGuidOrder = guidOrder;
		m_KillStats     = stats;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		if ( !panel )
			return;

		UIActionCard card = UIActionManager.CreateScrollCard( panel, "Combat Precision & Kill Metrics", m_KillStatsScroller );
		card.AddRefreshButton( m_Form, "OnClick_Refresh", "Refresh combat stats" );

		m_KillStatsContent = card.GetContent();
		m_KillStatsScroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_KillStatsScroller )
			m_KillStatsScroller.UpdateScroller();
	}

	void Rebuild( string filter )
	{
		if ( !m_KillStatsContent )
			return;

		UIActionManager.ClearChildren( m_KillStatsContent );
		Widget body = m_KillStatsContent;

		if ( !m_KillStats || m_KillGuidOrder.Count() == 0 )
		{
			UIActionManager.CreateEmptyState( body, "#STR_COT_ANTICHEAT_NO_KILL_STATISTICS_RECORDED", "#STR_COT_ANTICHEAT_COMBAT_DATA_AND_PRECISION_METRICS_FOR" );
			m_KillStatsScroller.UpdateScroller();
			return;
		}

		Widget header = UIActionManager.CreateGridSpacer( body, 1, 8 );
		UIActionText hh1 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_PLAYER" );         hh1.SetWidth( 0.22 );
		UIActionText hh2 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_KILLS" );          hh2.SetWidth( 0.08 );
		UIActionText hh3 = UIActionManager.CreateText( header, "", "Hit %" );          hh3.SetWidth( 0.08 );
		UIActionText hh4 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_HEAD" );          hh4.SetWidth( 0.08 );
		UIActionText hh5 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_TORSO" );         hh5.SetWidth( 0.08 );
		UIActionText hh6 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_LIMBS" );         hh6.SetWidth( 0.10 );
		UIActionText hh7 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_AVG_MAX_DIST" );   hh7.SetWidth( 0.22 );
		UIActionText hh8 = UIActionManager.CreateText( header, "", "#STR_COT_ANTICHEAT_K_D_RATIO" );     hh8.SetWidth( 0.12 );

		UIActionManager.CreateDivider( body, JMTheme.DIVIDER_LIGHT, 1 );

		int rendered = 0;

		JMSearchMatcher matcher = new JMSearchMatcher( filter );

		for ( int i = 0; i < m_KillGuidOrder.Count(); i++ )
		{
			string guid = m_KillGuidOrder.Get( i );
			JMAntiCheatKillStats stats = m_KillStats.Get( guid );
			if ( !stats )
				continue;

			if ( !matcher.Matches( stats.PlayerName ) && !matcher.Matches( guid ) )
				continue;

			rendered++;

			float hit = stats.GetHitRatio();
			string hitStr;
			if ( hit < 0 )
				hitStr = "-";
			else
				hitStr = ( hit * 100 ).ToString() + "%";

			int limbs = stats.BodyLeftArm + stats.BodyRightArm + stats.BodyLeftLeg + stats.BodyRightLeg;

			string kdStr = stats.TotalKills.ToString();
			if ( stats.TotalMisses > 0 )
			{
				float ratio = stats.TotalKills / (float)( stats.TotalKills + stats.TotalMisses );
				kdStr = ratio.ToString();
			}

			Widget row = UIActionManager.CreateGridSpacer( body, 1, 8 );
			UIActionText c1 = UIActionManager.CreateText( row, "", stats.PlayerName );       c1.SetWidth( 0.22 );
			UIActionText c2 = UIActionManager.CreateText( row, "", stats.TotalKills.ToString() ); c2.SetWidth( 0.08 );
			UIActionText c3 = UIActionManager.CreateText( row, "", hitStr );                 c3.SetWidth( 0.08 );
			UIActionText c4 = UIActionManager.CreateText( row, "", stats.BodyHead.ToString() );       c4.SetWidth( 0.08 );
			UIActionText c5 = UIActionManager.CreateText( row, "", stats.BodyTorso.ToString() );      c5.SetWidth( 0.08 );
			UIActionText c6 = UIActionManager.CreateText( row, "", limbs.ToString() );        c6.SetWidth( 0.10 );
			UIActionText c7 = UIActionManager.CreateText( row, "", stats.GetAvgDistance().ToString() + " / " + stats.DistanceMax.ToString() + "m" ); c7.SetWidth( 0.22 );
			UIActionText c8 = UIActionManager.CreateText( row, "", kdStr );                   c8.SetWidth( 0.12 );

			if ( hit >= 0.85 && stats.TotalKills >= 3 )
			{
				c3.SetColor( ARGB( 255, 217, 90, 90 ) );
				c3.SetTooltip( "Suspiciously high hit accuracy (" + hitStr + ")" );
			}

			UIActionManager.CreateRowDivider( body );
		}

		if ( rendered == 0 && filter != "" )
		{
			UIActionManager.CreateEmptyState( body, "No combat stats match filter '" + filter + "'.", "", 40 );
		}

		m_KillStatsScroller.UpdateScroller();
	}

}
