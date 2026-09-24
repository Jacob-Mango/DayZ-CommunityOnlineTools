//! "Players" mode of JMRoleManagerForm - the online-player roster and the
//! per-player role/name-restriction editor. Back-reference to the owning
//! form, same shape as JMPlayerRowWidget.Menu. See
//! JMRoleManagerFormTabRoles.c's header for why RoleManager splits by mode
//! rather than by lazy tab container.
//!
//! ConfirmRoleNameRestriction stays on the FORM, not here - JMConfirmation
//! dispatches its named callbacks against whatever object its window was
//! Init()'d with, not against whichever object raised the popup.
class JMRoleManagerFormTabPlayers: JMFormTab
{
	protected JMRoleManagerForm m_Form;

	//! Identity row fractions - the same layout the player manager's own
	//! identity card uses: label on the left, value/copy button on the right.
	static const float IDENTITY_LABEL_WIDTH = 0.22;
	static const float IDENTITY_VALUE_WIDTH = 0.76;
	protected Widget                      m_PlayersToolbar;
	protected UIActionSearchBox           m_PlayerSearchBar;
	protected Widget                      m_PlayerRows;
	protected string                      m_SelectedGUID;
	protected string                      m_SelectedPlayerName;
	protected string                      m_PlayerSearchFilter;
	protected UIActionImageButton         m_SaveRolesBtn;
	protected ref map< string, UIActionToggleSwitch > m_PlayerRoleChecks = new map< string, UIActionToggleSwitch >();
	protected string                      m_PlayerSelectedRole;

	//! GUID the player-role editor was built for - same reasoning as
	//! JMRoleManagerFormTabRoles.m_TempRoleBuiltFor.
	protected string                      m_PlayerEditorBuiltFor = "";
	protected ref map< string, string >   m_PlayerNameRestrictions = new map< string, string >();
	protected string                      m_EditingRestrictionRole;

	void JMRoleManagerFormTabPlayers( JMRoleManagerForm form )
	{
		m_Form = form;
	}

	bool HasSelection()
	{
		return m_SelectedGUID != "";
	}

	void ClearBuiltForMarker()
	{
		m_PlayerEditorBuiltFor = "";
	}

	//! Built once at form init (this mode has no lazy content container - see
	//! JMRoleManagerFormTabRoles.c's header), then shown/hidden by
	//! JMRoleManagerForm.ApplyModeToolbars().
	void BuildToolbar( Widget toolbarHost )
	{
		if ( !toolbarHost )
			return;

		m_PlayersToolbar = UIActionManager.CreateWrapSpacer( toolbarHost, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionImageButton btnRefreshPlayers = UIActionManager.CreateRefreshButton( m_PlayersToolbar, this, "" );
		if ( btnRefreshPlayers ) btnRefreshPlayers.SetOnClick( this, "OnClick_RefreshPlayers" );

		//! No count readout. The roster below is the count, and a number
		//! beside the box only took width the search wanted.
		m_PlayerSearchBar = UIActionManager.CreateSearchBox( m_PlayersToolbar, this, "OnChange_PlayerSearch", "#STR_COT_ROLEMANAGER_MODULE_SEARCH_PLAYERS_PLACEHOLDER" );
		m_PlayerSearchBar.SetWidth( 0.85 );
	}

	//! Deep link from another form - the Player Manager's identity card sends
	//! the admin here to change the role they are looking at. Called by the
	//! form's OpenPlayer(), which has already switched mode.
	void OpenPlayer( string guid )
	{
		m_PlayerSearchFilter = "";

		if ( m_PlayerSearchBar )
			m_PlayerSearchBar.SetText( "" );

		RebuildPlayerList();

		m_SelectedGUID = guid;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( guid );
		if ( pi )
			m_SelectedPlayerName = pi.GetName();
		else
			m_SelectedPlayerName = guid;

		RebuildPlayerEditor();
	}

	void OnRoleListArrived( bool forceEditor )
	{
		//! Same reasoning as the Roles side: m_PlayerSelectedRole /
		//! m_PlayerNameRestrictions hold an unsaved assignment. Rebuild only
		//! when the editor isn't already showing this player, or on an
		//! explicit Refresh.
		if ( m_SelectedGUID != "" && ( m_PlayerEditorBuiltFor != m_SelectedGUID || forceEditor ) )
			RebuildPlayerEditor();
	}

	void RebuildPlayerList()
	{
		if ( m_Form.m_ListWrapper )
			m_Form.m_ListWrapper.Unlink();

		m_Form.m_ListWrapper = UIActionManager.CreateGridSpacer( m_Form.m_LeftContent, 1, 1 );
		m_PlayerRows  = NULL;

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();

		m_PlayerRows = UIActionManager.CreateGridSpacer( m_Form.m_ListWrapper, 1, 1 );

		FilterPlayerRows( players );

		m_Form.m_LeftScroller.UpdateScroller();
	}

	protected void FilterPlayerRows( array<JMPlayerInstance> players = NULL )
	{
		if ( !m_PlayerRows )
			return;

		if ( players == NULL )
			players = GetPermissionsManager().GetPlayers();

		UIActionManager.ClearChildren( m_PlayerRows );

		if ( players.Count() == 0 )
		{
			UIActionManager.CreateText( m_PlayerRows, "#STR_COT_ROLEMANAGER_MODULE_NO_PLAYERS_ONLINE" );
			m_Form.m_LeftScroller.UpdateScroller();
			return;
		}

		JMSearchMatcher matcher = new JMSearchMatcher( m_PlayerSearchFilter );

		// The player currently open for editing is pinned to the top of its
		// own list, the same way a checked row in the player manager floats
		// above the rest - scrolling to find it again after every role tweak
		// would defeat the point of keeping the editor open beside the list.
		if ( m_SelectedGUID != "" )
		{
			for ( int i = 0; i < players.Count(); i++ )
			{
				if ( players[i].GetGUID() != m_SelectedGUID )
					continue;

				JMPlayerInstance selected = players[i];
				players.RemoveOrdered( i );
				players.InsertAt( selected, 0 );
				break;
			}
		}

		bool anyShown = false;
		foreach ( JMPlayerInstance pi : players )
		{
			if ( !matcher.Matches( pi.GetName() ) )
				continue;

			anyShown = true;

			UIActionButton btn = UIActionManager.CreateButton( m_PlayerRows, pi.GetName(), this, "" );
			if ( btn ) btn.SetOnClick( this, "OnClick_SelectPlayer" );
			btn.SetData( new JMStringData( pi.GetGUID() ) );

			// Blue/purple accent, matching every other "this is the selected
			// one" mark in COT - orange here read as a warning, not a selection.
			if ( pi.GetGUID() == m_SelectedGUID )
				btn.SetColor( JMTheme.ACCENT );

			UIActionManager.CreateRowDivider( m_PlayerRows );
		}

		if ( !anyShown )
			UIActionManager.CreateText( m_PlayerRows, "#STR_COT_ROLEMANAGER_MODULE_NO_PLAYERS_MATCH_SEARCH" );

		if ( m_PlayerSearchBar )
			m_PlayerSearchBar.SetTextPreview( matcher.GetClosestMatch() );

		m_Form.m_LeftScroller.UpdateScroller();
	}

	void OnClick_RefreshPlayers( UIActionBase action )
	{
		m_PlayerSearchFilter = "";
		RebuildPlayerList();
	}

	void OnChange_PlayerSearch( UIEvent eid, UIActionBase action )
	{
		UIActionSearchBox searchBar;
		if ( !Class.CastTo( searchBar, action ) )
			return;

		m_PlayerSearchFilter = searchBar.GetText();
		FilterPlayerRows();
	}

	void OnClick_SelectPlayer( UIActionBase action )
	{
		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_SelectedGUID = data.Value;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( pi )
			m_SelectedPlayerName = pi.GetName();
		else
			m_SelectedPlayerName = m_SelectedGUID;

		RebuildPlayerEditor();
		m_Form.UpdateUI();
	}

	void RebuildPlayerEditor()
	{
		m_PlayerRoleChecks.Clear();
		m_PlayerSelectedRole = "";
		m_PlayerEditorBuiltFor = "";
		ClearEditor();

		if ( m_SelectedGUID == "" )
		{
			UIActionManager.CreateText( m_Form.m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECT_PLAYER_PROMPT" );
			m_Form.m_RightScroller.UpdateScroller();
			return;
		}

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( !pi )
		{
			UIActionManager.CreateText( m_Form.m_EditorWrapper, COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_PLAYER_NOT_FOUND" , m_SelectedPlayerName ) );
			m_Form.m_RightScroller.UpdateScroller();
			return;
		}

		// ---- Card 1: Player Credentials ----
		UIActionCard idCard = UIActionManager.CreateCard( m_Form.m_EditorWrapper, COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_PLAYER_IDENTITY_TITLE", m_SelectedPlayerName ) );
		Widget idBody = idCard.GetContent();

		// Label on the left, value + copy icon on the right - the same identity
		// row the player manager uses, rather than one button carrying both the
		// label and the value baked into its own text.
		Widget rowGuid = UIActionManager.CreateRow( idBody, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowGuid, "#STR_COT_ROLEMANAGER_MODULE_GUID_LABEL", "" );
		UIActionFeedbackButton btnGuid = UIActionManager.CreateFeedbackButton( rowGuid, pi.GetGUID(), "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "OnClick_CopyGUID" );
		btnGuid.SetWidth( IDENTITY_VALUE_WIDTH );
		btnGuid.SetPosition( IDENTITY_LABEL_WIDTH );
		btnGuid.SetIcon( JMConstants.ICON_STACK );
		btnGuid.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_COPY_GUID_TOOLTIP" );

		Widget rowSteam = UIActionManager.CreateRow( idBody, JMFormBase.IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowSteam, "#STR_COT_ROLEMANAGER_MODULE_STEAM64_LABEL", "" );
		UIActionFeedbackButton btnSteam = UIActionManager.CreateFeedbackButton( rowSteam, pi.GetSteam64ID(), "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "OnClick_CopySteam" );
		btnSteam.SetWidth( IDENTITY_VALUE_WIDTH );
		btnSteam.SetPosition( IDENTITY_LABEL_WIDTH );
		btnSteam.SetIcon( JMConstants.ICON_STACK );
		btnSteam.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_COPY_STEAM64_TOOLTIP" );

		// ---- Card 2: Role Assignment ----
		UIActionCard roleCard = UIActionManager.CreateCard( m_Form.m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_ASSIGNED_ROLE_TITLE" );
		m_SaveRolesBtn = roleCard.AddSaveButton( this, "", "#STR_COT_ROLEMANAGER_MODULE_SAVE_ROLE_ASSIGNMENT_TOOLTIP" );
		if ( m_SaveRolesBtn ) m_SaveRolesBtn.SetOnClick( this, "OnClick_SavePlayerRole" );
		m_Form.BindPermission( m_SaveRolesBtn, JMConstants.PERM_ROLES_PERMISSIONS );

		Widget roleBody = roleCard.GetContent();

		array<string> currentRoles = pi.GetRoles();
		foreach ( string rn : currentRoles )
		{
			if ( rn != "everyone" )
			{
				m_PlayerSelectedRole = rn;
				break;
			}
		}

		array<JMRole> allRoles = new array<JMRole>();
		GetPermissionsManager().GetRolesAsList( allRoles );

		foreach ( JMRole role : allRoles )
		{
			if ( role.Name == "everyone" )
				continue;

			if ( !m_PlayerNameRestrictions.Contains( role.Name ) )
			{
				string existingRestriction = pi.GetRoleNameRestriction( role.Name );
				if ( existingRestriction != "" )
					m_PlayerNameRestrictions.Set( role.Name, existingRestriction );
			}

			bool isSelected = ( role.Name == m_PlayerSelectedRole );

			Widget roleRow = UIActionManager.CreateWrapSpacer( roleBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			// 1. Edit-name button FIRST (left).
			string currentRest = "";
			if ( m_PlayerNameRestrictions.Contains( role.Name ) )
				currentRest = m_PlayerNameRestrictions.Get( role.Name );

			string noteTip = COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_SET_NAME_RESTRICTION_TOOLTIP" , role.Name );
			if ( currentRest != "" )
				noteTip = COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_LABEL", currentRest );

			UIActionImageButton noteBtn = UIActionManager.CreateIconButton( roleRow, JMConstants.ICON_FOLDED_PAPER, this, "" );
			if ( noteBtn ) noteBtn.SetOnClick( this, "OnClick_EditRoleNameRestriction" );
			noteBtn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );
			noteBtn.SetData( new JMStringData( role.Name ) );
			noteBtn.SetTooltip( noteTip );

			if ( currentRest != "" )
				noteBtn.SetColor( JMTheme.WARNING );

			// 2. Toggle switch SECOND - assigning a role reads as an on/off
			// switch, not a checkbox, even though only one can be on at a time.
			// No label baked in; the name is its own widget THIRD so the row
			// reads left to right as edit / switch / name.
			UIActionToggleSwitch cb = UIActionManager.CreateToggleSwitch( roleRow, "", this, "OnClick_SelectRole_Radio", isSelected );
			cb.SetWidth( 0.18 );
			cb.SetData( new JMStringData( role.Name ) );
			m_PlayerRoleChecks.Insert( role.Name, cb );

			// 3. Role name THIRD (right).
			UIActionText nameText = UIActionManager.CreateText( roleRow, role.Name );
			nameText.SetWidth( 0.6 );
		}

		m_PlayerEditorBuiltFor = m_SelectedGUID;

		m_Form.m_RightScroller.UpdateScroller();
	}

	protected void ClearEditor()
	{
		m_Form.UnregisterPermission( m_SaveRolesBtn );
		m_SaveRolesBtn = NULL;

		m_Form.ClearEditorContainer();
	}

	protected void MarkPlayerRoleChanged()
	{
		if ( m_SaveRolesBtn )
			m_SaveRolesBtn.AnimatePulse( 5.0 );
	}

	void OnClick_CopyGUID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || m_SelectedGUID == "" )
			return;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( pi )
			g_Game.CopyToClipboard( pi.GetGUID() );
		else
			g_Game.CopyToClipboard( m_SelectedGUID );
	}

	void OnClick_CopySteam( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || m_SelectedGUID == "" )
			return;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( pi )
			g_Game.CopyToClipboard( pi.GetSteam64ID() );
	}

	void OnClick_EditRoleNameRestriction( UIActionBase action )
	{
		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_EditingRestrictionRole = data.Value;

		string currentRest = "";
		if ( m_PlayerNameRestrictions.Contains( m_EditingRestrictionRole ) )
			currentRest = m_PlayerNameRestrictions.Get( m_EditingRestrictionRole );

		string restrictionTitle = COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_TITLE", m_EditingRestrictionRole );
		string restrictionBody  = COT_String.TranslateEx( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_BODY" , m_EditingRestrictionRole );

		m_Form.PromptInput( restrictionTitle, restrictionBody, "ConfirmRoleNameRestriction" );
	}

	//! Called by the form's ConfirmRoleNameRestriction (JMConfirmation callback).
	void ApplyNameRestriction( string newRest )
	{
		if ( m_EditingRestrictionRole == "" )
			return;

		if ( newRest != "" )
			m_PlayerNameRestrictions.Set( m_EditingRestrictionRole, newRest );
		else if ( m_PlayerNameRestrictions.Contains( m_EditingRestrictionRole ) )
			m_PlayerNameRestrictions.Remove( m_EditingRestrictionRole );

		MarkPlayerRoleChanged();
		RebuildPlayerEditor();
	}

	void OnClick_SelectRole_Radio( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionToggleSwitch cb;
		if ( !Class.CastTo( cb, action ) )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		if ( cb.IsChecked() )
		{
			m_PlayerSelectedRole = data.Value;

			for ( int i = 0; i < m_PlayerRoleChecks.Count(); i++ )
			{
				string roleName = m_PlayerRoleChecks.GetKey( i );
				if ( roleName != data.Value )
					m_PlayerRoleChecks.GetElement( i ).SetChecked( false );
			}
		}
		else
		{
			m_PlayerSelectedRole = "";
		}

		MarkPlayerRoleChanged();
	}

	void OnClick_SavePlayerRole( UIActionBase action )
	{
		if ( m_SelectedGUID == "" )
			return;

		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		if ( !playerModule )
			return;

		array<string> roles = new array<string>();
		roles.Insert( "everyone" );
		if ( m_PlayerSelectedRole != "" )
			roles.Insert( m_PlayerSelectedRole );

		map<string, string> nameRestrictions = new map<string, string>();
		int nameRestCount = m_PlayerNameRestrictions.Count();
		for ( int i = 0; i < nameRestCount; i++ )
		{
			string rName = m_PlayerNameRestrictions.GetKey( i );
			string rVal  = m_PlayerNameRestrictions.GetElement( i );
			if ( rVal != "" )
				nameRestrictions.Insert( rName, rVal );
		}

		playerModule.SetRoles( roles, { m_SelectedGUID }, nameRestrictions );

		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.StopPulse();

		action.AnimateSpin( 1.0 );

		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_SAVED_ROLE_NOTIFICATION", m_SelectedPlayerName ) );
	}

	//! Leaving this mode drops its selection - the other mode starts with none.
	override void OnUnfocus()
	{
		SelectNone();
	}

	void SelectNone()
	{
		m_SelectedGUID = "";
	}
}
