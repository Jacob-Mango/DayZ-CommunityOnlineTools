// =============================================================================
//  JMRoleManagerForm.c
//
//  Role management + player role assignment view.
//  Archetype A: Split-Pane (Left Roster + Right Detail Editor).
//
//  Unlike the lazy-tab-container forms elsewhere in COT, this one has no
//  per-mode content panel: Roles and Players share the same two containers
//  (m_ListWrapper / m_EditorWrapper below) and SetMode() tears down and
//  rebuilds whichever is active. JMRoleManagerFormTabRoles.c and
//  JMRoleManagerFormTabPlayers.c each own one mode's rebuild logic and
//  state; this form owns the containers, the mode switch, and the
//  JMConfirmation-bound callbacks (OnCreateRole_Confirm / OnDeleteRole_Confirm
//  / ConfirmRenameRole / ConfirmRoleNameRestriction) - those can't move to
//  either tab class because JMConfirmation dispatches its named callbacks
//  against whatever object its window was Init()'d with, not against
//  whichever object raised the popup.
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

	// -------------------------------------------------------------------------
	//  Widgets
	// -------------------------------------------------------------------------

	//! Shared containers both mode classes rebuild into. Public: neither tab
	//! class inherits this form, so they need more than protected to reach
	//! these through their back-reference.
	UIActionScroller            m_LeftScroller;
	Widget                      m_LeftContent;

	UIActionScroller            m_RightScroller;
	Widget                      m_RightScrollContent;

	// Left panel geometry - tab strip over the roster block
	protected Widget                      m_LeftTabStrip;
	protected Widget                      m_LeftBody;
	protected Widget                      m_LeftToolbar;
	protected Widget                      m_LeftList;

	// Mode tabs - Roles / Players
	protected UIActionTabs                m_Tabs;

	// Left panel - dynamic list area (roles or players)
	Widget                      m_ListWrapper;

	// Right panel - editor
	Widget                      m_EditorWrapper;

	//! One class per mode, in its own file - built once at init (this form
	//! has no lazy content container to hang a Build() on - see file header),
	//! then dispatched to from SetMode()/PopulateRoleList()/OpenPlayer().
	ref JMRoleManagerFormTabRoles   m_TabRoles;
	ref JMRoleManagerFormTabPlayers m_TabPlayers;

	//! A role list arrives from the server every time the tab is reopened, not
	//! just when the admin clicks Refresh - JMRoleManagerModule.RequestRoleList()
	//! is called from OnShow() and from the Player Manager deep link too. That
	//! response must not blow away an editor the admin is mid-edit in, so
	//! PopulateRoleList() only forces a full editor rebuild when this is set -
	//! i.e. right before a call that came from an explicit Refresh click.
	bool                        m_ForceEditorRefresh = false;

	//! protected, not private: sub-mods reach for the module through the form.
	//! Also public enough (no modifier) for the two mode classes to reach it
	//! through their back-reference.
	JMRoleManagerModule         m_Module;

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
		m_TabRoles   = new JMRoleManagerFormTabRoles( this );
		m_TabPlayers = new JMRoleManagerFormTabPlayers( this );

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

		m_TabPlayers.BuildToolbar( m_LeftToolbar );

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
		bool hasSelection = m_TabRoles && m_TabRoles.HasSelection();

		if ( m_PlayerMode )
			hasSelection = m_TabPlayers && m_TabPlayers.HasSelection();

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

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
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

		ApplyModeToolbars();

		if ( m_PlayerMode )
		{
			m_TabRoles.SelectNone();
			m_TabPlayers.RebuildPlayerList();
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_PLAYER_PROMPT" );
		}
		else
		{
			m_TabPlayers.SelectNone();
			m_TabRoles.RebuildRoleList();
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
		}
	}

	// -------------------------------------------------------------------------
	//  PopulateRoleList - called by the module on RPC receive.
	// -------------------------------------------------------------------------

	void PopulateRoleList( array<ref JMRoleData> roles )
	{
		bool forceEditor = m_ForceEditorRefresh;
		m_ForceEditorRefresh = false;

		if ( !m_PlayerMode )
		{
			m_TabRoles.OnRoleListArrived( roles, forceEditor );
			return;
		}

		m_TabPlayers.OnRoleListArrived( forceEditor );
	}

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

		m_PlayerMode = true;
		m_TabRoles.SelectNone();

		ApplyModeToolbars();

		m_TabPlayers.OpenPlayer( guid );

		UpdateUI();

		//! The role checkboxes are built from the role list, which arrives from
		//! the server. On a first open it is still empty here - PopulateRoleList
		//! rebuilds the editor when it lands.
		if ( m_Module )
			m_Module.RequestRoleList();
	}

	// =========================================================================
	//  Shared editor container - both mode classes tear it down and rebuild
	//  into it; only the container lifecycle is shared, not what goes in it.
	// =========================================================================

	Widget ClearEditorContainer()
	{
		if ( m_EditorWrapper )
			delete m_EditorWrapper;

		m_EditorWrapper = UIActionManager.CreateGridSpacer( m_RightScrollContent, 1, 1 );

		return m_EditorWrapper;
	}

	private void RebuildEditorEmpty( string msg )
	{
		ClearEditorContainer();

		//! The editor now shows neither role nor player - clear both "already
		//! built for X" markers so a later PopulateRoleList() does not mistake
		//! this placeholder for a live editor and skip rebuilding it once a
		//! selection is made again.
		m_TabRoles.ClearBuiltForMarker();
		m_TabPlayers.ClearBuiltForMarker();

		UIActionCard emptyCard = UIActionManager.CreateCard( m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECTION_REQUIRED_TITLE" );
		UIActionManager.CreateText( emptyCard.GetContent(), msg );
		m_RightScroller.UpdateScroller();

		UpdateUI();
	}

	// =========================================================================
	//  JMConfirmation callbacks - stay here, see file header.
	// =========================================================================

	void OnCreateRole_Confirm( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		name.Trim();

		if ( name == "" || !m_Module )
			return;

		m_Module.CreateRole( name );
	}

	void OnDeleteRole_Confirm( JMConfirmation confirmation )
	{
		string pendingRole = m_TabRoles.TakePendingDeleteRole();

		if ( pendingRole == "" || !m_Module )
			return;

		if ( m_TabRoles.IsSelectedRole( pendingRole ) )
		{
			m_TabRoles.SelectNone();
			RebuildEditorEmpty( "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
		}

		m_Module.DeleteRole( pendingRole );
	}

	void ConfirmRenameRole( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation )
			return;

		string newName = confirmation.GetEditBoxValue();
		newName.Trim();

		m_TabRoles.ApplyRename( newName );
	}

	void ConfirmRoleNameRestriction( JMConfirmation confirmation = NULL )
	{
		if ( !confirmation )
			return;

		string newRest = confirmation.GetEditBoxValue();
		newRest.Trim();

		m_TabPlayers.ApplyNameRestriction( newRest );
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
