// =============================================================================
//  JMAntiCheatForm.c
//
//  Read-only view of the anti-cheat flag state + per-player kill stats
//  (hit/miss ratio, body-part distribution, distance avg/max).
// =============================================================================

class JMAntiCheatForm : JMFormBase
{
	private UIActionScroller m_ListScroller;
	private Widget m_ListContent;

	private UIActionButton m_Refresh;
	private UIActionSearchBox m_NameFilter;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMAntiCheatModule m_Module;

	// Kill-stats mirror
	private ref array< string > m_KillGuidOrder;
	private ref map< string, ref JMAntiCheatKillStats > m_KillStats;

	void JMAntiCheatForm()
	{
		m_KillGuidOrder = new array< string >;
		m_KillStats     = new map< string, ref JMAntiCheatKillStats >;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		Widget panel = layoutRoot.FindAnyWidget( "panel" );

		// Vertical stack: toolbar (row 0) + scroller (row 1)
		Widget stack = UIActionManager.CreateGridSpacer( panel, 2, 1 );

		Widget toolbar = UIActionManager.CreateGridSpacer( stack, 1, 2 );
		m_NameFilter = UIActionManager.CreateSearchBox( toolbar, this, "OnChange_Filter", "Filter by name..." );
		m_NameFilter.SetWidth( 0.75 );
		m_NameFilter.SetTooltip( "Filter the flagged-player list by name" );

		m_Refresh = UIActionManager.CreateButton( toolbar, "Refresh", this, "OnClick_Refresh" );
		m_Refresh.SetWidth( 0.24 );
		m_Refresh.SetIcon( JMConstants.Lucide( "refresh-cw" ) );
		m_Refresh.SetTooltip( "#STR_COT_GENERIC_REFRESH" );

		m_ListScroller = UIActionManager.CreateScroller( stack );
		m_ListContent  = m_ListScroller.GetContentWidget();
		m_ListScroller.UpdateScroller();

		// Initial empty-state placeholder so the panel isn't blank
		RebuildList();
	}

	override void OnShow()
	{
		super.OnShow();

		m_Refresh.UpdatePermission( "Admin.AntiCheat.View" );
		m_Module.RequestFlags();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
		m_Refresh.UpdatePermission( "Admin.AntiCheat.View" );
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );
		if ( m_ListScroller )
			m_ListScroller.UpdateScroller();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

		m_Module.RequestFlags();
	}

	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RebuildList();
	}

	// -----------------------------------------------------------------------
	//  Server update hooks
	// -----------------------------------------------------------------------

	void OnFlagsUpdated( array< string > guidOrder, map< string, ref JMAntiCheatFlag > flags )
	{
		RebuildList();
	}

	void OnKillStatsUpdated( array< string > guidOrder, map< string, ref JMAntiCheatKillStats > stats )
	{
		m_KillGuidOrder = guidOrder;
		m_KillStats     = stats;
		RebuildList();
	}

	// -----------------------------------------------------------------------
	//  Render
	// -----------------------------------------------------------------------

	private void RebuildList()
	{
		string filter = "";
		if ( m_NameFilter )
			filter = m_NameFilter.GetText();
		filter.ToLower();

		// Clear existing rows
		if ( m_ListContent )
		{
			Widget child = m_ListContent.GetChildren();
			while ( child )
			{
				Widget next = child.GetSibling();
				child.Unlink();
				child = next;
			}
		}

		if ( !m_Module )
			return;

		array< string > order = m_Module.GetClientGuidOrder();
		map< string, ref JMAntiCheatFlag > flags = m_Module.GetClientFlags();

		// Empty-state placeholder so the panel doesn't look broken
		if ( order.Count() == 0 )
		{
			Widget empty = UIActionManager.CreatePanel( m_ListContent, 0x00000000, 60 );
			UIActionManager.CreateText( empty, "No flags detected.", "Server is clean. Detections appear here as they happen." );
			RenderKillStatsSection( empty );
			m_ListScroller.UpdateScroller();
			return;
		}

		foreach ( string guid : order )
		{
			JMAntiCheatFlag flag = flags.Get( guid );
			if ( !flag )
				continue;

			if ( filter != "" )
			{
				string name = flag.PlayerName;
				name.ToLower();
				if ( name.IndexOf( filter ) == -1 )
					continue;
			}

			Widget row = UIActionManager.CreateGridSpacer( m_ListContent, 1, 2 );

			UIActionBadge scoreBadge = UIActionManager.CreateBadge( row, "Score: " + flag.TotalScore.ToString() );
			scoreBadge.SetWidth( 0.18 );
			scoreBadge.SetColor( ScoreToColor( flag.TotalScore ) );
			scoreBadge.SetTooltip( "Total accumulated cheat-detection score. Critical = " + JMConstants.WEBHOOK_COLOR_DANGER + "." );

			UIActionText info = UIActionManager.CreateText( row, "", flag.PlayerName + " (" + guid + ")" );
			info.SetWidth( 0.55 );
			info.SetTooltip( ConcatenateEvents( flag ) );

			UIActionButton clearBtn = UIActionManager.CreateButton( row, "Clear", this, "OnClick_Clear" );
			clearBtn.SetWidth( 0.2 );
			clearBtn.SetData( new JMAntiCheatClearData( guid ) );
			clearBtn.SetTooltip( "Reset this player's flag state" );
			clearBtn.UpdatePermission( "Admin.AntiCheat.Clear" );

			// Render the kill-stats for this player inline
			RenderPlayerKillStats( guid );
		}

		m_ListScroller.UpdateScroller();
	}

	// -----------------------------------------------------------------------
	//  Kill-stats renderers
	// -----------------------------------------------------------------------

	// Renders the kill-stats table (one row per player with a stat breakdown)
	// at the bottom of the scroller - visible whether or not a player is
	// currently flagged, so admins can see clean players too.
	private void RenderKillStatsSection( Widget parent )
	{
		if ( !m_KillStats || m_KillGuidOrder.Count() == 0 )
			return;

		UIActionManager.CreateText( parent, "Kill Stats" );
		Widget header = UIActionManager.CreateGridSpacer( parent, 1, 8 );
		UIActionText hh1 = UIActionManager.CreateText( header, "", "Player" );      hh1.SetWidth( 0.18 );
		UIActionText hh2 = UIActionManager.CreateText( header, "", "Kills" );       hh2.SetWidth( 0.08 );
		UIActionText hh3 = UIActionManager.CreateText( header, "", "Hit%" );       hh3.SetWidth( 0.08 );
		UIActionText hh4 = UIActionManager.CreateText( header, "", "Head" );       hh4.SetWidth( 0.08 );
		UIActionText hh5 = UIActionManager.CreateText( header, "", "Torso" );      hh5.SetWidth( 0.08 );
		UIActionText hh6 = UIActionManager.CreateText( header, "", "Arms" );       hh6.SetWidth( 0.10 );
		UIActionText hh7 = UIActionManager.CreateText( header, "", "Legs" );       hh7.SetWidth( 0.10 );
		UIActionText hh8 = UIActionManager.CreateText( header, "", "AvgDist/Max" );hh8.SetWidth( 0.30 );

		for ( int i = 0; i < m_KillGuidOrder.Count(); i++ )
		{
			JMAntiCheatKillStats stats = m_KillStats.Get( m_KillGuidOrder.Get( i ) );
			if ( !stats )
				continue;

			float hit = stats.GetHitRatio();
			string hitStr;
			if ( hit < 0 )
				hitStr = "-";
			else
				hitStr = ( hit * 100 ).ToString() + "%";

			Widget row = UIActionManager.CreateGridSpacer( parent, 1, 8 );
			UIActionText c1 = UIActionManager.CreateText( row, "", stats.PlayerName );     c1.SetWidth( 0.18 );
			UIActionText c2 = UIActionManager.CreateText( row, "", stats.TotalKills.ToString() ); c2.SetWidth( 0.08 );
			UIActionText c3 = UIActionManager.CreateText( row, "", hitStr );               c3.SetWidth( 0.08 );
			UIActionText c4 = UIActionManager.CreateText( row, "", stats.BodyHead.ToString() );     c4.SetWidth( 0.08 );
			UIActionText c5 = UIActionManager.CreateText( row, "", stats.BodyTorso.ToString() );    c5.SetWidth( 0.08 );
			UIActionText c6 = UIActionManager.CreateText( row, "", ( stats.BodyLeftArm + stats.BodyRightArm ).ToString() ); c6.SetWidth( 0.10 );
			UIActionText c7 = UIActionManager.CreateText( row, "", ( stats.BodyLeftLeg + stats.BodyRightLeg ).ToString() );  c7.SetWidth( 0.10 );
			UIActionText c8 = UIActionManager.CreateText( row, "", stats.GetAvgDistance().ToString() + " / " + stats.DistanceMax.ToString() ); c8.SetWidth( 0.30 );
		}
	}

	// Render a single player's kill stats inline below their flag row
	private void RenderPlayerKillStats( string guid )
	{
		if ( !m_KillStats || !m_KillStats.Contains( guid ) )
			return;
		JMAntiCheatKillStats stats = m_KillStats.Get( guid );
		if ( !stats || stats.TotalKills == 0 )
			return;

		Widget row = UIActionManager.CreateGridSpacer( m_ListContent, 1, 4 );
		UIActionText t1 = UIActionManager.CreateText( row, "", "  Kills: " + stats.TotalKills.ToString() );
		t1.SetWidth( 0.25 );
		UIActionText t2 = UIActionManager.CreateText( row, "", "  Head/Torso: " + stats.BodyHead + "/" + stats.BodyTorso );
		t2.SetWidth( 0.30 );
		UIActionText t3 = UIActionManager.CreateText( row, "", "  Avg: " + stats.GetAvgDistance().ToString() + "m  Max: " + stats.DistanceMax.ToString() + "m" );
		t3.SetWidth( 0.35 );
		string kdStr = stats.TotalKills.ToString();
		if ( stats.TotalMisses > 0 )
			kdStr = ( stats.TotalKills / (float)( stats.TotalKills + stats.TotalMisses ) ).ToString();
		UIActionText t4 = UIActionManager.CreateText( row, "", "  K/D: " + kdStr );
		t4.SetWidth( 0.10 );
	}

	private int ScoreToColor( int score )
	{
		if ( score >= 30 ) return ARGB(255, 217, 90, 90 );
		if ( score >= 15 ) return ARGB(255, 217, 164, 65 );
		return ARGB(255, 73, 184, 117 );
	}

	private string ConcatenateEvents( JMAntiCheatFlag flag )
	{
		string s = "";
		foreach ( string evt : flag.RecentEvents )
		{
			if ( s != "" )
				s = s + "\n";
			s = s + evt;
		}
		return s;
	}

	// -----------------------------------------------------------------------
	//  Actions
	// -----------------------------------------------------------------------

	void OnClick_Clear( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMAntiCheatClearData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		// Server-side direct call (works on local admin / singleplayer)
		m_Module.ClearFlag( data.Guid );
		m_Module.RequestFlags();
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