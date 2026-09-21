//! The player list's advanced-filter dropdown: the toolbar button, the menu it opens
//! and the six built-in "show ..." switches. Mods add rows with
//! JMFilterRegistry.Register( JMFilterRegistry.PLAYERS, ... ); the form's overridable
//! PassesListFilters() is what asks this class, so an override there still runs.
class JMPlayerRosterFilter
{
	protected JMPlayerForm m_Form;
	protected JMPlayerRoster m_Roster;
	protected UIActionImageButton m_PlayerListFilters;

	//! The advanced-filter dropdown, and the state its entries toggle. All on
	//! by default: an admin who has never touched this sees the whole roster.
	protected ref UIActionFilterMenu m_Menu;
	protected bool m_FilterShowDead     = true;
	protected bool m_FilterShowUncon    = true;
	protected bool m_FilterShowHurt     = true;
	protected bool m_FilterShowSick     = true;
	protected bool m_FilterShowAdmins   = true;
	protected bool m_FilterShowCheaters = true;

	//! Advanced list filters. Each one hides a category of player rather than
	//! selecting one: they start all-on, so the list is complete until an admin
	//! deliberately narrows it.
	static const string LIST_FILTER_DEAD     = "f_dead";
	static const string LIST_FILTER_UNCON    = "f_uncon";
	static const string LIST_FILTER_HURT     = "f_hurt";
	static const string LIST_FILTER_SICK     = "f_sick";
	static const string LIST_FILTER_ADMINS   = "f_admins";
	static const string LIST_FILTER_CHEATERS = "f_cheaters";

	void JMPlayerRosterFilter( JMPlayerForm form, JMPlayerRoster roster )
	{
		m_Form   = form;
		m_Roster = roster;
	}

	//! The toolbar pill that opens the dropdown. Built here rather than by the
	//! roster so the button and the menu it anchors to are one thing.
	UIActionImageButton CreateButton( Widget parent )
	{
		m_PlayerListFilters = UIActionManager.CreateIconButton( parent, JMConstants.Lucide( "list-filter" ), this, "OnClick_PlayerListFilters" );

		if ( m_PlayerListFilters )
		{
			m_PlayerListFilters.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_ADVANCED_FILTERS" );

			//! The grid sizes its row to the tallest child, so shrinking every pill
			//! shrinks the row - and the list below grows by the difference.
			m_PlayerListFilters.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		}

		return m_PlayerListFilters;
	}

	//! Open the advanced-filter dropdown under the toolbar button.
	//!
	//! A context menu rather than a panel of checkboxes: the whole row - glyph
	//! and word together - is already one hit area here, which is exactly the
	//! behaviour asked for, and it floats over the list instead of costing the
	//! list a permanent strip of height it only needs while being changed.
	//! Mods add rows with JMFilterRegistry.Register( JMFilterRegistry.PLAYERS, ... ).
	void OnClick_PlayerListFilters( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_PlayerListFilters )
			return;

		if ( !m_Menu )
		{
			m_Menu = UIActionManager.CreateOverlayFilterMenu( m_Form, m_PlayerListFilters.GetLayoutRoot(), JMFilterRegistry.PLAYERS );

			if ( !m_Menu )
				return;

			m_Menu.AddPage( "root", this, "BuildFilterRootPage", "", false, "", this, "OnFilterRootChange" );
		}

		//! A second click on the button puts the menu away rather than
		//! reopening it in place, which is what a dropdown is expected to do.
		m_Menu.ToggleAt( m_PlayerListFilters.GetLayoutRoot() );
	}

	//! Every entry carries its current state in its colour. An active filter is
	//! normal text; a disabled one is dimmed, which is the whole read-out - there
	//! is no separate tick, the row IS the checkbox.
	void BuildFilterRootPage( UIActionFilterMenu menu )
	{
		menu.AddToggleRow( LIST_FILTER_DEAD,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_DEAD",     m_FilterShowDead,     JMConstants.Lucide( "skull" ) );
		menu.AddToggleRow( LIST_FILTER_UNCON,    "#STR_COT_PLAYER_MODULE_FILTER_SHOW_UNCON",    m_FilterShowUncon,    JMConstants.Lucide( "bed" ) );
		menu.AddToggleRow( LIST_FILTER_HURT,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_HURT",     m_FilterShowHurt,     JMConstants.Lucide( "droplet" ) );
		menu.AddToggleRow( LIST_FILTER_SICK,     "#STR_COT_PLAYER_MODULE_FILTER_SHOW_SICK",     m_FilterShowSick,     JMConstants.Lucide( "thermometer" ) );
		menu.AddToggleRow( LIST_FILTER_ADMINS,   "#STR_COT_PLAYER_MODULE_FILTER_SHOW_ADMINS",   m_FilterShowAdmins,   JMConstants.Lucide( "shield" ) );
		menu.AddToggleRow( LIST_FILTER_CHEATERS, "#STR_COT_PLAYER_MODULE_FILTER_SHOW_CHEATERS", m_FilterShowCheaters, JMConstants.Lucide( "flag" ) );
	}

	//! Flip the filter the clicked row stands for and rebuild the list behind
	//! the menu; UIActionFilterMenu repaints the rows itself. Rows registered
	//! in JMFilterRegistry never reach here - they run their own callback.
	void OnFilterRootChange( string id )
	{
		if ( id == LIST_FILTER_DEAD )
			m_FilterShowDead = !m_FilterShowDead;
		else if ( id == LIST_FILTER_UNCON )
			m_FilterShowUncon = !m_FilterShowUncon;
		else if ( id == LIST_FILTER_HURT )
			m_FilterShowHurt = !m_FilterShowHurt;
		else if ( id == LIST_FILTER_SICK )
			m_FilterShowSick = !m_FilterShowSick;
		else if ( id == LIST_FILTER_ADMINS )
			m_FilterShowAdmins = !m_FilterShowAdmins;
		else if ( id == LIST_FILTER_CHEATERS )
			m_FilterShowCheaters = !m_FilterShowCheaters;
		else
			return;

		m_Roster.UpdatePlayerList( true );
	}

	//! Does this player survive the advanced filters? Dead is tested before the
	//! rest because a corpse is not separately unconscious or bleeding as far as
	//! the list is concerned - it is one row, and "show dead" owns it.
	bool Passes( JMPlayerInstance player )
	{
		bool playerDead = player.IsDead();

		if ( playerDead )
			return m_FilterShowDead;

		if ( !m_FilterShowUncon && player.IsUnconscious() )
			return false;

		if ( !m_FilterShowHurt && ( player.IsBleeding() || player.HasBrokenLegs() ) )
			return false;

		if ( !m_FilterShowSick && player.IsSick() )
			return false;

		if ( !m_FilterShowAdmins && player.HasPermission( "COT" ) )
			return false;

		if ( !m_FilterShowCheaters && JMAntiCheatStatus.IsFlagged( player.GetGUID() ) )
			return false;

		return true;
	}
}
