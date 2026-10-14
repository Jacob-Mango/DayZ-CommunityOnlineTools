//! The player list down the left of the Player form: the counters, the toolbar, the
//! search box, and the row pool grouped by role (with its fold animation). It owns
//! everything about WHICH players are shown; the form owns which one is selected and
//! what the right-hand tabs do with it, and is reached through m_Form.
class JMPlayerRoster
{
	protected JMPlayerForm m_Form;
	protected ref JMPlayerRosterFilter m_Filter;
	protected autoptr array< JMPlayerRowWidget > m_PlayerList;
	protected UIActionText m_PlayerListCount;
	protected UIActionText m_PlayerListPeak;
	protected UIActionText m_PlayerListSelected;
	protected UIActionSearchBox m_PlayerListFilter;

	//! Slot count of the server, when the client knows it - a direct connect
	//! never went through the browser, so it has no host data to read.
	protected int m_MaxPlayers;

	//! High-water mark of the roster. Static so it survives the form being
	//! rebuilt (permissions change, menu reopened) and keeps counting for the
	//! whole session rather than restarting at the current headcount.
	protected static int s_PeakPlayerCount;
	protected UIActionImageButtonToggle m_PlayerListSort;
	protected UIActionImageButton m_PlayerListSelectAll;
	protected UIActionImageButton m_PlayerListDeSelectAll;
	protected UIActionImageButton m_PlayerPrefSave;
	protected UIActionImageButton m_PlayerPrefLoad;
	protected UIActionScroller m_PlayerListScroller;
	protected Widget m_PlayerListRows;

	//! Roles the user has collapsed. Kept as names rather than as row indices
	//! because the row pool is reused - the row that carried a group before a
	//! refresh is probably carrying a different one after it.
	protected ref TStringArray m_CollapsedRoles;

	//! Role -> the GUIDs listed under it as of the last rebuild, so a header's
	//! checkbox and count act on exactly the rows it is sitting above and not on
	//! members the filter took out.
	protected ref map< string, ref TStringArray > m_RoleMembers;

	//! The one group currently folding or unfolding. Collapse is not a state
	//! change followed by a repaint: the rows have to still exist while they
	//! shrink, so the collapse is only committed to m_CollapsedRoles once the
	//! animation has run out.
	protected ref array< JMPlayerRowWidget > m_AnimRows;
	protected JMPlayerRowWidget m_AnimHeader;
	protected string m_AnimRole;
	protected bool   m_AnimExpanding;
	protected float  m_AnimTime;
	protected autoptr TStringArray m_PlayersPref1 = {};
	protected autoptr TStringArray m_PlayersPref2 = {};

	//  Role grouping: a header per role that collapses its members. Grouping happens after filtering,
	//  so a header's count is the rows under it and a group nothing matched does not show.

	//! Every player carries "everyone", so anything past it is a role somebody
	//! granted on purpose. A player with nothing else granted groups under
	//! "everyone" itself rather than under a made-up bucket name.
	static const string ROLE_UNGROUPED = "everyone";

	//  Fold animation: rows sit in a GridSpacer, so driving a row's height to zero IS the animation
	//  (rows below slide up). Alpha rides along so a half-height row does not read as clipped.

	static const float GROUP_ANIM_DURATION = 0.16;

	//! The loader shown while the roster is being fetched.
	protected ref JMBusySpinner m_Busy;
	protected int m_FetchBaseline;
	protected float m_FetchStart = -1000;

	//! Seconds the loader is held at least, and at most, after the form opens.
	static const float FETCH_MIN_SHOWN = 0.6;
	static const float FETCH_TIMEOUT = 10;
	protected int m_LastRosterVersion = -1;

	void JMPlayerRoster( JMPlayerForm form )
	{
		m_Form   = form;
		m_Filter = new JMPlayerRosterFilter( form, this );

		m_PlayerList     = new array< JMPlayerRowWidget >;
		m_CollapsedRoles = new TStringArray;
		m_RoleMembers    = new map< string, ref TStringArray >;
		m_AnimRows       = new array< JMPlayerRowWidget >;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerRoster()
	{
		if ( !g_Game )
			return;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	JMPlayerRosterFilter GetFilter()
	{
		return m_Filter;
	}

	string GetPrimaryRole( JMPlayerInstance player )
	{
		array< string > roles = player.GetRoles();

		//! Do not assume "everyone" sits at index 0 - a hand-edited player
		//! file (Roles written directly, not through the Role Manager UI)
		//! can list roles in any order or omit "everyone" altogether.
		if ( roles )
		{
			for ( int i = 0; i < roles.Count(); i++ )
			{
				if ( roles[i] != ROLE_UNGROUPED )
					return roles[i];
			}
		}

		return ROLE_UNGROUPED;
	}

	//! Does the role itself grant the COT permission? A player row's shield
	//! means "granted to this person"; the same shield on a header means every
	//! player under it is staff by virtue of being in the group.
	bool IsAdminRole( string role )
	{
		JMRole roleData = GetPermissionsManager().GetRole( role );
		if ( !roleData )
			return false;

		JMPermissionType permType;
		return roleData.HasPermission( JMPlayerRowWidget.PERMISSION_ADMIN, permType );
	}

	bool IsGroupAnimating()
	{
		return m_AnimRole != "";
	}

	bool IsRoleCollapsed( string role )
	{
		return m_CollapsedRoles.Find( role ) >= 0;
	}

	bool IsRoleFullySelected( string role )
	{
		TStringArray guids;
		if ( !m_RoleMembers.Find( role, guids ) )
			return false;

		if ( guids.Count() == 0 )
			return false;

		foreach ( string guid : guids )
		{
			if ( !JM_GetSelected().IsSelected( guid ) )
				return false;
		}

		return true;
	}

	//! Re-measure the list's scroll range after the form was resized.
	void UpdateScroller()
	{
		if ( m_PlayerListScroller )
			m_PlayerListScroller.UpdateScroller();
	}

	//! One roster counter: a Lucide glyph, then the number, with the word in the tooltip. The number
	//! sits next to its glyph (not right-aligned) so pairs read as pairs. STAT_TEXT_OFFSET is the
	//! list's name indent, so the glyph gap matches the checkbox-to-name gap below.
	protected UIActionText CreateRosterStat( Widget parent, string icon, string tooltip, float width, bool valueLeftAligned = true )
	{
		UIActionText stat = UIActionManager.CreateText( parent, "" );
		if ( !stat )
			return NULL;

		stat.SetIcon( JMConstants.Lucide( icon ) );
		stat.SetValueLeftAligned( valueLeftAligned );
		stat.SetLabelOffset( JMPlayerRowWidget.TEXT_OFFSET_PLAYER );
		stat.SetTooltip( tooltip );
		stat.SetWidth( width );

		return stat;
	}

	void Build( Widget layoutRoot )
	{
		Widget leftPanelGrid = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "panel_left_top" ), 3, 1 );

		// -- Roster stats: online vs slots, session peak, rows after filter, ticked; icons with tooltips.
		//  A WrapSpacer, not a 1x4 grid, as online alone carries a slot count ("127 / 128"); the
		//  fractions must sum to under 1.0 because a WrapSpacer wraps its last child at exactly 1.0.
		m_MaxPlayers = 0;
		GetServersResultRow hostData = g_Game.GetHostData();
		if ( hostData )
			m_MaxPlayers = hostData.m_MaxPlayers;

		//! Not equal thirds: Online alone carries a slot count ("61 / 128") and an even third clipped it.
		//! It takes the space the others do not need; those right-align as a pair on the right edge.
		Widget countRowGrid = UIActionManager.CreateWrapSpacerCompact( leftPanelGrid );
		m_PlayerListCount 	= CreateRosterStat( countRowGrid, "users",        "#STR_COT_PLAYER_MODULE_TT_STAT_ONLINE",   0.46 );
		m_PlayerListPeak 	= CreateRosterStat( countRowGrid, "trending-up",  "#STR_COT_PLAYER_MODULE_TT_STAT_PEAK",     0.26, false );
		m_PlayerListSelected 	= CreateRosterStat( countRowGrid, "square-check", "#STR_COT_PLAYER_MODULE_TT_STAT_SELECTED", 0.26, false );

		// -- Toolbar --------------------------------------------------------
		//  Four icon-only pills on one line (each has a tooltip). GridSpacer, not WrapSpacer: a
		//  WrapSpacer reserves margin and padding per child, so four children at 0.24 overflow and
		//  the fourth wraps.
		Widget toolbarRowGrid = UIActionManager.CreateGridSpacer( leftPanelGrid, 1, 5 );
		m_PlayerPrefSave 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.Lucide( "save" ), this, "" );
		if ( m_PlayerPrefSave ) m_PlayerPrefSave.SetOnClick( this, "OnClick_PlayerPrefSave" );
		m_PlayerPrefLoad 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.Lucide( "folder-open" ), this, "" );
		if ( m_PlayerPrefLoad ) m_PlayerPrefLoad.SetOnClick( this, "OnClick_PlayerPrefLoad" );
		m_PlayerListSelectAll 	= UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.ICON_SELECT_ALL, this, "Event_SelectAllPlayerList" );
		m_PlayerListDeSelectAll = UIActionManager.CreateIconButton( toolbarRowGrid, JMConstants.ICON_DESELECT_ALL, this, "Event_DeSelectAllPlayerList" );
		m_Filter.CreateButton( toolbarRowGrid );

		if ( m_PlayerPrefSave )        m_PlayerPrefSave.SetTooltip( "#STR_COT_GENERIC_SAVE" );
		if ( m_PlayerPrefLoad )        m_PlayerPrefLoad.SetTooltip( "#STR_COT_GENERIC_LOAD" );
		if ( m_PlayerListSelectAll )   m_PlayerListSelectAll.SetTooltip( "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL" );
		if ( m_PlayerListDeSelectAll ) m_PlayerListDeSelectAll.SetTooltip( "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL" );

		//! The grid sizes its row to the tallest child, so shrinking all four
		//! shrinks the row - and the list below grows by the difference.
		if ( m_PlayerPrefSave )        m_PlayerPrefSave.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		if ( m_PlayerPrefLoad )        m_PlayerPrefLoad.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		if ( m_PlayerListSelectAll )   m_PlayerListSelectAll.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		if ( m_PlayerListDeSelectAll ) m_PlayerListDeSelectAll.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );

		// -- Filter row -----------------------------------------------------
		//  Sort direction and filter share a line. The compact spacer has no margin or padding, so the
		//  two fractions are the whole story; they must sum to under 1.0 (a WrapSpacer wraps at exactly 1.0).
		Widget filterPanelGrid = UIActionManager.CreateWrapSpacerCompact( leftPanelGrid );

		m_PlayerListSort = UIActionManager.CreateImageButtonToggle( filterPanelGrid, JMConstants.Lucide( "arrow-down-a-z" ), JMConstants.Lucide( "arrow-down-z-a" ), this, "Event_UpdatePlayerList", 0.13 );
		if ( m_PlayerListSort )
		{
			m_PlayerListSort.SetTooltip( "#STR_COT_PLAYER_MODULE_LEFT_SORT_TOOLTIP" );

			//! The toggle's layout is a 32px square - the toolbar pill height,
			//! not the search box's. The search box's visible chrome is 22px
			//! centred inside a 30px root, so both are pinned to 22 here: same
			//! drawn height, and with equal roots the spacer aligns their tops.
			m_PlayerListSort.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		}

		m_PlayerListFilter = UIActionManager.CreateSearchBox( filterPanelGrid, this, "Event_UpdatePlayerList", "#STR_COT_GENERIC_SEARCH" );
		if ( m_PlayerListFilter )
		{
			m_PlayerListFilter.SetWidth( 0.85 );
			m_PlayerListFilter.SetFixedHeight( JMFormBase.HEADER_CONTROL_HEIGHT );
		}

		m_PlayerListScroller 	= UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel_left_bottom" ) );
		m_PlayerListRows 		= UIActionManager.CreateActionRows( m_PlayerListScroller.GetContentWidget() );
		
		for ( int i = 0; i < 2; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			gsw.Show( false );

			for ( int j = 0; j < 100; j++ )
			{
				Widget prWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_widget.layout", gsw );
				
				if ( !prWidget )
					continue;

				JMPlayerRowWidget prScript;
				prWidget.GetScript( prScript );

				if ( !prScript )
					continue;

				prScript.SetPlayer( "" );
				prScript.Menu = m_Form;

				m_PlayerList.Insert( prScript );
			}
		}

		m_PlayerListScroller.UpdateScroller();

		//! Drawn over the list (it is the later child), shown only while the roster has not arrived yet
		m_Busy = new JMBusySpinner( layoutRoot.FindAnyWidget( "panel_left_bottom" ), 32, 24 );
	}

	//! While the server has not sent the roster the list is simply empty, which reads as
	//! "nobody is here". A spinning loader says it is still being fetched instead. Called every frame.
	void UpdateBusy()
	{
		if ( !m_Busy )
			return;

		m_Busy.Update( IsFetching() );
	}

	//! The form was just opened: the roster on screen is whatever was cached (at the very least,
	//! the admin's own row) until the server answers this open's request.
	void BeginFetch()
	{
		m_FetchBaseline = GetPermissionsManager().GetRosterBatchCount();
		m_FetchStart = g_Game.GetTickTime();
	}

	//! Waiting for the first roster reply since the form opened. Held for a moment even when the
	//! reply is quick, so it reads as "loading" rather than as a flicker, and given up after a
	//! while so a lost reply cannot leave it spinning for good. Offline there is no server to wait for.
	protected bool IsFetching()
	{
		if ( IsMissionOffline() )
			return false;

		float elapsed = g_Game.GetTickTime() - m_FetchStart;

		if ( elapsed > FETCH_TIMEOUT )
			return false;

		return elapsed < FETCH_MIN_SHOWN || GetPermissionsManager().GetRosterBatchCount() == m_FetchBaseline;
	}

	void OnClick_PlayerPrefSave( UIActionBase action )
	{
		m_Form.CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_SAVE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_SavePlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_SavePlayerListPref02" );
	}

	void OnClick_SavePlayerListPref01()
	{
		m_PlayersPref1 = new TStringArray;
		SavePlayerListPref(m_PlayersPref1);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_SavePlayerListPref02()
	{
		m_PlayersPref2 = new TStringArray;
		SavePlayerListPref(m_PlayersPref2);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_PlayerPrefLoad( UIActionBase action )
	{
		m_Form.CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_LOAD_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_LoadPlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_LoadPlayerListPref02" );
	}

	void OnClick_LoadPlayerListPref01()
	{
		LoadPlayerListPref(m_PlayersPref1);
	}

	void OnClick_LoadPlayerListPref02()
	{
		LoadPlayerListPref(m_PlayersPref2);
	}

	void SavePlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			if (player.IsChecked())
				playerArr.Insert(player.GetGUID());
		}
	}

	void LoadPlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			bool state = playerArr.Find(player.GetGUID()) != -1;
			
			if (state)
				JM_GetSelected().AddPlayer(player.GetGUID());
			else
				JM_GetSelected().RemovePlayer(player.GetGUID());
			
			player.SetChecked(state);
		}
		
		m_Form.UpdateUI();
		UpdatePlayerCount();
	}

	void Event_UpdatePlayerList( UIEvent eid, UIActionBase action )
	{
		UpdatePlayerList( true );
	}

	void Event_DeSelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(false);
	}

	void Event_SelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(true);
	}

	//! Mark the one row whose player fills the right-hand panel. Cheap enough to
	//! run over the whole pool: the list is a fixed 200 rows and this only fires
	//! on a selection change or a list rebuild, not per frame.
	void ApplyListFocus()
	{
		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row )
				continue;

			row.SetFocused( m_Form.m_LastSelectedGuid != "" && row.GetGUID() == m_Form.m_LastSelectedGuid );
		}
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].SetChecked( checked );
			}
		}

		RefreshRoleHeaders();

		m_Form.UpdateUI();

		UpdatePlayerCount();
	}

	void OnPlayer_Button( string guid, bool check )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].SetChecked( check );
			} else
			{
				m_PlayerList[i].SetChecked( false );
			}
		}

		RefreshRoleHeaders();

		m_Form.UpdateUI();

		UpdatePlayerCount();
	}

	//! Sorts by display name. array<T>.Sort() only orders plain strings, so the names are sorted alone
	//! and matched back by CONSUMING one remaining player per name; a bare comparison matched the SAME
	//! index for two players sharing a name and silently dropped one from the list.
	protected void SortPlayersArray( out array< JMPlayerInstance > players, bool isReversed )
	{
		int count = players.Count();

		TStringArray pNames = new TStringArray;
		for ( int i = 0; i < count; i++ )
			pNames.Insert( players[i].GetName() );

		pNames.Sort( isReversed );

		array< JMPlayerInstance > remaining = new array< JMPlayerInstance >;
		remaining.Copy( players );

		array< JMPlayerInstance > sorted = new array< JMPlayerInstance >;

		for ( i = 0; i < count; i++ )
		{
			string wantName = pNames[i];

			for ( int j = 0; j < remaining.Count(); j++ )
			{
				if ( !remaining[j] )
					continue;

				if ( remaining[j].GetName() != wantName )
					continue;

				sorted.Insert( remaining[j] );
				remaining[j] = NULL;
				break;
			}
		}

		players.Clear();
		players.Copy( sorted );
	}

	void UpdatePlayerCount()
	{
		//! The roster, not the local entity list: this is the same set of players
		//! the list below is built from, so the two can never disagree.
		int online;
		JMPermissionManager permissions = GetPermissionsManager();
		if ( permissions )
			online = permissions.GetPlayers().Count();

		if ( online > s_PeakPlayerCount )
			s_PeakPlayerCount = online;

		string onlineText = "" + online;
		if ( m_MaxPlayers > 0 )
			onlineText = onlineText + " / " + m_MaxPlayers;

		m_PlayerListCount.SetText( onlineText );
		m_PlayerListPeak.SetText( "" + s_PeakPlayerCount );
		m_PlayerListSelected.SetText( "" + JM_GetSelected().GetPlayers(false).Count() );
	}

	void SelectAllPlayerList(bool state = true)
	{
		//! The filtered roster rather than the visible rows: a collapsed group
		//! has no rows on screen, and "select all" that quietly skipped it would
		//! be selecting less than the list says it is holding.
		foreach ( string memberRole, TStringArray memberGuids : m_RoleMembers )
		{
			foreach ( string guid : memberGuids )
			{
				if ( guid == string.Empty )
					continue;

				if ( state )
					JM_GetSelected().AddPlayer( guid );
				else
					JM_GetSelected().RemovePlayer( guid );
			}
		}

		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row || row.IsHeader() || row.GetGUID() == string.Empty )
				continue;

			row.SetChecked( state );
		}

		RefreshRoleHeaders();

		m_Form.UpdateUI();
		UpdatePlayerCount();
	}

	//! Roles are stored lowercase because that is how the permission files name
	//! them; a header is a heading, so it gets a capital.
	string RoleDisplayName( string role )
	{
		if ( role.Length() == 0 )
			return role;

		string head = role.Substring( 0, 1 );
		head.ToUpper();

		return head + role.Substring( 1, role.Length() - 1 );
	}

	//! Re-derive every header's box from the selection set. A member row
	//! toggling does not know which header aggregates it, so the headers are
	//! recomputed rather than kept in step incrementally.
	void RefreshRoleHeaders()
	{
		foreach ( JMPlayerRowWidget headerRow: m_PlayerList )
		{
			if ( !headerRow )
				continue;

			if ( !headerRow.IsHeader() )
				continue;

			headerRow.SetChecked( IsRoleFullySelected( headerRow.GetRole() ) );
		}
	}

	//! Called by a header row's chevron or a click on it. Not symmetrical: expand flips state and
	//! rebuilds BEFORE animating in; collapse keeps the rows until the animation ends and flips
	//! state at the END - see FinishGroupAnimation.
	void OnRoleHeader_Toggled( string role )
	{

		//! A second click while the last fold is still running would leave the
		//! first one's rows stranded at whatever height they had reached.
		FinishGroupAnimation();

		int at = m_CollapsedRoles.Find( role );

		if ( at >= 0 )
		{
			m_CollapsedRoles.Remove( at );
			UpdatePlayerList( true );
			BeginGroupAnimation( role, true );
			return;
		}

		BeginGroupAnimation( role, false );
	}

	protected void BeginGroupAnimation( string role, bool expanding )
	{
		m_AnimRows.Clear();
		m_AnimHeader = NULL;
		m_AnimRole = role;
		m_AnimExpanding = expanding;
		m_AnimTime = 0;

		TStringArray guids;
		m_RoleMembers.Find( role, guids );

		foreach ( JMPlayerRowWidget row: m_PlayerList )
		{
			if ( !row )
				continue;

			if ( row.IsHeader() )
			{
				if ( row.GetRole() == role )
					m_AnimHeader = row;

				continue;
			}

			if ( !guids )
				continue;

			if ( guids.Find( row.GetGUID() ) < 0 )
				continue;

			m_AnimRows.Insert( row );
		}

		//! Nothing to fold - an empty group still has to flip its chevron.

		if ( m_AnimRows.Count() == 0 )
		{
			FinishGroupAnimation();
			return;
		}

		if ( expanding )
			ApplyGroupAnimation( 0 );
		else
			ApplyGroupAnimation( 1.0 );
	}

	protected void ApplyGroupAnimation( float t )
	{
		foreach ( JMPlayerRowWidget row: m_AnimRows )
		{
			if ( row )
				row.SetRevealProgress( t );
		}

		if ( m_AnimHeader )
			m_AnimHeader.SetChevronProgress( t );
	}

	//! Snap to the end state and commit it. Safe to call when nothing is
	//! running, which is what makes it usable as an "interrupt whatever is
	//! going on" from OnHide and from a second click.
	void FinishGroupAnimation()
	{
		if ( !IsGroupAnimating() )
			return;

		string role = m_AnimRole;
		bool wasExpanding = m_AnimExpanding;

		m_AnimRole = "";
		m_AnimHeader = NULL;
		m_AnimRows.Clear();
		m_AnimTime = 0;

		if ( wasExpanding )
		{
			//! Rows are already in the list; all the animation was doing was
			//! growing them into place.
			UpdatePlayerList( true );
			return;
		}

		if ( m_CollapsedRoles.Find( role ) < 0 )
			m_CollapsedRoles.Insert( role );

		UpdatePlayerList( true );
	}

	void UpdateGroupAnimation( float timeSlice )
	{
		if ( !IsGroupAnimating() )
			return;

		m_AnimTime += timeSlice;

		float t = m_AnimTime / GROUP_ANIM_DURATION;

		if ( t >= 1.0 )
		{
			FinishGroupAnimation();
			return;
		}

		if ( m_AnimExpanding )
			ApplyGroupAnimation( t );
		else
			ApplyGroupAnimation( 1.0 - t );
	}

	//! Called by a header row's checkbox. Acts on the members the header counted,
	//! which is the filtered set - not on every holder of the role.
	void OnRoleHeader_Checked( string role, bool checked )
	{
		TStringArray guids;
		if ( !m_RoleMembers.Find( role, guids ) )
			return;

		foreach ( string guid : guids )
		{
			if ( checked )
				JM_GetSelected().AddPlayer( guid );
			else
				JM_GetSelected().RemovePlayer( guid );
		}

		m_Form.UpdateUI();
		UpdatePlayerList( true );
	}

	//! The pool is split across Content_Row_0N grids of 100 rows each, and a grid
	//! is only revealed once the emit actually reaches it. Returns false when the
	//! grid a row would need does not exist, which is the end of the pool.
	protected bool RevealRowBlock( int entryId, inout int contentID )
	{
		if ( entryId % 100 != 0 )
			return true;

		GridSpacerWidget spacer;
		if ( !Class.CastTo( spacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + contentID ) ) )
			return false;

		spacer.Show( true );
		contentID++;

		return true;
	}

	//! force=true rebuilds unconditionally, for what the roster version can't see (search/sort/filter,
	//! fold state, checkboxes). The 1500ms poll uses force=false so an unchanged roster is a cheap no-op.
	void UpdatePlayerList( bool force = false )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "UpdatePlayerList" );
		#endif

		if ( !IsMissionOffline() )
			GetCommunityOnlineTools().RefreshClients();

		//! A rebuild resets every row to its resting height, which would cut a
		//! fold off halfway. The 1500ms refresh can wait out the ~160ms fold.
		if ( IsGroupAnimating() )
			return;

		int rosterVersion = GetPermissionsManager().GetRosterVersion();
		if ( !force && rosterVersion == m_LastRosterVersion )
		{
			UpdatePlayerCount();
			return;
		}
		m_LastRosterVersion = rosterVersion;

		int contentID;
		GridSpacerWidget parentSpacer;
		while ( Class.CastTo( parentSpacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + contentID ) ) )
		{
			contentID++;
			parentSpacer.Show( false );
		}

		bool isReversed = m_PlayerListSort.IsToggled();

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		SortPlayersArray( players, isReversed );

		JMSearchMatcher matcher = new JMSearchMatcher( m_PlayerListFilter.GetText() );
		bool isFiltering = !matcher.IsEmpty();

		//! Bucket the survivors of the filter by role, remembering the order the
		//! roles were first seen so the sort below has something to sort.
		TStringArray roleOrder = new TStringArray;
		map< string, ref array< JMPlayerInstance > > groups = new map< string, ref array< JMPlayerInstance > >;

		m_RoleMembers.Clear();

		array< JMPlayerInstance > bucket;
		TStringArray bucketGuids;

		//! One row per guid: checkbox, focus bar and row menu all key on it, so a guid emitted twice
		//! produced rows that ticked, focused and acted as one.
		map< string, bool > seenGuids = new map< string, bool >;

		foreach ( JMPlayerInstance cPlayer: players )
		{
			string cGuid = cPlayer.GetGUID();

			if ( cGuid == "" || seenGuids.Contains( cGuid ) )
				continue;

			seenGuids.Insert( cGuid, true );

			if ( isFiltering && !matcher.Matches( cPlayer.GetName() ) )
				continue;

			if ( !m_Form.FilterPlayer( cPlayer ) )
				continue;

			string role = GetPrimaryRole( cPlayer );

			if ( !groups.Find( role, bucket ) )
			{
				bucket = new array< JMPlayerInstance >;
				groups.Insert( role, bucket );
				roleOrder.Insert( role );

				bucketGuids = new TStringArray;
				m_RoleMembers.Insert( role, bucketGuids );
			}

			m_RoleMembers.Find( role, bucketGuids );

			bucket.Insert( cPlayer );
			bucketGuids.Insert( cPlayer.GetGUID() );
		}

		roleOrder.Sort( isReversed );

		//! You are always the first row of your own category, whichever way the sort points.
		//! The guid list is reordered in step: the header's "all selected" test reads it
		//! positionally against the bucket.
		string selfGUID = GetPermissionsManager().GetClientGUID();
		if ( selfGUID != "" )
		{
			array< JMPlayerInstance > selfBucket;
			TStringArray selfGuids;

			foreach ( string selfRole: roleOrder )
			{
				selfBucket = groups.Get( selfRole );
				if ( !selfBucket )
					continue;

				int selfIdx = -1;
				for ( int si = 0; si < selfBucket.Count(); si++ )
				{
					if ( selfBucket[si].GetGUID() == selfGUID )
					{
						selfIdx = si;
						break;
					}
				}

				if ( selfIdx <= 0 )
					continue;

				JMPlayerInstance selfInstance = selfBucket[selfIdx];
				selfBucket.Remove( selfIdx );
				selfBucket.InsertAt( selfInstance, 0 );

				if ( m_RoleMembers.Find( selfRole, selfGuids ) && selfIdx < selfGuids.Count() )
				{
					selfGuids.Remove( selfIdx );
					selfGuids.InsertAt( selfGUID, 0 );
				}

				break;
			}
		}

		//! Every pool row starts blank, so anything the emit below does not reach
		//! is hidden rather than left showing whoever it carried last time.
		int entryId;
		int maxThreshold = m_PlayerList.Count();

		while ( entryId < maxThreshold )
		{
			m_PlayerList[entryId].SetPlayer( "" );
			entryId++;
		}

		entryId = 0;
		contentID = 0;

		foreach ( string roleName: roleOrder )
		{
			if ( entryId >= maxThreshold )
				break;

			if ( !RevealRowBlock( entryId, contentID ) )
				break;

			array< JMPlayerInstance > members = groups.Get( roleName );
			bool collapsed = IsRoleCollapsed( roleName );

			//! Negated into its own local rather than passed as `!collapsed`.
			//! Enforce did not carry the negation through the argument list - the
			//! header was emitted with collapsed=true and arrived at the row with
			//! expanded=true, which is why a collapsed group kept a down arrow.
			bool expanded = true;
			if ( collapsed )
				expanded = false;

			m_PlayerList[entryId].SetRoleHeader( roleName, RoleDisplayName( roleName ), members.Count(), expanded, IsRoleFullySelected( roleName ), IsAdminRole( roleName ) );
			entryId++;

			if ( collapsed )
				continue;

			foreach ( JMPlayerInstance member: members )
			{
				if ( entryId >= maxThreshold )
					break;

				if ( !RevealRowBlock( entryId, contentID ) )
					break;

				m_PlayerList[entryId].SetPlayer( member.GetGUID() );
				entryId++;
			}
		}

		m_PlayerListFilter.SetTextPreview(matcher.GetClosestMatch());

		//! Rows are a reused pool, so the row that carried the focused player
		//! before this rebuild is probably showing somebody else now.
		ApplyListFocus();

		UpdatePlayerCount();

		m_PlayerListScroller.UpdateScroller();
	}
}
