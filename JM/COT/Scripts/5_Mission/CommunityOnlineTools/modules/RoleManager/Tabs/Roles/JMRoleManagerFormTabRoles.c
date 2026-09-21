//! "Roles" mode of JMRoleManagerForm - the role roster, the permission-tree
//! editor, and role create/rename/delete. Back-reference to the owning form,
//! same shape as JMPlayerRowWidget.Menu.
//!
//! Unlike the lazy-tab-container forms elsewhere in COT, RoleManager has no
//! per-mode content panel: Roles and Players share the same two containers
//! (m_ListWrapper/m_EditorWrapper on the form) and the form's tab focus hook tears down and
//! rebuilds whichever one is active. This class owns everything Roles-mode
//! rebuilds into them, but not the containers themselves.
//!
//! OnCreateRole_Confirm / OnDeleteRole_Confirm / ConfirmRenameRole stay on
//! the FORM, not here - JMConfirmation dispatches its named callbacks
//! against whatever object its window was Init()'d with, not against
//! whichever object raised the popup.
class JMRoleManagerFormTabRoles: JMFormTab
{
	protected JMRoleManagerForm m_Form;

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
	protected UIActionImageButton         m_SavePermBtn;

	// ---- Permission tree collapse state ------------------------------------
	static const int                      PERM_TREE_OPEN_DEPTH = 0;
	protected ref map< string, bool >     m_Collapsed = new map< string, bool >();

	// ---- Permission tree search ---------------------------------------------
	protected UIActionSearchBox           m_PermSearchBar;
	protected string                      m_PermSearchFilter = "";
	protected Widget                      m_PermTreeHost;
	protected Widget                      m_PermTreeWrapper;
	protected ref array< ref JMPermTreeNode > m_PermRoots = new array< ref JMPermTreeNode >();

	void JMRoleManagerFormTabRoles( JMRoleManagerForm form )
	{
		m_Form = form;
	}

	protected JMRoleData GetRoleData( string name )
	{
		foreach ( JMRoleData rd : m_RoleList )
		{
			if ( rd.Name == name )
				return rd;
		}
		return NULL;
	}

	bool HasSelection()
	{
		return m_SelectedRole != "";
	}

	bool IsSelectedRole( string role )
	{
		return m_SelectedRole == role;
	}

	//! Called by the form on mode entry / OnClick_SelectRole / etc.
	void ClearBuiltForMarker()
	{
		m_TempRoleBuiltFor = "";
	}

	// -------------------------------------------------------------------------
	//  PopulateRoleList hook
	// -------------------------------------------------------------------------

	void OnRoleListArrived( array<ref JMRoleData> roles, bool forceEditor )
	{
		m_RoleList.Clear();
		foreach ( JMRoleData rd : roles )
			m_RoleList.Insert( rd );

		if ( m_SelectedRole == "" && m_RoleList.Count() > 0 )
			m_SelectedRole = "everyone";

		//! The roster is cheap and stateless - always safe to redraw with
		//! whatever the server just sent.
		RebuildRoleList();

		//! The editor is not stateless: m_TempRole holds unsaved toggles. Only
		//! (re)build it when it is not already showing this exact role, or
		//! when this response is the direct result of an explicit Refresh
		//! click - never as a side effect of the tab merely being reopened.
		if ( m_SelectedRole != "" && ( m_TempRoleBuiltFor != m_SelectedRole || forceEditor ) )
			RebuildRolePermEditor();
	}

	void RebuildRoleList()
	{
		if ( m_Form.m_ListWrapper )
			delete m_Form.m_ListWrapper;

		m_Form.m_ListWrapper = UIActionManager.CreateGridSpacer( m_Form.m_LeftContent, 1, 1 );

		UIActionCard card = UIActionManager.CreateCard( m_Form.m_ListWrapper, "#STR_COT_ROLEMANAGER_MODULE_CONFIGURED_ROLES" );
		card.AddRefreshButton( this, "OnClick_Refresh", "#STR_COT_ROLEMANAGER_MODULE_REFRESH_ROLES_TOOLTIP" );
		Widget cardContent = card.GetContent();

		foreach ( JMRoleData rd : m_RoleList )
		{
			UIActionButton selectBtn = UIActionManager.CreateButton( cardContent, rd.Name, this, "OnClick_SelectRole" );
			selectBtn.SetData( new JMStringData( rd.Name ) );

			UIActionManager.CreateRowDivider( cardContent );
		}

		UIActionButton createRoleBtn = UIActionManager.CreateButton( cardContent, "#STR_COT_ROLEMANAGER_MODULE_NEW_ROLE_BUTTON", this, "" );
		if ( createRoleBtn ) createRoleBtn.SetOnClick( this, "OnClick_CreateRole" );
		createRoleBtn.SetColor( JMTheme.SUCCESS_FILL );
		createRoleBtn.SetTooltip( "#STR_COT_ROLEMANAGER_MODULE_NEW_ROLE_TOOLTIP" );

		m_Form.BindPermission( createRoleBtn, JMConstants.PERM_ROLES_CREATE );

		m_Form.m_LeftScroller.UpdateScroller();
	}

	void RebuildRolePermEditor()
	{
		m_TempRole = NULL;
		m_TempRoleBuiltFor = "";
		m_Collapsed.Clear();
		ClearEditor();

		if ( m_SelectedRole == "" )
		{
			UIActionManager.CreateText( m_Form.m_EditorWrapper, "#STR_COT_ROLEMANAGER_MODULE_SELECT_ROLE_PROMPT" );
			m_Form.m_RightScroller.UpdateScroller();
			return;
		}

		bool isDeletable = ( m_SelectedRole != "admin" && m_SelectedRole != "everyone" );

		// Permission Configuration Card
		UIActionCard permCard = UIActionManager.CreateCard( m_Form.m_EditorWrapper, Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_PERMISSION_CONFIG_TITLE" ) + ": " + m_SelectedRole );
		m_SavePermBtn = permCard.AddSaveButton( this, "", Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_SAVE_PERMISSIONS_TOOLTIP" ) + ": " + m_SelectedRole );
		if ( m_SavePermBtn ) m_SavePermBtn.SetOnClick( this, "OnClick_SaveRolePermissions" );
		m_Form.BindPermission( m_SavePermBtn, JMConstants.PERM_ROLES_PERMISSIONS );

		if ( isDeletable )
		{
			UIActionImageButton renameBtn = permCard.AddCardHeaderAction( JMConstants.ICON_FOLDED_PAPER, this, "", Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_TOOLTIP" ) + " '" + m_SelectedRole + "'" );
			if ( renameBtn ) renameBtn.SetOnClick( this, "OnClick_RenameRole" );
			renameBtn.SetData( new JMStringData( m_SelectedRole ) );

			if ( JMPermissions.Has( JMConstants.PERM_ROLES_DELETE ) )
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

		m_Form.m_RightScroller.UpdateScroller();
	}

	//! Clears the shared editor container and this tab's own registered
	//! button, mirroring what JMRoleManagerFormTabPlayers.ClearEditor() does
	//! for its own save button.
	protected void ClearEditor()
	{
		m_Form.UnregisterPermission( m_SavePermBtn );
		m_SavePermBtn = NULL;

		// The nodes point at widgets that are about to go, so they go first.
		m_PermRoots.Clear();

		m_PermTreeHost    = NULL;
		m_PermTreeWrapper = NULL;
		m_PermSearchBar   = NULL;

		m_Form.ClearEditorContainer();
	}

	//! Rebuilds just the tree portion of the permission editor - used both for
	//! the initial build and for every keystroke in the search box, so the card
	//! header, save/rename/delete buttons and m_TempRole are left untouched.
	protected void RebuildPermTreeWidgets()
	{
		if ( !m_TempRole || !m_PermTreeHost )
			return;

		m_PermRoots.Clear();

		if ( m_PermTreeWrapper )
			delete m_PermTreeWrapper;

		m_PermTreeWrapper = UIActionManager.CreateGridSpacer( m_PermTreeHost, 1, 1 );

		BuildPermissionTree( m_PermTreeWrapper, m_TempRole.RootPermission, NULL, 0 );
		RefreshPermTreeState();

		m_Form.m_RightScroller.UpdateScroller();
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
	protected bool PermSubtreeMatchesFilter( JMPermission perm )
	{
		if ( !perm )
			return false;

		JMSearchMatcher matcher = new JMSearchMatcher( m_PermSearchFilter );
		if ( matcher.Matches( perm.Name ) )
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

		m_Form.m_ForceEditorRefresh = true;

		if ( m_Form.m_Module )
			m_Form.m_Module.RequestRoleList();
	}

	void OnClick_CreateRole( UIActionBase action )
	{
		m_Form.PromptInput( "#STR_COT_ROLEMANAGER_MODULE_CREATE_ROLE_TITLE", "#STR_COT_ROLEMANAGER_MODULE_CREATE_ROLE_BODY", "OnCreateRole_Confirm" );
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
		m_Form.UpdateUI();
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

		m_Form.ConfirmAction( "#STR_COT_GENERIC_CONFIRM", delMsg, "OnDeleteRole_Confirm", "#STR_COT_GENERIC_YES", "#STR_COT_GENERIC_NO" );
	}

	//! Called by the form's OnDeleteRole_Confirm (JMConfirmation callback).
	string TakePendingDeleteRole()
	{
		string role = m_PendingDeleteRole;
		m_PendingDeleteRole = "";
		return role;
	}

	//! Leaving this mode drops its selection - the other mode starts with none.
	override void OnUnfocus()
	{
		SelectNone();
	}

	void SelectNone()
	{
		m_SelectedRole = "";
	}

	void OnClick_RenameRole( UIActionBase action )
	{
		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_EditingRenameRole = data.Value;

		string renameTitle = Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_TITLE" ) + ": " + m_EditingRenameRole;
		string renameBody  = string.Format( Widget.TranslateString( "#STR_COT_ROLEMANAGER_MODULE_RENAME_ROLE_BODY" ), m_EditingRenameRole );

		m_Form.PromptInput( renameTitle, renameBody, "ConfirmRenameRole" );
	}

	//! Called by the form's ConfirmRenameRole (JMConfirmation callback).
	void ApplyRename( string newName )
	{
		if ( m_EditingRenameRole == "" || !m_Form.m_Module || !m_TempRole )
			return;

		if ( newName == "" || newName == m_EditingRenameRole )
			return;

		m_Form.m_Module.CreateRole( newName );
		m_Form.m_Module.SetRolePermissions( newName, m_TempRole.Serialize() );
		m_Form.m_Module.DeleteRole( m_EditingRenameRole );

		m_SelectedRole = newName;
		m_EditingRenameRole = "";
	}

	void OnClick_SaveRolePermissions( UIActionBase action )
	{
		if ( m_SelectedRole == "" || !m_Form.m_Module || !m_TempRole )
			return;

		OptimizeInherit( m_TempRole.RootPermission );

		m_Form.m_Module.SetRolePermissions( m_SelectedRole, m_TempRole.Serialize() );

		UIActionImageButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.StopPulse();

		action.AnimateSpin( 1.0 );

		COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_ROLEMANAGER_MODULE_SAVED_PERMISSIONS_NOTIFICATION", m_SelectedRole ) );
	}

	protected void MarkRolePermChanged()
	{
		if ( m_SavePermBtn )
			m_SavePermBtn.AnimatePulse( 5.0 );
	}

	// =========================================================================
	//  Permission tree
	// =========================================================================

	protected void BuildPermissionTree( Widget parent, JMPermission perm, JMPermTreeNode parentNode, int depth )
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
				UIActionButton nodeBtn = UIActionManager.CreateButton( row, child.Name, this, "" );
				if ( nodeBtn ) nodeBtn.SetOnClick( this, "OnClick_ToggleCollapse" );
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
	protected float ChevronAngle( bool collapsed )
	{
		if ( collapsed )
			return -90;

		return 0;
	}

	void OnClick_ToggleCollapse( UIActionBase action )
	{
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

		if ( m_Form.m_RightScroller )
			m_Form.m_RightScroller.UpdateScroller();
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
	protected void DenySubtree( JMPermTreeNode node )
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
	protected void AllowSubtree( JMPermTreeNode node )
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
	protected void ApplySubtreeEnabled( JMPermTreeNode node, bool parentOn )
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

	protected int RowTextColor( bool enabled, bool isBranch )
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
	protected void RefreshPermTreeState()
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

	protected void OptimizeInherit( JMPermission perm )
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
	//! chain.
	protected bool EffectiveAllow( JMPermission perm )
	{
		if ( !perm )
			return false;

		if ( perm.Type == JMPermissionType.ALLOW )
			return true;

		if ( perm.Type == JMPermissionType.DISALLOW )
			return false;

		return ResolveParentAllows( perm );
	}

	protected bool ResolveParentAllows( JMPermission node )
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
}
