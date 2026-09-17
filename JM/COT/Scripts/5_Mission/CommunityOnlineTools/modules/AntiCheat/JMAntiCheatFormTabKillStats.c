//! "Combat & Precision Stats" tab of JMAntiCheatForm - kill/accuracy table.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
class JMAntiCheatFormTabKillStats
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

	void Build( Widget parentPanel )
	{
		if ( !parentPanel )
			return;

		m_KillStatsScroller = UIActionManager.CreateScroller( parentPanel );
		Widget scrollerContent = m_KillStatsScroller.GetContentWidget();

		UIActionCard card = UIActionManager.CreateCard( scrollerContent, "Combat Precision & Kill Metrics" );
		card.AddRefreshButton( m_Form, "OnClick_Refresh", "Refresh combat stats" );

		m_KillStatsContent = card.GetContent();
		m_KillStatsScroller.UpdateScroller();
	}

	void OnResize()
	{
		if ( m_KillStatsScroller )
			m_KillStatsScroller.UpdateScroller();
	}

	void SetStats( array< string > guidOrder, map< string, ref JMAntiCheatKillStats > stats )
	{
		m_KillGuidOrder = guidOrder;
		m_KillStats     = stats;
	}

	void Rebuild( string filter )
	{
		if ( !m_KillStatsContent )
			return;

		ClearWidgetChildren( m_KillStatsContent );
		Widget body = m_KillStatsContent;

		if ( !m_KillStats || m_KillGuidOrder.Count() == 0 )
		{
			Widget empty = UIActionManager.CreatePanel( body, 0x00000000, 60 );
			UIActionManager.CreateText( empty, "No kill statistics recorded.", "Combat data and precision metrics for active players will appear here." );
			m_KillStatsScroller.UpdateScroller();
			return;
		}

		Widget header = UIActionManager.CreateGridSpacer( body, 1, 8 );
		UIActionText hh1 = UIActionManager.CreateText( header, "", "Player" );         hh1.SetWidth( 0.22 );
		UIActionText hh2 = UIActionManager.CreateText( header, "", "Kills" );          hh2.SetWidth( 0.08 );
		UIActionText hh3 = UIActionManager.CreateText( header, "", "Hit %" );          hh3.SetWidth( 0.08 );
		UIActionText hh4 = UIActionManager.CreateText( header, "", "Head" );          hh4.SetWidth( 0.08 );
		UIActionText hh5 = UIActionManager.CreateText( header, "", "Torso" );         hh5.SetWidth( 0.08 );
		UIActionText hh6 = UIActionManager.CreateText( header, "", "Limbs" );         hh6.SetWidth( 0.10 );
		UIActionText hh7 = UIActionManager.CreateText( header, "", "Avg/Max Dist" );   hh7.SetWidth( 0.22 );
		UIActionText hh8 = UIActionManager.CreateText( header, "", "K/D Ratio" );     hh8.SetWidth( 0.12 );

		UIActionManager.CreatePanel( body, JMTheme.DIVIDER_LIGHT, 1 );

		int rendered = 0;

		for ( int i = 0; i < m_KillGuidOrder.Count(); i++ )
		{
			string guid = m_KillGuidOrder.Get( i );
			JMAntiCheatKillStats stats = m_KillStats.Get( guid );
			if ( !stats )
				continue;

			if ( filter != "" )
			{
				string nameLow = stats.PlayerName;
				nameLow.ToLower();
				string guidLow = guid;
				guidLow.ToLower();

				if ( nameLow.IndexOf( filter ) == -1 && guidLow.IndexOf( filter ) == -1 )
					continue;
			}

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

			UIActionManager.CreatePanel( body, 0x22FFFFFF, 1 );
		}

		if ( rendered == 0 && filter != "" )
		{
			Widget emptyFilter = UIActionManager.CreatePanel( body, 0x00000000, 40 );
			UIActionManager.CreateText( emptyFilter, "No combat stats match filter '" + filter + "'." );
		}

		m_KillStatsScroller.UpdateScroller();
	}

	private void ClearWidgetChildren( Widget parent )
	{
		if ( !parent )
			return;

		Widget child = parent.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
	}
}
