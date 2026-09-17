// =============================================================================
//  JMRoleManagerForm.c
//
//  Role management + player role assignment view.
//  Archetype A: Split-Pane (Left Roster + Right Detail Editor).
// =============================================================================

class JMRoleManagerForm : JMFormBase
{
	// -------------------------------------------------------------------------
	//  Mode state
	// -------------------------------------------------------------------------

	protected bool                        m_PlayerMode = false; // false = Roles, true = Players

	static const int TAB_ROLES            = 0;
	static const int TAB_PLAYERS          = 1;

	//! Height of the roster toolbar block pinned above the list.
	static const int LEFT_TOOLBAR_HEIGHT  = 75;

	//! Identity row fractions - the same layout the player manager's own
	//! identity card uses: label on the left, value/copy button on the right.
	static const float IDENTITY_LABEL_WIDTH = 0.22;
	static const float IDENTITY_VALUE_WIDTH = 0.76;

	// -------------------------------------------------------------------------
	//  Widgets
	// -------------------------------------------------------------------------

	protected UIActionScroller            m_LeftScroller;
	protected Widget                      m_LeftContent;

	protected UIActionScroller            m_RightScroller;
	protected Widget                      m_RightScrollContent;

	// Left panel geometry - tab strip over the roster block
	protected Widget                      m_LeftTabStrip;
	protected Widget                      m_LeftBody;
	protected Widget                      m_LeftToolbar;
	protected Widget                      m_LeftList;

	// Mode tabs - Roles / Players
	protected UIActionTabs                m_Tabs;

	// Per-mode toolbars
	protected Widget                      m_RolesToolbar;
	protected Widget                      m_PlayersToolbar;
	protected UIActionButton              m_CreateRoleBtn;
	protected UIActionSearchBox           m_PlayerSearchBar;

	// Left panel - dynamic list area (roles or players)
	protected Widget                      m_ListWrapper;
	protected Widget                      m_PlayerRows;

	// Right panel - editor
	protected Widget                      m_EditorWrapper;
	protected UIActionImageButton         m_SavePermBtn;
	protected UIActionImageButton         m_SaveRolesBtn;

	// ---- Roles mode state --------------------------------------------------
	protected string                      m_SelectedRole;
	protected autoptr array<ref JMRoleData> m_RoleList = new array<ref JMRoleData>();
	protected ref JMRole                  m_TempRole;
	//! Role name m_TempRole was built for. Compared against m_SelectedRole
	//! rather than just checking "is m_TempRole set" so a rename - which moves
	//! m_SelectedRole onto a new name while the editor is still open - is
	//! detected as a change and redrawn, instead of being mistaken for "already
	//! showing this role, leave it alone".
	protected string                      m_TempRoleBuiltFor = "";
	protected string                      m_PendingDeleteRole;
	protected string                      m_EditingRenameRole;

	//! A role list arrives from the server every time the tab is reopened, not
	//! just when the admin clicks Refresh - JMRoleManagerModule.RequestRoleList()
	//! is called from OnShow() and from the Player Manager deep link too. That
	//! response must not blow away an editor the admin is mid-edit in, so
	//! PopulateRoleList() only forces a full editor rebuild when this is set -
	//! i.e. right before a call that came from an explicit Refresh click.
	protected bool                        m_ForceEditorRefresh = false;

	// ---- Permission tree collapse state ------------------------------------
	//! Rows at a depth >= this start folded. 0 folds every level, so the tree
	//! opens showing only the top-level categories instead of dumping every
	//! permission the mod owns on screen. An entry in m_Collapsed always wins
	//! over this default.
	static const int                      PERM_TREE_OPEN_DEPTH = 0;
	protected ref map< string, bool >     m_Collapsed = new map< string, bool >();

	// ---- Permission tree search ---------------------------------------------
	protected UIActionSearchBox           m_PermSearchBar;
	protected string                      m_PermSearchFilter = "";
	//! Parent widget the tree wrapper is rebuilt into on every filter change.
	protected Widget                      m_PermTreeHost;
	protected Widget                      m_PermTreeWrapper;

	//! The tree as it currently stands on screen - top-level nodes only, each
	//! holding its own children. Built alongside the widgets so a toggle can
	//! reach its whole subtree without walking the widget hierarchy back.
	protected ref array< ref JMPermTreeNode > m_PermRoots = new array< ref JMPermTreeNode >();

	// ---- Players mode state ------------------------------------------------
	protected string                      m_SelectedGUID;
	protected string                      m_SelectedPlayerName;
	protected string                      m_PlayerSearchFilter;

	protected ref map< string, UIActionToggleSwitch > m_PlayerRoleChecks = new map< string, UIActionToggleSwitch >();
	protected string                      m_PlayerSelectedRole;

	//! GUID the player-role editor was built for - same reasoning as
	//! m_TempRoleBuiltFor above, on the Players side of the tab.
	protected string                      m_PlayerEditorBuiltFor = "";

	protected ref map< string, string >   m_PlayerNameRestrictions = new map< string, string >();
	protected string                      m_EditingRestrictionRole;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMRoleManagerModule         m_Module;

	// -------------------------------------------------------------------------
	//  SetModule
	// -------------------------------------------------------------------------

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// -------------------------------------------------------------------------
	//  OnInit
	// -------------------------------------------------------------------------

	override void OnInit()
	{
		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );

		m_LeftTabStrip      = layoutRoot.FindAnyWidget( "panel_left_tabs" );
		m_LeftBody          = layoutRoot.FindAnyWidget( "panel_left_body" );
		m_LeftToolbar       = layoutRoot.FindAnyWidget( "panel_left_top" );
		m_LeftList          = layoutRoot.FindAnyWidget( "panel_left_bottom" );

		InitWidgetsLeft();
		InitWidgetsRight();

		UpdateUI();
	}

	protected void InitWidgetsLeft()
	{
		//! The tabs own the mode; both panels are rebuilt on CHANGE, so no
		//! content panels are registered with UIActionTabs here.
		if ( m_LeftTabStrip )
		{
			ref array<string> tabLabels = { "#STR_COT_ROLEMANAGER_MODULE_TAB_ROLES", "#STR_COT_ROLEMANAGER_MODULE_TAB_PLAYERS" };
			ref array<string> tabIcons  = { JMConstants.Lucide( "shield" ), JMConstants.Lucide( "users" ) };

			m_Tabs = UIActionManager.CreateTabs( m_LeftTabStrip, tabLabels, tabIcons, this, "OnChange_Tab" );

			if ( m_Tabs )
				m_Tabs.SetSelection( TAB_ROLES, false );
		}

		Widget toolbarHost = m_LeftToolbar;
		if ( toolbarHost )
		{
			// ---- Players-mode toolbar -----------------------------------------
			m_PlayersToolbar = UIActionManager.CreateWrapSpacer( toolbarHost, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			UIActionImageButton btnRefreshPlayers = UIActionManager.CreateRefreshButton( m_PlayersToolbar, this, "OnClick_RefreshPlayers" );
			if ( btnRefreshPlayers )
				btnRefreshPlayers.SetFixedSize( 30, 30 );

			//! No count readout. The roster below is the count, and a number
			//! beside the box only took width the search wanted.
			m_PlayerSearchBar = UIActionManager.CreateSearchBox( m_PlayersToolbar, this, "OnChange_PlayerSearch", "#STR_COT_ROLEMANAGER_MODULE_SEARCH_PLAYERS_PLACEHOLDER" );
			m_PlayerSearchBar.SetWidth( 0.85 );
		}

		ApplyModeToolbars();

		// ---- The list itself ----------------------------------------------
		m_LeftScroller = UIActionManager.CreateScroller( m_LeftList );
		m_LeftContent  = m_LeftScroller.GetContentWidget();

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );
		UIActionManager.CreateText( m_ListWrapper, "#STR_COT_ROLEMANAGER_MODULE_LOADING" );

		m_LeftScroller.UpdateScroller();
	}

	protected void InitWidgetsRight()
	{
		Widget rightTarget = m_RightContent;
		if ( !rightTarget )
			rightTarget = m_RightPanel;

		m_RightScroller      = UIActionManager.CreateScroller( rightTarget );
		m_RightScrollContent = m_RightScroller.GetContentWidget();

		m_EditorWrapper = UIActionManager.CreateGridSpacer( m_RightScrollContent, 1, 1 );

		m_RightScroller.UpdateScroller();
	}

	protected void ApplyModeToolbars()
	{
		if ( m_LeftToolbar )
			m_LeftToolbar.Show( m_PlayerMode );

		float h = 600;
		if ( layoutRoot )
		{
			float w;
			layoutRoot.GetScreenSize( w, h );
		}

		int toolbarH = 0;
		if ( m_PlayerMode )
			toolbarH = 40;

		PinStripGeometry( m_LeftToolbar, m_LeftList, h - TAB_STRIP_HEIGHT, toolbarH );

		if ( m_LeftScroller )
			m_LeftScroller.UpdateScroller();
	}

	void UpdateUI()
	{
		bool hasSelection = ( m_SelectedRole != "" );

		if ( m_PlayerMode )
			hasSelection = ( m_SelectedGUID != "" );

		if ( hasSelection )
			ShowUI();
		else
			HideUI();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		PinRightPanelGeometry( h );
		PinStripGeometry( m_LeftTabStrip, m_LeftBody, h, TAB_STRIP_HEIGHT );

		int toolbarH = 0;
		if ( m_PlayerMode )
			toolbarH = 40;

		PinStripGeometry( m_LeftToolbar, m_LeftList, h - TAB_STRIP_HEIGHT, toolbarH );

		if ( m_LeftScroller  ) m_LeftScroller.UpdateScroller();
		if ( m_RightScroller ) m_RightScroller.UpdateScroller();
	}

	override void OnShow()
	{
		super.OnShow();

		if ( m_Module )
			m_Module.RequestRoleList();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	// -------------------------------------------------------------------------
	//  Mode helpers
	// -------------------------------------------------------------------------

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UIActionTabs tabs;
		if ( !Class.CastTo( tabs, action ) )
			return;

		SetMode( tabs.GetSelection() == TAB_PLAYERS );
	}

	protected void SetMode( bool playerMode )
	{
		if ( m_PlayerMode == playerMode )
			return;

		m_PlayerMode = playerMode;
		m_PlayerRows = NULL;

		ApplyModeToolbars();

		if ( m_PlayerMode )
		{
			m_SelectedRole       = "";
			m_PlayerSearchFilter = "";
			RebuildPlayerList();
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_PLAYER_PROMPT" );
		}
		else
		{
			m_SelectedGUID = "";
			RebuildRoleList();
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
		}
	}

	// -------------------------------------------------------------------------
	//  PopulateRoleList
	// -------------------------------------------------------------------------

	void PopulateRoleList( array<ref JMRoleData> roles )
	{
		bool forceEditor = m_ForceEditorRefresh;
		m_ForceEditorRefresh = false;

		m_RoleList.Clear();
		foreach ( JMRoleData rd : roles )
			m_RoleList.Insert( rd );

		if ( !m_PlayerMode )
		{
			if ( m_SelectedRole == "" && m_RoleList.Count() > 0 )
				m_SelectedRole = "everyone";

			//! The roster is cheap and stateless - always safe to redraw with
			//! whatever the server just sent.
			RebuildRoleList();

			//! The editor is not stateless: m_TempRole holds unsaved toggles.
			//! Only (re)build it when it is not already showing this exact
			//! role, or when this response is the direct result of an
			//! explicit Refresh click - never as a side effect of the tab
			//! merely being reopened. Comparing the role NAME (not just
			//! "is m_TempRole set") is what still catches a rename, which
			//! moves m_SelectedRole onto a new name while the editor is open.
			if ( m_SelectedRole != "" && ( m_TempRoleBuiltFor != m_SelectedRole || forceEditor ) )
				RebuildRolePermEditor();

			return;
		}

		//! Same reasoning on the Players side: m_PlayerSelectedRole /
		//! m_PlayerNameRestrictions hold an unsaved assignment. Rebuild only
		//! when the editor isn't already showing this player, or on an
		//! explicit Refresh.
		if ( m_SelectedGUID != "" && ( m_PlayerEditorBuiltFor != m_SelectedGUID || forceEditor ) )
			RebuildPlayerEditor();
	}

	// =========================================================================
	//  ROLES MODE
	// =========================================================================

	private void RebuildRoleList()
	{
		if ( m_ListWrapper )
			delete m_ListWrapper;

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );

		UIActionCard card = UIActionManager.CreateCard( m_ListWrapper, "#STR_COT_ROLEMANAGER_MODULE_CONFIGURED_ROLES" );
		card.AddRefreshButton( this, "OnClick_Refresh", "#STR_COT_ROLEMANAGER_MODULE_REFRESH_ROLES_TOOLTIP" );
		Widget cardContent = card.GetContent();

		foreach ( JMRoleData rd : m_RoleList )
		{
			UIActionButton selectBtn = UIActionManager.CreateButton( cardContent, rd.Name, this, "OnClick_SelectRole" );
			selectBtn.SetData( new JMStringData( rd.Name ) );

			UIActionManager.CreatePanel( cardContent, 0x22FFFFFF, 1 );
		}

		m_CreateRoleBtn = UIActionManager.CreateButton( cardContent, "#STR_COT_ROLEMANAGER_MODULE_NEW_ROLE_BUTTON", this, "OnClick_CreateRole" );
		m_CreateRoleBtn.SetColor( JMTheme.SUCCESS_FILL );
		m_CreateRoleBtn.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_NEW_ROLE_TOOLTIP" );

		RegisterPermission( m_CreateRoleBtn, "Admin.Roles.Create" );

		m_LeftScroller.UpdateScroller();
	}

	private void RebuildRolePermEditor()
	{
		m_TempRole = NULL;
		m_TempRoleBuiltFor = "";
		m_Collapsed.Clear();
		RebuildEditorClear();

		if ( m_SelectedRole == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
			m_RightScroller.UpdateScroller();
			return;
		}

		bool isDeletable = ( m_SelectedRole != "admin" && m_SelectedRole != "everyone" );

		// Permission Configuration Card
		UIActionCard permCard = UIActionManager.CreateCard( m_EditorWrapper, Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_PERMISSION_CONFIG_TITLE" ) + ": " + m_SelectedRole );
		m_SavePermBtn = permCard.AddSaveButton( this, "OnClick_SaveRolePermissions", Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_SAVE_PERMISSIONS_TOOLTIP" ) + ": " + m_SelectedRole );
		RegisterPermission( m_SavePermBtn, "Admin.Roles.Permissions" );

		if ( isDeletable )
		{
			UIActionImageButton renameBtn = permCard.AddCardHeaderAction( JMConstants.ICON_FOLDED_PAPER, this, "OnClick_RenameRole", Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_TOOLTIP" ) + " '" + m_SelectedRole + "'" );
			renameBtn.SetData( new JMStringData( m_SelectedRole ) );

			if ( GetPermissionsManager().HasPermission( "Admin.Roles.Delete" ) )
			{
				UIActionImageButton delBtn = permCard.AddDeleteButton( this, "OnClick_DeleteRole", "#STR_COT_ROLEMANAGER_MODULE_DELETE_ROLE_TOOLTIP" );
				delBtn.SetData( new JMStringData( m_SelectedRole ) );
			}
		}

		Widget permBody = permCard.GetContent();

		m_PermSearchBar = UIActionManager.CreateSearchBox( permBody, this, "OnChange_PermSearch", "#STR_COT_ROLEMANAGER_MODULE_SEARCH_PERMISSIONS_PLACEHOLDER", m_PermSearchFilter );

		m_PermTreeHost = permBody;

		JMRoleData roleData = GetRoleData( m_SelectedRole );
		m_TempRole = new JMRole( m_SelectedRole );
		if ( roleData )
		{
			foreach ( string line : roleData.Permissions )
				m_TempRole.AddPermission( line );
		}

		m_TempRoleBuiltFor = m_SelectedRole;

		RebuildPermTreeWidgets();

		m_RightScroller.UpdateScroller();
	}

	//! Rebuilds just the tree portion of the permission editor - used both for
	//! the initial build and for every keystroke in the search box, so the card
	//! header, save/rename/delete buttons and m_TempRole are left untouched.
	private void RebuildPermTreeWidgets()
	{
		if ( !m_TempRole || !m_PermTreeHost )
			return;

		m_PermRoots.Clear();

		if ( m_PermTreeWrapper )
			delete m_PermTreeWrapper;

		m_PermTreeWrapper = UIActionManager.CreateGridSpacer( m_PermTreeHost, 1, 1 );

		BuildPermissionTree( m_PermTreeWrapper, m_TempRole.RootPermission, NULL, 0 );
		RefreshPermTreeState();

		m_RightScroller.UpdateScroller();
	}

	void OnChange_PermSearch( UIEvent eid, UIActionBase action )
	{
		UIActionSearchBox searchBar;
		if ( !Class.CastTo( searchBar, action ) )
			return;

		m_PermSearchFilter = searchBar.GetText();
		RebuildPermTreeWidgets();
	}

	//! True if this permission's own name matches the active search filter, or
	//! anything nested under it does - a branch stays visible while filtering
	//! as long as something inside it is still a match.
	private bool PermSubtreeMatchesFilter( JMPermission perm )
	{
		if ( !perm )
			return false;

		string filterLow = m_PermSearchFilter;
		filterLow.ToLower();

		string nameLow = perm.Name;
		nameLow.ToLower();

		if ( nameLow.IndexOf( filterLow ) != -1 )
			return true;

		for ( int i = 0; i < perm.Children.Count(); i++ )
		{
			if ( PermSubtreeMatchesFilter( perm.Children[i] ) )
				return true;
		}

		return false;
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

		m_ForceEditorRefresh = true;

		if ( m_Module )
			m_Module.RequestRoleList();
	}

	void OnClick_CreateRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_ROLEMANAGER_MODULE_CREATE_ROLE_TITLE", "#STR_COT_ROLEMANAGER_MODULE_CREATE_ROLE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnCreateRole_Confirm" );
	}

	void OnCreateRole_Confirm( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		name.Trim();

		if ( name == "" || !m_Module )
			return;

		m_Module.CreateRole( name );
	}

	void OnClick_SelectRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_SelectedRole = data.Value;
		RebuildRolePermEditor();
		UpdateUI();
	}

	void OnClick_DeleteRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK && eid != UIEvent.CHANGE )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_PendingDeleteRole = data.Value;

		string delMsg = string.Format( Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_DELETE_ROLE_CONFIRM_BODY" ), m_PendingDeleteRole );

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", delMsg, "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnDeleteRole_Confirm" );
	}

	void OnDeleteRole_Confirm( JMConfirmation confirmation )
	{
		if ( m_PendingDeleteRole == "" || !m_Module )
			return;

		if ( m_SelectedRole == m_PendingDeleteRole )
		{
			m_SelectedRole = "";
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
		}

		m_Module.DeleteRole( m_PendingDeleteRole );
		m_PendingDeleteRole = "";
	}

	protected void MarkRolePermChanged()
	{
		if ( m_SavePermBtn )
			m_SavePermBtn.AnimatePulse( 5.0 );
	}

	void OnClick_RenameRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_EditingRenameRole = data.Value;

		string renameTitle = Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_TITLE" ) + ": " + m_EditingRenameRole;
		string renameBody  = string.Format( Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_BODY" ), m_EditingRenameRole );

		CreateConfirmation_Two( JMConfirmationType.EDIT, renameTitle, renameBody, "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmRenameRole" );
	}

	void ConfirmRenameRole( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation || m_EditingRenameRole == "" || !m_Module || !m_TempRole )
			return;

		string newName = confirmation.GetEditBoxValue();
		newName.Trim();

		if ( newName == "" || newName == m_EditingRenameRole )
			return;

		m_Module.CreateRole( newName );
		m_Module.SetRolePermissions( newName, m_TempRole.Serialize() );
		m_Module.DeleteRole( m_EditingRenameRole );

		m_SelectedRole = newName;
		m_EditingRenameRole = "";
	}

	void OnClick_SaveRolePermissions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_SelectedRole == "" || !m_Module || !m_TempRole )
			return;

		OptimizeInherit( m_TempRole.RootPermission );

		m_Module.SetRolePermissions( m_SelectedRole, m_TempRole.Serialize() );

		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.StopPulse();

		action.AnimateSpin( 1.0 );

		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_SAVED_PERMISSIONS_NOTIFICATION", m_SelectedRole ) );
	}

	// =========================================================================
	//  PLAYERS MODE
	// =========================================================================

	//! Deep link from another form - the Player Manager's identity card sends
	//! the admin here to change the role they are looking at.
	//!
	//! SetMode() is deliberately not used: it returns early when the form is
	//! already in Players mode, which is the common case for a second jump, and
	//! it would then leave the previous player selected.
	void OpenPlayer( string guid )
	{
		if ( guid == "" )
			return;

		if ( m_Tabs )
			m_Tabs.SetSelection( TAB_PLAYERS, false );

		m_PlayerMode         = true;
		m_PlayerRows         = NULL;
		m_SelectedRole       = "";
		m_PlayerSearchFilter = "";

		//! The filter is state the roster still reads, so clearing the string
		//! without clearing the box would hide rows for no visible reason.
		if ( m_PlayerSearchBar )
			m_PlayerSearchBar.SetText( "" );

		ApplyModeToolbars();
		RebuildPlayerList();

		m_SelectedGUID = guid;

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( guid );
		if ( pi )
			m_SelectedPlayerName = pi.GetName();
		else
			m_SelectedPlayerName = guid;

		RebuildPlayerEditor();
		UpdateUI();

		//! The role checkboxes are built from m_RoleList, which arrives from the
		//! server. On a first open it is still empty here - PopulateRoleList
		//! rebuilds the editor when it lands.
		if ( m_Module )
			m_Module.RequestRoleList();
	}

	private void RebuildPlayerList()
	{
		if ( m_ListWrapper )
			delete m_ListWrapper;

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );
		m_PlayerRows  = NULL;

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();

		m_PlayerRows = UIActionManager.CreateGridSpacer( m_ListWrapper, 1, 1 );

		FilterPlayerRows( players );

		m_LeftScroller.UpdateScroller();
	}

	private void FilterPlayerRows( array<JMPlayerInstance> players = NULL )
	{
		if ( !m_PlayerRows )
			return;

		if ( players == NULL )
			players = GetPermissionsManager().GetPlayers();

		Widget child = m_PlayerRows.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if ( players.Count() == 0 )
		{
			UIActionManager.CreateText( m_PlayerRows, "#STR_COT_ROLEMANAGER_MODULE_NO_PLAYERS_ONLINE" );
			m_LeftScroller.UpdateScroller();
			return;
		}

		COT_String strSearch = m_PlayerSearchFilter;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare( requireAllKeywords );
		string closestMatch;

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
			if ( strSearch != string.Empty )
			{
				COT_String pName = pi.GetName();
				pName.ToLower();
				if ( !pName.KeywordSearchImplEx( strSearch, keywords, requireAllKeywords, closestMatch ) )
					continue;
			}

			anyShown = true;

			UIActionButton btn = UIActionManager.CreateButton( m_PlayerRows, pi.GetName(), this, "OnClick_SelectPlayer" );
			btn.SetData( new JMStringData( pi.GetGUID() ) );

			// Blue/purple accent, matching every other "this is the selected
			// one" mark in COT - orange here read as a warning, not a selection.
			if ( pi.GetGUID() == m_SelectedGUID )
				btn.SetColor( JMTheme.ACCENT );

			UIActionManager.CreatePanel( m_PlayerRows, 0x22FFFFFF, 1 );
		}

		if ( !anyShown )
			UIActionManager.CreateText( m_PlayerRows, "#STR_COT_ROLEMANAGER_MODULE_NO_PLAYERS_MATCH_SEARCH" );

		if ( m_PlayerSearchBar )
			m_PlayerSearchBar.SetTextPreview( closestMatch );

		m_LeftScroller.UpdateScroller();
	}

	void OnClick_RefreshPlayers( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

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

	void OnClick_SelectPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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
		UpdateUI();
	}

	private void RebuildPlayerEditor()
	{
		m_PlayerRoleChecks.Clear();
		m_PlayerSelectedRole = "";
		m_PlayerEditorBuiltFor = "";
		RebuildEditorClear();

		if ( m_SelectedGUID == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECT_PLAYER_PROMPT" );
			m_RightScroller.UpdateScroller();
			return;
		}

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( !pi )
		{
			UIActionManager.CreateText( m_EditorWrapper, Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_PLAYER_NOT_FOUND" ) + ": " + m_SelectedPlayerName );
			m_RightScroller.UpdateScroller();
			return;
		}

		// ---- Card 1: Player Credentials ----
		UIActionCard idCard = UIActionManager.CreateCard( m_EditorWrapper, Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_PLAYER_IDENTITY_TITLE" ) + ": " + m_SelectedPlayerName );
		Widget idBody = idCard.GetContent();

		// Label on the left, value + copy icon on the right - the same identity
		// row the player manager uses, rather than one button carrying both the
		// label and the value baked into its own text.
		Widget rowGuid = UIActionManager.CreatePanel( idBody, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowGuid, "#STR_COT_ROLEMANAGER_MODULE_GUID_LABEL", "" );
		UIActionFeedbackButton btnGuid = UIActionManager.CreateFeedbackButton( rowGuid, pi.GetGUID(), "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "OnClick_CopyGUID" );
		btnGuid.SetWidth( IDENTITY_VALUE_WIDTH );
		btnGuid.SetPosition( IDENTITY_LABEL_WIDTH );
		btnGuid.SetIcon( JMConstants.ICON_STACK );
		btnGuid.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_COPY_GUID_TOOLTIP" );

		Widget rowSteam = UIActionManager.CreatePanel( idBody, 0x00000000, IDENTITY_ROW_HEIGHT );
		UIActionManager.CreateText( rowSteam, "#STR_COT_ROLEMANAGER_MODULE_STEAM64_LABEL", "" );
		UIActionFeedbackButton btnSteam = UIActionManager.CreateFeedbackButton( rowSteam, pi.GetSteam64ID(), "#STR_COT_COPIED", JMConstants.ICON_CHECK_MARK, this, "OnClick_CopySteam" );
		btnSteam.SetWidth( IDENTITY_VALUE_WIDTH );
		btnSteam.SetPosition( IDENTITY_LABEL_WIDTH );
		btnSteam.SetIcon( JMConstants.ICON_STACK );
		btnSteam.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_COPY_STEAM64_TOOLTIP" );

		// ---- Card 2: Role Assignment ----
		UIActionCard roleCard = UIActionManager.CreateCard( m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_ASSIGNED_ROLE_TITLE" );
		m_SaveRolesBtn = roleCard.AddSaveButton( this, "OnClick_SavePlayerRole", "#STR_COT_ROLEMANAGER_MODULE_SAVE_ROLE_ASSIGNMENT_TOOLTIP" );
		RegisterPermission( m_SaveRolesBtn, "Admin.Roles.Permissions" );

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

			string noteTip = string.Format( Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_SET_NAME_RESTRICTION_TOOLTIP" ), role.Name );
			if ( currentRest != "" )
				noteTip = Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_LABEL" ) + ": " + currentRest;

			UIActionImageButton noteBtn = UIActionManager.CreateIconButton( roleRow, JMConstants.ICON_FOLDED_PAPER, this, "OnClick_EditRoleNameRestriction" );
			noteBtn.SetFixedSize( HEADER_ACTION_PX, HEADER_ACTION_PX );
			noteBtn.SetData( new JMStringData( role.Name ) );
			noteBtn.SetTooltip( noteTip );

			if ( currentRest != "" )
				noteBtn.SetColor( JMTheme.WARNING );

			// 2. Toggle switch SECOND - assigning a role reads as an on/off
			// switch, not a checkbox, even though only one can be on at a time.
			// No label baked in; the name is its own widget THIRD so the row
			// reads left to right as edit / switch / name.
			//
			// A WrapSpacer only keeps two items on one line if both fit the
			// remaining width - the toggle and the text default to filling the
			// WHOLE row (width 1), so each one wrapped to a line of its own.
			// Explicit narrow widths are what keep edit / switch / name on one
			// line instead of three.
			UIActionToggleSwitch cb = UIActionManager.CreateToggleSwitch( roleRow, "", this, "OnClick_SelectRole_Radio", isSelected );
			cb.SetWidth( 0.18 );
			cb.SetData( new JMStringData( role.Name ) );
			m_PlayerRoleChecks.Insert( role.Name, cb );

			// 3. Role name THIRD (right).
			UIActionText nameText = UIActionManager.CreateText( roleRow, role.Name );
			nameText.SetWidth( 0.6 );
		}

		m_PlayerEditorBuiltFor = m_SelectedGUID;

		m_RightScroller.UpdateScroller();
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

	void OnClick_EditRoleNameRestriction( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_EditingRestrictionRole = data.Value;

		string currentRest = "";
		if ( m_PlayerNameRestrictions.Contains( m_EditingRestrictionRole ) )
			currentRest = m_PlayerNameRestrictions.Get( m_EditingRestrictionRole );

		string restrictionTitle = Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_TITLE" ) + ": " + m_EditingRestrictionRole;
		string restrictionBody  = string.Format( Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_NAME_RESTRICTION_BODY" ), m_EditingRestrictionRole );

		CreateConfirmation_Two( JMConfirmationType.EDIT, restrictionTitle, restrictionBody, "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ConfirmRoleNameRestriction" );
	}

	void ConfirmRoleNameRestriction( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation || m_EditingRestrictionRole == "" )
			return;

		string newRest = confirmation.GetEditBoxValue();
		newRest.Trim();

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

	void OnClick_SavePlayerRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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

	// =========================================================================
	//  Shared editor helpers
	// =========================================================================

	private void RebuildEditorClear()
	{
		UnregisterPermission( m_SavePermBtn );
		UnregisterPermission( m_SaveRolesBtn );

		m_SavePermBtn          = NULL;
		m_SaveRolesBtn         = NULL;

		// The nodes point at widgets that are about to go, so they go first.
		m_PermRoots.Clear();

		// m_PermTreeHost/m_PermTreeWrapper/m_PermSearchBar live inside
		// m_EditorWrapper - about to be deleted below, so drop the references
		// rather than leave them dangling.
		m_PermTreeHost    = NULL;
		m_PermTreeWrapper = NULL;
		m_PermSearchBar   = NULL;

		if ( m_EditorWrapper )
			delete m_EditorWrapper;

		m_EditorWrapper = UIActionManager.CreateGridSpacer( m_RightScrollContent, 1, 1 );
	}

	private void RebuildEditorEmpty( string msg )
	{
		RebuildEditorClear();

		//! The editor now shows neither role nor player - clear both "already
		//! built for X" markers so a later PopulateRoleList() (which only
		//! compares against m_SelectedRole / m_SelectedGUID) does not mistake
		//! this placeholder for a live editor and skip rebuilding it once a
		//! selection is made again.
		m_TempRoleBuiltFor     = "";
		m_PlayerEditorBuiltFor = "";

		UIActionCard emptyCard = UIActionManager.CreateCard( m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECTION_REQUIRED_TITLE" );
		UIActionManager.CreateText( emptyCard.GetContent(), msg );
		m_RightScroller.UpdateScroller();

		UpdateUI();
	}

	// =========================================================================
	//  Permission tree
	//
	//  Every branch owns a fold panel and its subtree lives INSIDE that fold,
	//  not beside it. That is what makes opening one a slide rather than a
	//  rebuild: the fold clips a single widget whose height is the whole
	//  subtree however deep it nests, and it measures that height itself, so
	//  nothing here has to know how tall a branch came out.
	//
	//  The old version rebuilt the entire tree on every click, which meant the
	//  page jumped under the cursor and there was nothing left to animate.
	// =========================================================================

	private void BuildPermissionTree( Widget parent, JMPermission perm, JMPermTreeNode parentNode, int depth )
	{
		if ( !perm )
			return;

		//! One indent step, in pixels. Applied to the label text rather than to
		//! the row, so every row keeps the same hit area and the switch column
		//! stays put no matter how deep the node is.
		static const float INDENT_UNIT = 18.0;
		//! Width of the switch column on the right of every row.
		static const float SWITCH_W    = 0.12;

		float labelW = 1.0 - SWITCH_W;

		int count = perm.Children.Count();
		for ( int i = 0; i < count; i++ )
		{
			JMPermission child = perm.Children[i];
			if ( !child )
				continue;

			if ( m_PermSearchFilter != "" && !PermSubtreeMatchesFilter( child ) )
				continue;

			bool hasChildren = child.Children.Count() > 0;
			string fullName  = child.GetFullName();

			bool isCollapsed = ( depth >= PERM_TREE_OPEN_DEPTH );
			if ( m_Collapsed.Contains( fullName ) )
				isCollapsed = m_Collapsed.Get( fullName );

			//! While a search is active, force every matching branch open so the
			//! result is actually visible instead of hiding behind a fold.
			if ( m_PermSearchFilter != "" )
				isCollapsed = false;

			JMPermTreeNode node = new JMPermTreeNode();
			node.Perm     = child;
			node.FullName = fullName;

			Widget row = UIActionManager.CreateGridSpacer( parent, 1, 2 );

			// A branch is the whole row: clicking anywhere on the name folds or
			// unfolds it. A leaf has nothing to fold, so it is plain text and
			// never lights up under the cursor.
			if ( hasChildren )
			{
				UIActionButton nodeBtn = UIActionManager.CreateButton( row, child.Name, this, "OnClick_ToggleCollapse" );
				nodeBtn.SetFlat( true );

				//! ONE glyph for both states, turned rather than swapped. The
				//! turn is the animation - a swap has no in-between to show.
				nodeBtn.SetIcon( JMConstants.ICON_CHEVRON_DOWN );
				nodeBtn.SetIconRotation( ChevronAngle( isCollapsed ), false );

				nodeBtn.SetContentIndent( depth * INDENT_UNIT );
				nodeBtn.SetWidth( labelW );
				nodeBtn.SetPosition( 0 );
				nodeBtn.SetData( node );

				node.NodeButton = nodeBtn;
			}
			else
			{
				UIActionText leaf = UIActionManager.CreateText( row, child.Name );
				leaf.SetLabelColor( JMTheme.TEXT_SECONDARY );
				// Matches the flat button's icon slot so a leaf's name lines up
				// with the name of a foldable sibling.
				leaf.SetLabelOffset( ( depth * INDENT_UNIT ) + UIActionButton.ICON_SLOT_W );
				leaf.SetWidth( labelW );
				leaf.SetPosition( 0 );

				node.LeafText = leaf;
			}

			bool isAllow = EffectiveAllow( child );

			UIActionToggleSwitch sw = UIActionManager.CreateToggleSwitch( row, "", this, "OnClick_PermToggle", isAllow );
			sw.SetWidth( SWITCH_W );
			sw.SetPosition( labelW );
			sw.SetData( node );

			node.Toggle = sw;

			if ( parentNode )
				parentNode.Children.Insert( node );
			else
				m_PermRoots.Insert( node );

			if ( hasChildren )
			{
				UIActionFoldPanel fold = UIActionManager.CreateFoldPanel( parent, this, "OnChange_PermFold", !isCollapsed );

				node.Fold = fold;

				BuildPermissionTree( fold.GetContent(), child, node, depth + 1 );
			}
		}
	}

	//! Where the chevron rests. 0 points it down over an open subtree and -90
	//! points it right at a shut one, which is the convention
	//! UIActionCollapsibleSection already uses.
	private float ChevronAngle( bool collapsed )
	{
		if ( collapsed )
			return -90;

		return 0;
	}

	void OnClick_ToggleCollapse( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPermTreeNode node;
		if ( !Class.CastTo( node, action.GetData() ) )
			return;

		if ( !node.Fold )
			return;

		bool nowCollapsed = node.Fold.IsExpanded();

		node.Fold.SetExpanded( !nowCollapsed );

		if ( node.NodeButton )
			node.NodeButton.SetIconRotation( ChevronAngle( nowCollapsed ) );

		m_Collapsed.Remove( node.FullName );
		m_Collapsed.Insert( node.FullName, nowCollapsed );
	}

	//! The fold reports every frame its height moves, and the page above it
	//! grows and shrinks with it, so the scroller has to be re-measured for the
	//! whole slide rather than once at the end.
	void OnChange_PermFold( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_RightScroller )
			m_RightScroller.UpdateScroller();
	}

	void OnClick_PermToggle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionToggleSwitch sw;
		if ( !Class.CastTo( sw, action ) )
			return;

		JMPermTreeNode node;
		if ( !Class.CastTo( node, action.GetData() ) )
			return;

		if ( !node.Perm )
			return;

		bool on = sw.IsChecked();

		if ( on )
			node.Perm.Type = JMPermissionType.ALLOW;
		else
			node.Perm.Type = JMPermissionType.DISALLOW;

		//! Turning a branch off turns its subtree off for real, not just
		//! visually. A child left ALLOW under a DISALLOW parent is a
		//! contradiction, and the save would carry it.
		//! Turning a branch on cascades the same way: enabling a permission
		//! enables everything nested under it too.
		if ( on )
			AllowSubtree( node );
		else
			DenySubtree( node );

		ApplySubtreeEnabled( node, on );

		MarkRolePermChanged();
	}

	//! Write DISALLOW through everything below a node.
	private void DenySubtree( JMPermTreeNode node )
	{
		for ( int i = 0; i < node.Children.Count(); i++ )
		{
			JMPermTreeNode child = node.Children[i];

			if ( child.Perm )
				child.Perm.Type = JMPermissionType.DISALLOW;

			DenySubtree( child );
		}
	}

	//! Write ALLOW through everything below a node.
	private void AllowSubtree( JMPermTreeNode node )
	{
		for ( int i = 0; i < node.Children.Count(); i++ )
		{
			JMPermTreeNode child = node.Children[i];

			if ( child.Perm )
				child.Perm.Type = JMPermissionType.ALLOW;

			AllowSubtree( child );
		}
	}

	//! Grey out and lock everything under a branch that is off, and hand it all
	//! back when the branch comes on again.
	//!
	//! The FOLD stays clickable either way. A locked subtree is still worth
	//! reading - being unable to grant a permission is not a reason to be
	//! unable to see that it exists.
	private void ApplySubtreeEnabled( JMPermTreeNode node, bool parentOn )
	{
		for ( int i = 0; i < node.Children.Count(); i++ )
		{
			JMPermTreeNode child = node.Children[i];

			bool childOn = ( child.Perm && EffectiveAllow( child.Perm ) );
			bool showOn  = ( parentOn && childOn );

			if ( child.Toggle )
			{
				child.Toggle.SetChecked( showOn );
				child.Toggle.SetEnabled( parentOn );
			}

			if ( child.NodeButton )
				child.NodeButton.SetTextColor( RowTextColor( parentOn, true ) );

			if ( child.LeafText )
				child.LeafText.SetLabelColor( RowTextColor( parentOn, false ) );

			ApplySubtreeEnabled( child, showOn );
		}
	}

	private int RowTextColor( bool enabled, bool isBranch )
	{
		if ( !enabled )
			return JMTheme.TEXT_DISABLED;

		if ( isBranch )
			return JMTheme.TEXT_PRIMARY;

		return JMTheme.TEXT_SECONDARY;
	}

	//! Run once after a build so a role that arrives with a branch already off
	//! shows that branch's subtree greyed straight away, rather than only after
	//! the admin touches something.
	private void RefreshPermTreeState()
	{
		for ( int i = 0; i < m_PermRoots.Count(); i++ )
		{
			JMPermTreeNode root = m_PermRoots[i];

			bool on = ( root.Perm && EffectiveAllow( root.Perm ) );

			ApplySubtreeEnabled( root, on );
		}
	}

	// =========================================================================
	//  Inherit optimisation
	// =========================================================================

	private void OptimizeInherit( JMPermission perm )
	{
		if ( !perm )
			return;

		for ( int i = 0; i < perm.Children.Count(); i++ )
		{
			JMPermission child = perm.Children[i];
			if ( !child )
				continue;

			bool parentAllows = ResolveParentAllows( child );

			if ( child.Type == JMPermissionType.ALLOW && parentAllows )
				child.Type = JMPermissionType.INHERIT;
			else if ( child.Type == JMPermissionType.DISALLOW && !parentAllows )
				child.Type = JMPermissionType.INHERIT;

			OptimizeInherit( child );
		}
	}

	//! What a node actually resolves to, matching JMPermission.Check(): an
	//! explicit ALLOW/DISALLOW is authoritative, and INHERIT looks up the
	//! chain. OptimizeInherit collapses redundant ALLOW/DISALLOW down to
	//! INHERIT on save, so a plain `Type == ALLOW` check on a freshly loaded
	//! role under-reports which permissions are actually on - use this instead
	//! anywhere the UI needs to show whether a permission is effectively
	//! granted.
	private bool EffectiveAllow( JMPermission perm )
	{
		if ( !perm )
			return false;

		if ( perm.Type == JMPermissionType.ALLOW )
			return true;

		if ( perm.Type == JMPermissionType.DISALLOW )
			return false;

		return ResolveParentAllows( perm );
	}

	private bool ResolveParentAllows( JMPermission node )
	{
		JMPermission p = node.Parent;
		while ( p != NULL )
		{
			if ( p.Type == JMPermissionType.ALLOW )
				return true;
			if ( p.Type == JMPermissionType.DISALLOW )
				return false;
			p = p.Parent;
		}
		return false;
	}

	private JMRoleData GetRoleData( string name )
	{
		foreach ( JMRoleData rd : m_RoleList )
		{
			if ( rd.Name == name )
				return rd;
		}
		return NULL;
	}
}

class JMStringData : UIActionData
{
	string Value;

	void JMStringData( string val )
	{
		Value = val;
	}
}

//! One row of the permission tree: the permission it stands for, the widgets
//! drawn for it, and the rows nested under it.
//!
//! It is a UIActionData so the row's switch and its fold button can both carry
//! it, which is what lets either of them reach the whole subtree. Children are
//! owned here; the widget pointers are not - the widgets own their own scripts.
class JMPermTreeNode : UIActionData
{
	JMPermission          Perm;
	string                FullName;

	UIActionToggleSwitch  Toggle;
	//! Branch only - the flat row that folds the subtree.
	UIActionButton        NodeButton;
	//! Leaf only - a leaf has nothing to fold, so it is text, not a button.
	UIActionText          LeafText;
	//! Branch only - holds this node's subtree and animates it open and shut.
	UIActionFoldPanel     Fold;

	ref array< ref JMPermTreeNode > Children;

	void JMPermTreeNode()
	{
		Children = new array< ref JMPermTreeNode >();
	}
}
