class JMRoleManagerForm : JMFormBase
{
	// -------------------------------------------------------------------------
	//  Mode
	// -------------------------------------------------------------------------

	private bool                        m_PlayerMode = false; // false = Roles, true = Players

	// -------------------------------------------------------------------------
	//  Widgets
	// -------------------------------------------------------------------------

	private UIActionScroller            m_LeftScroller;
	private Widget                      m_LeftContent;

	private UIActionScroller            m_RightScroller;
	private Widget                      m_RightContent;

	// Mode toggle buttons
	private UIActionButton              m_BtnModeRoles;
	private UIActionButton              m_BtnModePlayers;

	// Left panel — dynamic list area (roles or players)
	private Widget                      m_ListWrapper;

	// Right panel — editor
	private Widget                      m_EditorWrapper;
	private UIActionButton              m_SavePermBtn;
	private UIActionButton              m_SaveRolesBtn;

	// ---- Roles mode state --------------------------------------------------
	private string                      m_SelectedRole;
	private autoptr array<ref JMRoleData> m_RoleList = new array<ref JMRoleData>();
	private ref JMRole                  m_TempRole;
	private string                      m_PendingDeleteRole;

	// ---- Permission tree collapse state ------------------------------------
	// Key: permission full name (e.g. "Camera.View"), Value: true = collapsed
	private ref map< string, bool >     m_Collapsed = new map< string, bool >();

	// ---- Players mode state ------------------------------------------------
	private string                      m_SelectedGUID;
	private string                      m_SelectedPlayerName;
	private string                      m_PlayerSearchFilter;

	// Radio-button role selection: maps role name -> checkbox widget
	// Only one can be "selected" at a time
	private ref map< string, UIActionCheckbox > m_PlayerRoleChecks = new map< string, UIActionCheckbox >();
	private string                      m_PlayerSelectedRole;

	private JMRoleManagerModule         m_Module;

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
		Widget leftPanel  = layoutRoot.FindAnyWidget( "panel_left"  );
		Widget rightPanel = layoutRoot.FindAnyWidget( "panel_right" );

		// ---- Left panel ---------------------------------------------------
		m_LeftScroller = UIActionManager.CreateScroller( leftPanel );
		m_LeftContent  = m_LeftScroller.GetContentWidget();

		// Mode toggle
		Widget modeBar = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 2 );
		m_BtnModeRoles   = UIActionManager.CreateButton( modeBar, "Roles",   this, "OnClick_ModeRoles"   );
		m_BtnModePlayers = UIActionManager.CreateButton( modeBar, "Players", this, "OnClick_ModePlayers" );

		UIActionManager.CreatePanel( m_LeftContent, 0xFF444444, 2 );

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );
		UIActionManager.CreateText( m_ListWrapper, "Loading..." );

		m_LeftScroller.UpdateScroller();

		// ---- Right panel --------------------------------------------------
		m_RightScroller = UIActionManager.CreateScroller( rightPanel );
		m_RightContent  = m_RightScroller.GetContentWidget();

		m_EditorWrapper = UIActionManager.CreateGridSpacer( m_RightContent, 1, 1 );
		UIActionManager.CreateText( m_EditorWrapper, "Select an item on the left to begin." );

		m_RightScroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_LeftScroller  ) m_LeftScroller.UpdateScroller();
		if ( m_RightScroller ) m_RightScroller.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  OnShow
	// -------------------------------------------------------------------------

	override void OnShow()
	{
		super.OnShow();

		if ( m_Module )
			m_Module.RequestRoleList();
	}

	// -------------------------------------------------------------------------
	//  Mode helpers
	// -------------------------------------------------------------------------

	void OnClick_ModeRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PlayerMode )
		{
			m_PlayerMode = false;
			m_SelectedGUID = "";
			RebuildRoleList();
			RebuildEditorEmpty( "Select a role to edit its permissions." );
		}
	}

	void OnClick_ModePlayers( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_PlayerMode )
		{
			m_PlayerMode = true;
			m_SelectedRole = "";
			m_PlayerSearchFilter = "";
			RebuildPlayerList();
			RebuildEditorEmpty( "Select a player to manage their role." );
		}
	}

	// -------------------------------------------------------------------------
	//  PopulateRoleList — called by module on RPC receive
	// -------------------------------------------------------------------------

	void PopulateRoleList( array<ref JMRoleData> roles )
	{
		m_RoleList.Clear();
		foreach ( JMRoleData rd : roles )
			m_RoleList.Insert( rd );

		if ( !m_PlayerMode )
			RebuildRoleList();
	}

	// =========================================================================
	//  ROLES MODE
	// =========================================================================

	private void RebuildRoleList()
	{
		if ( m_ListWrapper )
			delete m_ListWrapper;

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );

		// Toolbar
		Widget toolbar = UIActionManager.CreateGridSpacer( m_ListWrapper, 1, 2 );
		UIActionManager.CreateButton( toolbar, "Refresh", this, "OnClick_Refresh" );
		UIActionButton createBtn = UIActionManager.CreateButton( toolbar, "New Role", this, "OnClick_CreateRole" );
		createBtn.SetColor( COLOR_GREEN );

		UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );

		foreach ( JMRoleData rd : m_RoleList )
		{
			UIActionButton selectBtn = UIActionManager.CreateButton( m_ListWrapper, rd.Name, this, "OnClick_SelectRole" );
			selectBtn.SetData( new JMStringData( rd.Name ) );

			UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );
		}

		m_LeftScroller.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Role permission editor
	// -------------------------------------------------------------------------

	private void RebuildRolePermEditor()
	{
		m_TempRole = NULL;
		m_Collapsed.Clear();
		RebuildEditorClear();

		if ( m_SelectedRole == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "Select a role to edit its permissions." );
			m_RightScroller.UpdateScroller();
			return;
		}

		// ---- Header row: title + Delete button on the right ----------------
		bool isDeletable = ( m_SelectedRole != "admin" && m_SelectedRole != "everyone" );

		int headerCols = 1;
		if ( isDeletable )
			headerCols = 2;

		Widget headerRow = UIActionManager.CreateGridSpacer( m_EditorWrapper, 1, headerCols );
		UIActionManager.CreateText( headerRow, "Role: " + m_SelectedRole );

		if ( isDeletable && GetPermissionsManager().HasPermission( "Admin.Roles.Delete" ) )
		{
			UIActionButton delBtn = UIActionManager.CreateButton( headerRow, "Delete", this, "OnClick_DeleteRole" );
			delBtn.SetColor( COLOR_RED );
			delBtn.SetData( new JMStringData( m_SelectedRole ) );
		}

		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );

		JMRoleData roleData = GetRoleData( m_SelectedRole );

		m_TempRole = new JMRole( m_SelectedRole );
		if ( roleData )
		{
			foreach ( string line : roleData.Permissions )
				m_TempRole.AddPermission( line );
		}

		// Build collapsable permission tree
		BuildPermissionTree( m_EditorWrapper, m_TempRole.RootPermission, 0 );

		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );
		m_SavePermBtn = UIActionManager.CreateButton( m_EditorWrapper, "Save Permissions", this, "OnClick_SaveRolePermissions" );
		m_SavePermBtn.SetColor( COLOR_GREEN );

		m_RightScroller.UpdateScroller();
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Module )
			m_Module.RequestRoleList();
	}

	void OnClick_CreateRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Create Role", "Enter new role name:", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "OnCreateRole_Confirm" );
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
	}

	void OnClick_DeleteRole( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_PendingDeleteRole = data.Value;

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM",
			"Delete role '" + m_PendingDeleteRole + "'? Players with this role will lose its permissions.",
			"#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnDeleteRole_Confirm" );
	}

	void OnDeleteRole_Confirm( JMConfirmation confirmation )
	{
		if ( m_PendingDeleteRole == "" || !m_Module )
			return;

		if ( m_SelectedRole == m_PendingDeleteRole )
		{
			m_SelectedRole = "";
			RebuildEditorEmpty( "Select a role to edit its permissions." );
		}

		m_Module.DeleteRole( m_PendingDeleteRole );
		m_PendingDeleteRole = "";
	}

	void OnClick_SaveRolePermissions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_SelectedRole == "" || !m_Module || !m_TempRole )
			return;

		OptimizeInherit( m_TempRole.RootPermission );

		m_Module.SetRolePermissions( m_SelectedRole, m_TempRole.Serialize() );

		COTCreateLocalAdminNotification( new StringLocaliser( "Saved permissions for role: " + m_SelectedRole ) );
	}

	// =========================================================================
	//  PLAYERS MODE
	// =========================================================================

	private void RebuildPlayerList()
	{
		if ( m_ListWrapper )
			delete m_ListWrapper;

		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();

		// Toolbar: Refresh + count
		Widget playerToolbar = UIActionManager.CreateGridSpacer( m_ListWrapper, 1, 2 );
		UIActionManager.CreateButton( playerToolbar, "Refresh", this, "OnClick_RefreshPlayers" );
		UIActionManager.CreateText( playerToolbar, "" + players.Count() + " online" );

		// Search bar
		UIActionEditableTextPreview searchBar = UIActionManager.CreateEditableTextPreview(
			m_ListWrapper, "Search by name...", this, "OnChange_PlayerSearch" );
		searchBar.SetWidth( 1.0 );
		searchBar.SetEditBoxWidth( 0.85 );

		UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );

		if ( players.Count() == 0 )
		{
			UIActionManager.CreateText( m_ListWrapper, "No players online." );
		}
		else
		{
			string selfGUID = "";
			JMPlayerInstance selfInst = GetPermissionsManager().GetClientPlayer();
			if ( selfInst )
				selfGUID = selfInst.GetGUID();

			string filterLow = m_PlayerSearchFilter;
			filterLow.ToLower();

			bool anyShown = false;
			foreach ( JMPlayerInstance pi : players )
			{
				// Apply search filter
				if ( filterLow != "" )
				{
					string nameLow = pi.GetName();
					nameLow.ToLower();
					if ( nameLow.IndexOf( filterLow ) == -1 )
						continue;
				}

				anyShown = true;

				// Name only — no GUID displayed in the list
				UIActionButton btn = UIActionManager.CreateButton( m_ListWrapper, pi.GetName(), this, "OnClick_SelectPlayer" );
				btn.SetData( new JMStringData( pi.GetGUID() ) );

				if ( pi.GetGUID() == selfGUID )
					btn.SetColor( 0xFFFF8800 );

				UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );
			}

			if ( !anyShown )
				UIActionManager.CreateText( m_ListWrapper, "No players match the search." );
		}

		m_LeftScroller.UpdateScroller();
	}

	void OnClick_RefreshPlayers( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_PlayerSearchFilter = "";
		RebuildPlayerList();
	}

	void OnChange_PlayerSearch( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UIActionEditableTextPreview searchBar;
		if ( !Class.CastTo( searchBar, action ) )
			return;

		searchBar.SetTextPreview( "" );
		m_PlayerSearchFilter = searchBar.GetText();
		RebuildPlayerList();
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
	}

	private void RebuildPlayerEditor()
	{
		m_PlayerRoleChecks.Clear();
		m_PlayerSelectedRole = "";
		RebuildEditorClear();

		if ( m_SelectedGUID == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "Select a player to manage their role." );
			m_RightScroller.UpdateScroller();
			return;
		}

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( m_SelectedGUID );
		if ( !pi )
		{
			UIActionManager.CreateText( m_EditorWrapper, "Player not found: " + m_SelectedPlayerName );
			m_RightScroller.UpdateScroller();
			return;
		}

		UIActionManager.CreateText( m_EditorWrapper, "Player: " + m_SelectedPlayerName );
		// GUID (internal identity ID)
		UIActionManager.CreateText( m_EditorWrapper, "GUID: " + pi.GetGUID() );
		// Steam 64 ID (plainId)
		UIActionManager.CreateText( m_EditorWrapper, "Steam: " + pi.GetSteam64ID() );
		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );

		UIActionManager.CreateText( m_EditorWrapper, "Role:" );
		UIActionManager.CreatePanel( m_EditorWrapper, 0x44FFFFFF, 1 );

		// Determine current role (first non-"everyone" role, if any)
		array<string> currentRoles = pi.GetRoles();
		foreach ( string rn : currentRoles )
		{
			if ( rn != "everyone" )
			{
				m_PlayerSelectedRole = rn;
				break;
			}
		}

		// Build radio-button style role list
		array<JMRole> allRoles = new array<JMRole>();
		GetPermissionsManager().GetRolesAsList( allRoles );

		foreach ( JMRole role : allRoles )
		{
			if ( role.Name == "everyone" )
				continue;

			bool isSelected = ( role.Name == m_PlayerSelectedRole );
			UIActionCheckbox cb = UIActionManager.CreateCheckbox( m_EditorWrapper, role.Name, this, "OnClick_SelectRole_Radio", isSelected );
			cb.SetData( new JMStringData( role.Name ) );
			m_PlayerRoleChecks.Insert( role.Name, cb );
		}

		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );
		m_SaveRolesBtn = UIActionManager.CreateButton( m_EditorWrapper, "Save Role", this, "OnClick_SavePlayerRole" );
		m_SaveRolesBtn.SetColor( COLOR_GREEN );

		m_RightScroller.UpdateScroller();
	}

	// Radio-button behaviour: uncheck all others when one is checked
	void OnClick_SelectRole_Radio( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionCheckbox cb;
		if ( !Class.CastTo( cb, action ) )
			return;

		JMStringData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		if ( cb.IsChecked() )
		{
			m_PlayerSelectedRole = data.Value;

			// Uncheck all other roles
			for ( int i = 0; i < m_PlayerRoleChecks.Count(); i++ )
			{
				string roleName = m_PlayerRoleChecks.GetKey( i );
				if ( roleName != data.Value )
					m_PlayerRoleChecks.GetElement( i ).SetChecked( false );
			}
		}
		else
		{
			// Unchecking = no role selected
			m_PlayerSelectedRole = "";
		}
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

		playerModule.SetRoles( roles, { m_SelectedGUID } );

		COTCreateLocalAdminNotification( new StringLocaliser( "Saved role for player: " + m_SelectedPlayerName ) );
	}

	// =========================================================================
	//  Shared editor helpers
	// =========================================================================

	private void RebuildEditorClear()
	{
		m_SavePermBtn  = NULL;
		m_SaveRolesBtn = NULL;

		if ( m_EditorWrapper )
			delete m_EditorWrapper;

		m_EditorWrapper = UIActionManager.CreateGridSpacer( m_RightContent, 1, 1 );
	}

	private void RebuildEditorEmpty( string msg )
	{
		RebuildEditorClear();
		UIActionManager.CreateText( m_EditorWrapper, msg );
		m_RightScroller.UpdateScroller();
	}

	// =========================================================================
	//  Collapsable permission tree (checkbox only)
	// =========================================================================

	// Build tree rows for permission and its children.
	//
	// Indentation is done purely via widget widths (font-independent).
	// The connector label is only the local symbol — no spaces for indent.
	//
	// Each row is a 3-column grid:
	//   col A: indent spacer    — width = depth * INDENT_UNIT, empty text
	//   col B: connector/toggle — width = CONNECTOR_W, text = "|---" / " \--" / "[+]" / "[-]"
	//   col C: checkbox         — width = remaining
	//
	// A separate pass draws the vertical continuation pipes for open ancestor
	// levels as thin panel widgets behind each row (not possible without
	// absolute positioning), so instead we encode the pipe in col A text
	// only for the immediate parent level — one "|" at the correct indent.
	private void BuildPermissionTree( Widget parent, JMPermission perm, int depth )
	{
		if ( !perm )
			return;

		// Each depth level adds this much width to the indent spacer
		static const float INDENT_UNIT  = 0.06;
		// Width of the connector symbol cell
		static const float CONNECTOR_W  = 0.10;

		int count = perm.Children.Count();
		for ( int i = 0; i < count; i++ )
		{
			JMPermission child = perm.Children[i];
			if ( !child )
				continue;

			bool hasChildren = child.Children.Count() > 0;
			bool isLast      = ( i == count - 1 );
			string fullName  = child.GetFullName();

			// All nodes start collapsed by default.
			bool isCollapsed = true;
			if ( m_Collapsed.Contains( fullName ) )
				isCollapsed = m_Collapsed.Get( fullName );

			// ---- Connector symbol for this node ----------------------------
			// Depth-0 nodes are roots — no connector, just the toggle indicator.
			// Deeper nodes: non-last sibling gets "|---", last sibling gets " \--"
			string connector = "";
			if ( depth > 0 )
			{
				if ( isLast )
					connector = " \\--";
				else
					connector = "|---";
			}

			// ---- Toggle label for branch nodes -----------------------------
			string toggleLabel = "";
			if ( hasChildren )
			{
				if ( isCollapsed )
					toggleLabel = "[+]";
				else
					toggleLabel = "[-]";
			}

			// ---- Column widths ---------------------------------------------
			float indentW = depth * INDENT_UNIT;
			float cbPos   = indentW + CONNECTOR_W;
			float cbW     = 1.0 - cbPos;

			// ---- Row: 3 columns --------------------------------------------
			Widget row = UIActionManager.CreateGridSpacer( parent, 1, 2 );

			// Col A — connector + toggle.
			// Depth 0 with children: just the toggle label, no connector prefix.
			// Depth 0 leaf: empty cell.
			// Deeper branch: connector + toggle as a button.
			// Deeper leaf: connector as plain text.
			string connLabel = connector;
			if ( depth == 0 )
				connLabel = toggleLabel;
			else if ( toggleLabel != "" )
				connLabel = connector + " " + toggleLabel;

			if ( hasChildren )
			{
				UIActionButton toggleBtn = UIActionManager.CreateButton( row, connLabel, this, "OnClick_ToggleCollapse" );
				toggleBtn.SetWidth( CONNECTOR_W );
				toggleBtn.SetPosition( indentW );
				toggleBtn.SetData( new JMPermCollapseData( fullName, depth ) );
			}
			else
			{
				UIActionText connText = UIActionManager.CreateText( row, connLabel );
				connText.SetWidth( CONNECTOR_W );
				connText.SetPosition( indentW );
			}

			// Col B — permission checkbox
			bool isAllow = ( child.Type == JMPermissionType.ALLOW );
			UIActionCheckbox cb = UIActionManager.CreateCheckbox( row, child.Name, this, "OnClick_PermCheckbox", isAllow );
			cb.SetWidth( cbW );
			cb.SetPosition( cbPos );
			cb.SetData( new JMPermNodeData( child ) );

			// ---- Recurse into children if expanded -------------------------
			if ( hasChildren && !isCollapsed )
				BuildPermissionTree( parent, child, depth + 1 );
		}
	}

	void OnClick_PermCheckbox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionCheckbox cb;
		if ( !Class.CastTo( cb, action ) )
			return;

		JMPermNodeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		if ( !data.Perm )
			return;

		if ( cb.IsChecked() )
			data.Perm.Type = JMPermissionType.ALLOW;
		else
			data.Perm.Type = JMPermissionType.DISALLOW;
	}

	void OnClick_ToggleCollapse( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPermCollapseData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		// All nodes start collapsed by default.
		bool cur = true;
		if ( m_Collapsed.Contains( data.FullName ) )
			cur = m_Collapsed.Get( data.FullName );

		// Remove first to guarantee overwrite — Insert may not update existing keys.
		m_Collapsed.Remove( data.FullName );
		m_Collapsed.Insert( data.FullName, !cur );

		// Re-draw the permission tree without resetting m_TempRole,
		// so unsaved checkbox changes are preserved.
		RebuildPermTreeOnly();
	}

	// Rebuild just the tree widget area inside the existing editor wrapper,
	// keeping m_TempRole intact so in-progress edits are not lost.
	private void RebuildPermTreeOnly()
	{
		if ( !m_TempRole || !m_EditorWrapper )
			return;

		// Destroy existing editor content and recreate it in-place.
		// We must keep m_EditorWrapper itself (it's a child of m_RightContent).
		Widget child = m_EditorWrapper.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		bool isDeletable = ( m_SelectedRole != "admin" && m_SelectedRole != "everyone" );

		int headerCols2 = 1;
		if ( isDeletable )
			headerCols2 = 2;

		Widget headerRow = UIActionManager.CreateGridSpacer( m_EditorWrapper, 1, headerCols2 );
		UIActionManager.CreateText( headerRow, "Role: " + m_SelectedRole );

		if ( isDeletable && GetPermissionsManager().HasPermission( "Admin.Roles.Delete" ) )
		{
			UIActionButton delBtn = UIActionManager.CreateButton( headerRow, "Delete", this, "OnClick_DeleteRole" );
			delBtn.SetColor( COLOR_RED );
			delBtn.SetData( new JMStringData( m_SelectedRole ) );
		}

		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );

		BuildPermissionTree( m_EditorWrapper, m_TempRole.RootPermission, 0 );

		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );
		m_SavePermBtn = UIActionManager.CreateButton( m_EditorWrapper, "Save Permissions", this, "OnClick_SaveRolePermissions" );
		m_SavePermBtn.SetColor( COLOR_GREEN );

		m_RightScroller.UpdateScroller();
	}

	// =========================================================================
	//  Inherit optimisation
	//  Walk the permission tree; for each node whose explicit type would produce
	//  the same resolved result as inheriting from its parent, set it to INHERIT.
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

			// Resolve what the parent chain gives this child if it were INHERIT
			bool parentAllows = ResolveParentAllows( child );

			// If child's explicit ALLOW/DISALLOW matches what inheritance would give,
			// collapse it back to INHERIT.
			if ( child.Type == JMPermissionType.ALLOW && parentAllows )
				child.Type = JMPermissionType.INHERIT;
			else if ( child.Type == JMPermissionType.DISALLOW && !parentAllows )
				child.Type = JMPermissionType.INHERIT;

			OptimizeInherit( child );
		}
	}

	// Returns what the inherited resolution would be for a node if it were INHERIT.
	// Walks up the parent chain to find the first non-INHERIT ancestor type.
	private bool ResolveParentAllows( JMPermission node )
	{
		JMPermission p = node.Parent;
		while ( p != NULL )
		{
			if ( p.Type == JMPermissionType.ALLOW )
				return true;
			if ( p.Type == JMPermissionType.DISALLOW )
				return false;
			// INHERIT — keep walking up
			p = p.Parent;
		}
		// Root is always DISALLOW by default
		return false;
	}

	// -------------------------------------------------------------------------
	//  Misc helpers
	// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
//  Simple data carrier for a single string attached to a button
// -------------------------------------------------------------------------
class JMStringData : UIActionData
{
	string Value;

	void JMStringData( string val )
	{
		Value = val;
	}
}

// -------------------------------------------------------------------------
//  Data carrier for permission node reference (checkbox)
// -------------------------------------------------------------------------
class JMPermNodeData : UIActionData
{
	JMPermission Perm;

	void JMPermNodeData( JMPermission perm )
	{
		Perm = perm;
	}
}

// -------------------------------------------------------------------------
//  Data carrier for collapse toggle (stores full permission path)
// -------------------------------------------------------------------------
class JMPermCollapseData : UIActionData
{
	string FullName;
	int    Depth;

	void JMPermCollapseData( string fullName, int depth )
	{
		FullName = fullName;
		Depth    = depth;
	}
}
