//! "Position" tab of JMPlayerForm - coordinates, teleports, the teleport history
//! and the embedded map of the selected player. Back-reference to the owning
//! form, same shape as JMPlayerRowWidget.Menu.
class JMPlayerFormTabPosition: JMFormTab
{
	protected JMPlayerForm m_Form;
    UIActionImageButton m_CopyRotationPlayer;
    UIActionImageButton m_CopyPositionPlayer;
    UIActionImageButton m_PastePositionPlayer;
	UIActionButton m_TeleportToMe;
	UIActionButton m_TeleportMeTo;
	UIActionEditableText m_PositionX;
	bool m_PositionXUpdated;
	UIActionEditableText m_PositionY;
	bool m_PositionYUpdated;
	UIActionEditableText m_PositionZ;
	bool m_PositionZUpdated;
	UIActionImageButton m_PositionRefresh;
	UIActionImageButton m_Position;

	//! Recenters the Position tab map on the selected player. Panning is only
	//! useful if something puts the view back.
	UIActionImageButton m_PositionRecenter;

	//! Vehicle Manager's map made generic (UIActionMap) - markers, hover and
	//! click routing are handled there instead of by hand here.
	UIActionMap m_PositionMap;

	//! Whether the deferred first-open recentre has already been scheduled.
	bool m_PositionMapCentered;

	//! The last few places the selected player was moved away FROM, most recent
	//! first, one button per entry - a click sends them back to that row.
	//!
	//! The rows are REBUILT rather than updated: the list is at most
	//! JMTeleportHistory.MAX_ENTRIES long, and an undo several steps deep drops
	//! every row above the one it lands on in a single go.
	UIActionCard m_TeleportHistoryCard;
	UIActionImageButton m_TeleportHistoryClear;
	UIActionImageButton m_TeleportHistoryRedo;
	Widget m_TeleportHistoryHost;
	Widget m_TeleportHistoryGrid;
	ref array<ref UIActionButton> m_TeleportHistoryButtons;

	//! The two icon buttons beside each row, in the same order as the row
	//! buttons above - each array's index IS the step it acts on, because all
	//! three are filled in one pass.
	ref array<ref UIActionImageButton> m_TeleportHistoryCopy;
	ref array<ref UIActionImageButton> m_TeleportHistoryFocus;
	UIActionText m_TeleportHistoryEmpty;

	//! What the panel above was last built for. RefreshTeleports runs ten times
	//! a second, and rebuilding five buttons on every one of those would churn
	//! widgets for nothing - and a rebuild under the cursor steals the press.
	string m_TeleportHistoryGuid;
	int    m_TeleportHistoryCount;

	//! Timestamp of the newest row the panel was built from.
	//!
	//! The count alone is not enough to notice a change: once five steps are
	//! recorded the list is capped, so a sixth teleport pushes one off the
	//! bottom and leaves the count at five.
	int    m_TeleportHistoryTop;

	//! Whether the coordinates on screen came from the player's entity on this
	//! client rather than from the roster snapshot the server pushes.
	bool m_PositionIsLive;

	//! Widths of the three controls in a history row, as FRACTIONS of the row.
	//!
	//! Never a fixed pixel size beside a fraction: a WrapSpacer wraps the moment
	//! its children total more than the row, and mixing the two units makes that
	//! edge impossible to reason about. These sum to less than 1.
	static const float HISTORY_GO_W  = 0.70;
	static const float HISTORY_BTN_W = 0.13;

	//! Zoom the map returns to when it is recentered.
	static const float MAP_DEFAULT_SCALE = 0.15;

	//! Zoom the map takes when it is pointed at one recorded step. Closer than
	//! the default: the point of focusing a step is to see WHERE it is, and at
	//! the overview scale three positions in the same town are one dot.
	static const float MAP_FOCUS_SCALE = 0.06;
	static const int MAP_MARK_TARGET = 0xFFFFFF00;
	static const int MAP_MARK_SELF   = 0xFF00FF00;

	//! Coordinate precision. The roster copy of a position is a snapshot sent a
	//! few times a second; when the entity itself is loaded here it can be read
	//! directly, which is both current and worth printing to millimetres.
	static const int COORD_DECIMALS_SYNCED = 2;
	static const int COORD_DECIMALS_LIVE   = 3;

	//! Drawn height of the embedded map, in layout pixels.
	static const int MAP_HEIGHT = 220;

	//! Direct children this tab's builder adds to its panel, with headroom. Over-
	//! provisioning is free under Size-To-Content-V; running short silently clips the
	//! last rows with no error anywhere. The form sizes the panel from this.
	static const int PANEL_ROWS = 8;

	void JMPlayerFormTabPosition( JMPlayerForm form )
	{
		m_Form = form;

		m_TeleportHistoryButtons = new array<ref UIActionButton>;
		m_TeleportHistoryCopy    = new array<ref UIActionImageButton>;
		m_TeleportHistoryFocus   = new array<ref UIActionImageButton>;
		m_TeleportHistoryCount   = -1;
	}

	vector GetPosition()
	{
		vector pos = vector.Zero;
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();
		return pos;
	}

	void SetPositionTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Form.UpdateLastChangeTime();
		m_Form.m_Module.TeleportTo(GetPosition(), targets );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		InitActionWidgetsPosition( panel );
	}

	override void OnFocus()
	{
		OnEnterPositionTab();
	}

	//! The form was hidden: a MapWidget left visible behind a hidden form keeps rendering.
	void OnFormHide()
	{
		if ( m_PositionMap )
			m_PositionMap.Hide();
	}

	//! The position map's marker widgets need reprojecting onto its current
	//! pan/zoom every frame the tab is up - see UIActionMap.TickMarkers().
	void Tick()
	{
		if ( m_PositionMap )
			m_PositionMap.TickMarkers();
	}

	//! Coordinates and teleport history, from the roster's copy of the player.
	void RefreshPosition( bool force = false )
	{
		RefreshTeleports( force );
		SyncTeleportHistory();
	}

	void BindPermissions()
	{
		m_Form.BindPermission( m_PositionX,             JMConstants.PERM_PLAYER_TELEPORT_POSITION     );
		m_Form.BindPermission( m_PositionY,             JMConstants.PERM_PLAYER_TELEPORT_POSITION     );
		m_Form.BindPermission( m_PositionZ,             JMConstants.PERM_PLAYER_TELEPORT_POSITION     );
		m_Form.BindPermission( m_Position,              JMConstants.PERM_PLAYER_TELEPORT_POSITION     );
		m_Form.BindPermission( m_TeleportToMe,          JMConstants.PERM_PLAYER_TELEPORT_POSITION     );

		if ( JM_GetSelected().NumPlayers(false) == 1 )
		{
			m_Form.BindPermission( m_TeleportMeTo, JMConstants.PERM_PLAYER_TELEPORT_SENDERTO );
		} else if ( m_TeleportMeTo ) {
			m_TeleportMeTo.Disable();
		}
	}

	void DisableForOffline()
	{
		if ( m_TeleportToMe )
			m_TeleportToMe.Disable();
		if ( m_TeleportMeTo )
			m_TeleportMeTo.Disable();
	}

	//! The selected player changed: put the map back on them.
	void OnSelectionChanged()
	{
		RecenterMap();
	}

	Widget InitActionWidgetsPosition( Widget actionsParent )
	{
		// header + toolbar + vector + map header + map slot + teleport header
		// + teleport row + history header + history rows + divider
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 12, 1 );

		UIActionCard section0Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_POSITION" );
		Widget section0 = section0Card.GetContent();

		// -- Card quick actions ---------------------------------------------
		//  Every verb that acts on the three coordinate fields lives in the
		//  card's own title bar - the same place the Inventory and Session
		//  cards put theirs. Refresh re-reads the player's live coordinates
		//  into the boxes, the clipboard trio copies or pastes them, and the
		//  map pin pushes whatever is typed back to the server. Left to right
		//  that reads reload, clipboard tools, apply.
		//
		//  Icon-only, so each keeps the label it lost as its tooltip, and the
		//  three clipboard buttons answer a click by flashing a check mark.
		m_PositionRefresh     = section0Card.AddRefreshButton( this, "Click_RefreshTeleports", "#STR_COT_PLAYER_MODULE_TT_REFRESH_COORDS" );
		m_CopyRotationPlayer  = section0Card.AddCardHeaderAction( JMConstants.Lucide( "rotate-3d" ), this, "Click_CopyPlayerRotation", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_ORIENTATION_COPY" );
		m_CopyPositionPlayer  = section0Card.AddCopyButton( this, "Click_CopyPlayerPostion", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_COPY" );
		m_PastePositionPlayer = section0Card.AddPasteButton( this, "Click_PastePlayerPostion", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_PASTE" );
		m_Position            = section0Card.AddCardHeaderAction( JMConstants.Lucide( "map-pin-check" ), this, "Click_SetPosition", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES" );

		Widget positionActionsVec = UIActionManager.CreateGridSpacer( section0, 1, 3 );

		m_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:", this, "Change_PositionX" );
		m_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:", this, "Change_PositionY" );
		m_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:", this, "Change_PositionZ" );

		m_PositionX.SetOnlyNumbers( true );
		m_PositionY.SetOnlyNumbers( true );
		m_PositionZ.SetOnlyNumbers( true );

		UpdatePositionSourceHint();

		// -- Map ------------------------------------------------------------
		//  Built here rather than lazily into a reserved slot: every panel this
		//  form can reserve space with is a "Size To Content V" UIPanel, and a
		//  MapWidget reports no content height, so the slot collapses to
		//  nothing and the map is created inside a zero-height box - present,
		//  handled and invisible. Its own layout carries an exact height, so
		//  parenting it directly to the tab grid is both simpler and correct,
		//  and it lands between the two section headers instead of being
		//  appended to the end of the grid on first open.
		//
		//  It starts hidden and is only shown while the tab is up: a MapWidget
		//  left visible keeps rendering behind everything else.
		UIActionCard section1Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_MAP" );
		Widget section1 = section1Card.GetContent();

		// Recentre acts on the map as a whole, not on anything drawn inside it,
		// so it belongs in the map card's title bar rather than in a toolbar
		// row that would cost a line of height above the map itself.
		m_PositionRecenter = section1Card.AddCardHeaderAction( JMConstants.Lucide( "locate-fixed" ), this, "Click_RecenterMap" );
		m_PositionRecenter.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_RECENTER_MAP" );

		InitActionWidgetsMap( section1 );

		UIActionCard section2Card = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_TELEPORT" );
		Widget section2 = section2Card.GetContent();
		Widget teleportActions = UIActionManager.CreateGridSpacer( section2, 1, 2 );
		m_TeleportToMe   = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_TELEPORT_TO_ME", this, "Click_TeleportToMe"     );
		m_TeleportToMe.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_TELEPORT_TO_ME" );
		m_TeleportMeTo   = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_TELEPORT_ME_TO", this, "Click_TeleportMeTo"     );
		m_TeleportMeTo.SetTooltip( "#STR_COT_PLAYER_MODULE_TT_TELEPORT_ME_TO" );

		// -- Teleport history ------------------------------------------------
		//  Where this player has been moved away from, newest first. Recorded on
		//  this client as the admin issues the moves - it is a record of what
		//  THIS session did, not a server-side audit trail, so it is empty for
		//  teleports another admin issued or ones from before this connection.
		m_TeleportHistoryCard = UIActionManager.CreateCard( parent, "#STR_COT_PLAYER_MODULE_SECTION_TELEPORT_HISTORY" );
		m_TeleportHistoryHost = m_TeleportHistoryCard.GetContent();

		//  Clearing is card-level - it wipes every row at once - so it sits in
		//  the title bar with the other card-level wipes and keeps the caption
		//  it lost as an icon-only button's tooltip.
		//  Redo is card-level too: it acts on the stack, not on any one row -
		//  the rows are the steps still ahead of the target, and a redone step
		//  is by definition not one of them.
		m_TeleportHistoryRedo = m_TeleportHistoryCard.AddCardHeaderAction( JMConstants.Lucide( "redo-2" ), this, "Click_RedoTeleport", "#STR_COT_TELEPORT_REDO" );

		m_TeleportHistoryClear = m_TeleportHistoryCard.AddDeleteButton( this, "Click_ClearTeleportHistory", "#STR_COT_TELEPORT_HISTORY_CLEAR" );
		m_TeleportHistoryClear.SetColor( JMTheme.DANGER_FILL );

		RebuildTeleportHistory();

		return parent;
	}

	//! Rebuild the history rows for whoever is selected.
	//!
	//! Cheap enough to call outright - it is at most five buttons - but never
	//! from inside one of their own click handlers: the engine answers a
	//! vanished press target by recentring the cursor, so a rebuild caused by a
	//! click is deferred a tick.
	void RebuildTeleportHistory()
	{
		if ( !m_TeleportHistoryHost )
			return;

		//! Drop the script references BEFORE the widgets they wrap, so a button
		//! is never left alive holding a layoutRoot that has been unlinked.
		m_TeleportHistoryButtons.Clear();
		m_TeleportHistoryCopy.Clear();
		m_TeleportHistoryFocus.Clear();
		m_TeleportHistoryEmpty = NULL;

		if ( m_TeleportHistoryGrid )
		{
			m_TeleportHistoryGrid.Unlink();
			m_TeleportHistoryGrid = NULL;
		}

		array<ref JMTeleportHistoryEntry> entries = JMTeleportHistory.Entries( JMTeleportHistory.PlayerKey( m_Form.m_LastSelectedGuid ) );

		int count = 0;

		if ( entries )
			count = entries.Count();

		m_TeleportHistoryGuid  = m_Form.m_LastSelectedGuid;
		m_TeleportHistoryCount = count;
		m_TeleportHistoryTop   = 0;

		//! Greyed rather than hidden: a header action that comes and goes moves
		//! the two beside it every time the stack changes.
		if ( m_TeleportHistoryRedo )
		{
			if ( JMTeleportHistory.HasRedo( JMTeleportHistory.PlayerKey( m_Form.m_LastSelectedGuid ) ) )
				m_TeleportHistoryRedo.Enable();
			else
				m_TeleportHistoryRedo.Disable();
		}

		if ( count > 0 )
			m_TeleportHistoryTop = entries[0].Time;

		if ( count == 0 )
		{
			m_TeleportHistoryGrid  = UIActionManager.CreateGridSpacer( m_TeleportHistoryHost, 1, 1 );
			m_TeleportHistoryEmpty = UIActionManager.CreateText( m_TeleportHistoryGrid, "#STR_COT_TELEPORT_HISTORY_EMPTY", "" );

			return;
		}

		m_TeleportHistoryGrid = UIActionManager.CreateGridSpacer( m_TeleportHistoryHost, count, 1 );

		for ( int i = 0; i < count; i++ )
		{
			//! A WrapSpacer packs left to right in creation order, so this IS
			//! the order on screen: the step, then copy, then focus.
			Widget rowSpacer = UIActionManager.CreateWrapSpacerCompact( m_TeleportHistoryGrid, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			//! "1. 7521 305 8134  (12s)" - the step, where it was, how long ago.
			string label = string.Format( "%1. %2  (%3)", i + 1, entries[i].FormatPosition(), entries[i].FormatAge() );

			UIActionButton row = UIActionManager.CreateButton( rowSpacer, label, this, "OnClick_TeleportHistory" );
			row.SetWidth( HISTORY_GO_W );
			row.SetTooltip( "#STR_COT_TELEPORT_HISTORY_ROW_DESC" );

			//! Copies in the same "<x, y, z>" shape the coordinate box does, so
			//! it pastes straight back into that box or into an ESP paste.
			UIActionImageButton copy = UIActionManager.CreateIconButton( rowSpacer, JMConstants.Lucide( "copy" ), this, "OnClick_TeleportHistoryCopy" );
			copy.SetWidth( HISTORY_BTN_W );
			copy.SetTooltip( "#STR_COT_TELEPORT_HISTORY_COPY" );

			//! Shows where the step was WITHOUT moving anyone - the one way to
			//! tell three sets of coordinates apart is to look at them on a map.
			UIActionImageButton focus = UIActionManager.CreateIconButton( rowSpacer, JMConstants.Lucide( "locate-fixed" ), this, "OnClick_TeleportHistoryFocus" );
			focus.SetWidth( HISTORY_BTN_W );
			focus.SetTooltip( "#STR_COT_TELEPORT_HISTORY_FOCUS" );

			m_TeleportHistoryButtons.Insert( row );
			m_TeleportHistoryCopy.Insert( copy );
			m_TeleportHistoryFocus.Insert( focus );
		}
	}

	//! The entry a row's controls act on, or NULL if the panel has moved on
	//! since the button was built.
	JMTeleportHistoryEntry TeleportHistoryEntryAt( int index )
	{
		return JMTeleportHistory.EntryAt( JMTeleportHistory.PlayerKey( m_Form.m_LastSelectedGuid ), index );
	}

	//! Rebuild only when there is something new to show.
	//!
	//! Called off the ten-a-second refresh, so it compares first: who is
	//! selected, how many steps are recorded, and when the newest of them was
	//! recorded. The last of those is what catches a push at the cap, where a
	//! new step drops one off the bottom and the count does not move.
	void SyncTeleportHistory()
	{
		if ( !m_TeleportHistoryHost )
			return;

		if ( m_Form.m_LastSelectedGuid != m_TeleportHistoryGuid )
		{
			RebuildTeleportHistory();

			return;
		}

		string key = JMTeleportHistory.PlayerKey( m_Form.m_LastSelectedGuid );

		if ( JMTeleportHistory.Count( key ) != m_TeleportHistoryCount )
		{
			RebuildTeleportHistory();

			return;
		}

		JMTeleportHistoryEntry newest = JMTeleportHistory.EntryAt( key, 0 );

		if ( newest && newest.Time != m_TeleportHistoryTop )
			RebuildTeleportHistory();
	}

	//! Send the selected player back to the place this row names.
	//!
	//! The rows ABOVE the one clicked go with it - they are places the player
	//! only passed through on the way here, and leaving them in would make the
	//! next undo walk forwards. See JMTeleportHistory.Pop.
	void OnClick_TeleportHistory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton row;

		if ( !Class.CastTo( row, action ) )
			return;

		int index = m_TeleportHistoryButtons.Find( row );

		if ( index < 0 )
			return;

		if ( !JMTeleportHistory.UndoPlayer( m_Form.m_LastSelectedGuid, index ) )
			return;

		//! Deferred: this rebuild destroys the very button the press landed on.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	void OnClick_TeleportHistoryCopy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton button;

		if ( !Class.CastTo( button, action ) )
			return;

		int index = m_TeleportHistoryCopy.Find( button );

		if ( index < 0 )
			return;

		JMTeleportHistoryEntry entry = TeleportHistoryEntryAt( index );

		if ( !entry )
			return;

		//! Same shape as Click_CopyPlayerPostion, so the two are interchangeable
		//! wherever a position is pasted.
		COTFeedback.Copy( "<" + entry.Position[0] + ", " + entry.Position[1] + ", " + entry.Position[2] + ">" );

		//! No toast: the button swaps to a check mark where the click happened.
		button.ShowFeedback();
	}

	void OnClick_TeleportHistoryFocus( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionImageButton button;

		if ( !Class.CastTo( button, action ) )
			return;

		int index = m_TeleportHistoryFocus.Find( button );

		if ( index < 0 )
			return;

		JMTeleportHistoryEntry entry = TeleportHistoryEntryAt( index );

		if ( !entry )
			return;

		FocusMap( entry.Position );
	}

	//! Point the position tab's map at a coordinate, closer in than the default
	//! so the step is placed rather than just centred.
	//!
	//! Split out of RecenterMap because the two differ only in what they centre
	//! on, and the map is a MapWidget either way - nothing is moved, nothing is
	//! sent, the view just changes.
	void FocusMap( vector position )
	{
		if ( !m_PositionMap || position == vector.Zero )
			return;

		m_PositionMap.CenterOn( position, MAP_FOCUS_SCALE );
	}

	void Click_RedoTeleport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMTeleportHistory.RedoPlayer( m_Form.m_LastSelectedGuid, 0 ) )
			return;

		//! Deferred: a redo puts a row back on the panel this button lives in.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	void Click_ClearTeleportHistory( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMTeleportHistory.Clear( JMTeleportHistory.PlayerKey( m_Form.m_LastSelectedGuid ) );

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildTeleportHistory );
	}

	//! Built eagerly with the rest of the tab (see InitActionWidgetsMap), but
	//! kept hidden until the Position tab is actually up - a MapWidget left
	//! visible behind another tab keeps rendering.
	void OnEnterPositionTab()
	{
		if ( m_PositionMap )
			m_PositionMap.Show();

		RebuildTeleportHistory();

		RecenterMap();

		//! First open only: CenterOn lands before the widget has been laid out
		//! on the very first show and is discarded - the map came up on the
		//! world origin and stayed there until the recentre button was pressed
		//! by hand. Every later visit re-enters an already-laid-out map, which
		//! is why this only ever looked broken the first time.
		if ( !m_PositionMapCentered )
		{
			m_PositionMapCentered = true;
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RecenterMap, 100, false );
		}
	}

	void InitActionWidgetsMap( Widget parent )
	{
		m_PositionMap = UIActionManager.CreateMap( parent, this, "OnClick_PositionMap", MAP_HEIGHT );

		if ( !m_PositionMap )
			return;

		m_PositionMap.SetScale( MAP_DEFAULT_SCALE );
		m_PositionMap.Hide();
	}

	//! Put the view back on the selected player. Only ever called on a selection
	//! change, on entering the tab, or from the recenter button - never from the
	//! marker refresh, or the admin could not pan or zoom at all.
	void RecenterMap()
	{
		if ( !m_PositionMap || !m_Form.m_SelectedInstance )
			return;

		// A remote player's position is only as fresh as the last client refresh,
		// and before the first one it is the world origin - which is open sea,
		// so the map would come up as a flat blue square and read as broken.
		// Fall back to the admin's own position until the target has a real one.
		vector center = m_Form.m_SelectedInstance.GetPosition();
		if ( center == vector.Zero )
		{
			JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();
			if ( self )
				center = self.GetPosition();
		}

		m_PositionMap.CenterOn( center, MAP_DEFAULT_SCALE );
	}

	//! Draw the selected player and the admin, and nothing else. The COT map
	//! module already exists for a whole-server view; sixty marks a second here
	//! would only duplicate it.
	//!
	//! Both live under stable ids and get moved in place rather than cleared
	//! and redrawn every poll - UIActionMap.AddMarker replaces the marker
	//! already under an id, so this only ever touches its own two widgets.
	void UpdateMapMarkers()
	{
		if ( !m_PositionMap || !m_Form.m_SelectedInstance )
			return;

		if ( !m_Form.IsTabActive( GetTabId() ) )
			return;

		JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

		//! Looking at your own row is ONE player, so it gets one mark. Drawing
		//! both put the admin name and "You" on top of each other at the same
		//! coordinate, which read as two players standing in the same spot.
		bool targetIsSelf = false;
		if ( self )
			targetIsSelf = ( self.GetGUID() == m_Form.m_SelectedInstance.GetGUID() );

		if ( !targetIsSelf )
			m_PositionMap.AddMarker( "position_target", m_Form.m_SelectedInstance.GetPosition(), m_Form.m_SelectedInstance.GetName(), MAP_MARK_TARGET, UIActionMap.ICON_DOT, "position" );
		else
			m_PositionMap.RemoveMarker( "position_target" );

		if ( self )
			m_PositionMap.AddMarker( "position_self", self.GetPosition(), "#STR_COT_PLAYER_MODULE_MAP_YOU", MAP_MARK_SELF, UIActionMap.ICON_DOT, "position" );

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	//! Ground clicks: a double press fills the coordinate fields and runs the
	//! ordinary teleport-to-coordinates path, confirmation dialog included.
	void OnClick_PositionMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.DOUBLE_CLICK )
			return;

		if ( !m_PositionMap )
			return;

		vector target = m_PositionMap.GetLastClickWorldPos();

		m_PositionX.SetText( target[0].ToString() );
		m_PositionY.SetText( target[1].ToString() );
		m_PositionZ.SetText( target[2].ToString() );

		Click_SetPosition( UIEvent.CLICK, NULL );
	}

	void Click_RecenterMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RecenterMap();
	}

	void Click_SetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES", this, "SetPositionTargets", false );
	}

	void Click_TeleportToMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Form.ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_ME", this, "TeleportToMeTargets", false );
	}

	void TeleportToMeTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		vector pos = GetCurrentPosition();
		if (pos != vector.Zero)
			m_Form.m_Module.TeleportTo(pos, targets );
	}

    void Click_CopyPlayerPostion( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		COTFeedback.Copy("<" + m_PositionX.GetText() + ", " + m_PositionY.GetText() + ", " + m_PositionZ.GetText() + ">");

		m_CopyPositionPlayer.ShowFeedback();
	}

    void Click_CopyPlayerRotation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_Form.m_SelectedInstance )
			return;

		vector rotation = m_Form.m_SelectedInstance.GetOrientation();

		//! Same deal as the position: the entity is here when the player is in
		//! our bubble, and reading it beats the rounded roster snapshot.
		PlayerBase loaded = m_Form.m_SelectedInstance.PlayerObject;
		if ( loaded )
			rotation = loaded.GetOrientation();

		COTFeedback.Copy("<" + FormatCoordinate( rotation[0], COORD_DECIMALS_LIVE ) + ", " + FormatCoordinate( rotation[1], COORD_DECIMALS_LIVE ) + ", " + FormatCoordinate( rotation[2], COORD_DECIMALS_LIVE ) + ">");

		m_CopyRotationPlayer.ShowFeedback();
	}

	void Click_PastePlayerPostion( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		g_Game.CopyFromClipboard(clipboard);

		vector pos = clipboard.BeautifiedToVector();

		// The copies answer a click with a check mark, so the paste has to
		// answer one too - otherwise the only button in the strip that can
		// fail is also the only one that says nothing when it works. A
		// clipboard holding anything that is not a vector flashes a red cross
		// instead: the fields are left alone and nothing is sent.
		if (pos == vector.Zero)
		{
			m_PastePositionPlayer.SetFeedbackIcon( JMConstants.Lucide( "x" ) );
			m_PastePositionPlayer.SetFeedbackColor( JMTheme.DANGER_FILL );
			m_PastePositionPlayer.ShowFeedback();
			return;
		}

		m_PositionX.SetText(FormatCoordinate( pos[0], COORD_DECIMALS_LIVE ));
		m_PositionY.SetText(FormatCoordinate( pos[1], COORD_DECIMALS_LIVE ));
		m_PositionZ.SetText(FormatCoordinate( pos[2], COORD_DECIMALS_LIVE ));

		Click_SetPosition(eid, action);

		m_PastePositionPlayer.SetFeedbackIcon( JMConstants.ICON_CHECK_MARK );
		m_PastePositionPlayer.SetFeedbackColor( JMTheme.SUCCESS_DIM );
		m_PastePositionPlayer.ShowFeedback();
	}

	void Click_TeleportMeTo( UIEvent eid, UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Form.m_Module.TeleportSenderTo( JM_GetSelected().GetPlayers()[0] );
	}

	void Click_RefreshTeleports( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_PositionRefresh.TriggerSpin( 2 );
		RefreshTeleports(true);
		m_Form.RefreshStats(true);

		//! Outright, not through SyncTeleportHistory: nothing about the panel
		//! has to have changed for the ages on its rows to be stale.
		RebuildTeleportHistory();
	}

	void Change_PositionX( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		m_Form.UpdateLastChangeTime();

		m_PositionXUpdated = true;
	}

	void Change_PositionY( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		m_Form.UpdateLastChangeTime();

		m_PositionYUpdated = true;
	}

	void Change_PositionZ( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		m_Form.UpdateLastChangeTime();

		m_PositionZUpdated = true;
	}

	void RefreshTeleports(bool force = false)
	{
		if ( !m_PositionX )
			return;

		if ( !m_Form.m_SelectedInstance )
			return;

		if ( IsMissionOffline() )
			m_Form.m_SelectedInstance.Update();

		vector position = m_Form.m_SelectedInstance.GetPosition();
		int decimals = COORD_DECIMALS_SYNCED;

		//! Null unless the entity is loaded on this client - the server only
		//! sends a reference, and a player outside our network bubble has none.
		//! When it is there it is both fresher and finer than the roster
		//! snapshot, which is rounded down to what the sync is worth.
		PlayerBase loaded = m_Form.m_SelectedInstance.PlayerObject;
		bool isLive = loaded != NULL;

		if ( isLive )
		{
			position = loaded.GetPosition();
			decimals = COORD_DECIMALS_LIVE;
		}

		if ( isLive != m_PositionIsLive )
		{
			m_PositionIsLive = isLive;
			UpdatePositionSourceHint();
		}

		if (force)
		{
			m_PositionXUpdated = false;
			m_PositionYUpdated = false;
			m_PositionZUpdated = false;
		}

		if ( m_PositionX && !m_PositionXUpdated )
			m_PositionX.SetText( FormatCoordinate( position[0], decimals ) );

		if ( m_PositionY && !m_PositionYUpdated )
			m_PositionY.SetText( FormatCoordinate( position[1], decimals ) );

		if ( m_PositionZ && !m_PositionZUpdated )
			m_PositionZ.SetText( FormatCoordinate( position[2], decimals ) );
	}

	//! Says where the three numbers came from: "11969.70" and "11969.703" are
	//! otherwise the same reading with a stray digit, and only one of them is
	//! accurate enough to teleport something back onto a roof.
	void UpdatePositionSourceHint()
	{
		if ( !m_PositionX )
			return;

		string hint = "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_SOURCE_SYNCED";

		if ( m_PositionIsLive )
			hint = "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_SOURCE_LIVE";

		if ( m_PositionX )
			m_PositionX.SetTooltip( hint );

		if ( m_PositionY )
			m_PositionY.SetTooltip( hint );

		if ( m_PositionZ )
			m_PositionZ.SetTooltip( hint );
	}

	//! float.ToString() prints about six significant digits, which on a
	//! five-digit map coordinate is a whole metre of slop - 11969.7 is every
	//! position between 11969.65 and 11969.75. Print the fraction from an
	//! integer instead so the digits that survived the read are all shown.
	string FormatCoordinate( float value, int decimals )
	{
		int scale = 1;
		for ( int i = 0; i < decimals; i++ )
			scale *= 10;

		bool negative = value < 0;
		if ( negative )
			value = -value;

		int whole = Math.Floor( value );
		int fraction = Math.Round( ( value - whole ) * scale );

		// Rounding the fraction up to the scale is a carry into the whole part.
		if ( fraction >= scale )
		{
			whole++;
			fraction -= scale;
		}

		string text = fraction.ToString();
		while ( text.Length() < decimals )
			text = "0" + text;

		text = whole.ToString() + "." + text;

		if ( negative )
			text = "-" + text;

		return text;
	}
}
