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

	// ---- Players mode state ------------------------------------------------
	private string                      m_SelectedGUID;
	private string                      m_SelectedPlayerName;
	private ref JMPermission            m_PlayerRootPerm;  // live tree modified in-place
	private ref array<ref JMRoleRowWidget> m_PlayerRoleWidgets = new array<ref JMRoleRowWidget>();

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

		// Toolbar (role-specific, hidden in player mode)
		m_ListWrapper = UIActionManager.CreateGridSpacer( m_LeftContent, 1, 1 );
		UIActionManager.CreateText( m_ListWrapper, "Loading..." );

		m_LeftScroller.UpdateScroller();

		// ---- Right panel --------------------------------------------------
		m_RightScroller = UIActionManager.CreateScroller( rightPanel );
		m_RightContent  = m_RightScroller.GetContentWidget();

		UIActionManager.CreateText( m_RightContent, "Select an item on the left to begin." );

		m_RightScroller.UpdateScroller();

		UpdateModeButtons();
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

	private void UpdateModeButtons()
	{
		if ( !m_PlayerMode )
		{
			m_BtnModeRoles.SetColor( COLOR_GREEN );
			m_BtnModePlayers.SetColor( 0x00000000 );
		}
		else
		{
			m_BtnModeRoles.SetColor( 0x00000000 );
			m_BtnModePlayers.SetColor( COLOR_GREEN );
		}
	}

	void OnClick_ModeRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_PlayerMode )
		{
			m_PlayerMode = false;
			m_SelectedGUID = "";
			UpdateModeButtons();
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
			UpdateModeButtons();
			RebuildPlayerList();
			RebuildEditorEmpty( "Select a player to edit their permissions and roles." );
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
		UIActionButton createBtn = UIActionManager.CreateButton( toolbar, "Create Role", this, "OnClick_CreateRole" );
		createBtn.SetColor( COLOR_GREEN );

		UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );

		bool canDelete = GetPermissionsManager().HasPermission( "Admin.Roles.Delete" );

		foreach ( JMRoleData rd : m_RoleList )
		{
			Widget row = UIActionManager.CreateGridSpacer( m_ListWrapper, 1, 2 );

			UIActionButton selectBtn = UIActionManager.CreateButton( row, rd.Name, this, "OnClick_SelectRole" );
			selectBtn.SetData( new JMStringData( rd.Name ) );

			if ( canDelete && rd.Name != "everyone" )
			{
				UIActionButton delBtn = UIActionManager.CreateButton( row, "Delete", this, "OnClick_DeleteRole" );
				delBtn.SetColor( COLOR_RED );
				delBtn.SetData( new JMStringData( rd.Name ) );
			}
			else
			{
				UIActionManager.CreateText( row, "" );
			}

			UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );
		}

		m_LeftScroller.UpdateScroller();
	}

	private void RebuildRolePermEditor()
	{
		m_TempRole = NULL;
		RebuildEditorClear();

		if ( m_SelectedRole == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "Select a role to edit its permissions." );
			m_RightScroller.UpdateScroller();
			return;
		}

		UIActionManager.CreateText( m_EditorWrapper, "Permissions: " + m_SelectedRole );
		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );

		JMRoleData roleData = GetRoleData( m_SelectedRole );

		m_TempRole = new JMRole( m_SelectedRole );
		if ( roleData )
		{
			foreach ( string line : roleData.Permissions )
				m_TempRole.AddPermission( line );
		}

		BuildPermissionWidgets( m_EditorWrapper, m_TempRole.RootPermission );

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

		if ( players.Count() == 0 )
		{
			UIActionManager.CreateText( m_ListWrapper, "No players online." );
		}
		else
		{
			foreach ( JMPlayerInstance pi : players )
			{
				UIActionButton btn = UIActionManager.CreateButton( m_ListWrapper, pi.GetName(), this, "OnClick_SelectPlayer" );
				btn.SetData( new JMStringData( pi.GetGUID() ) );

				UIActionManager.CreatePanel( m_ListWrapper, 0x22FFFFFF, 1 );
			}
		}

		m_LeftScroller.UpdateScroller();
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
		m_PlayerRootPerm = NULL;
		m_PlayerRoleWidgets.Clear();
		RebuildEditorClear();

		if ( m_SelectedGUID == "" )
		{
			UIActionManager.CreateText( m_EditorWrapper, "Select a player to edit their permissions and roles." );
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
		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );

		// ---- Permissions section ------------------------------------------
		UIActionManager.CreateText( m_EditorWrapper, "Individual Permissions" );
		UIActionManager.CreatePanel( m_EditorWrapper, 0x44FFFFFF, 1 );

		m_PlayerRootPerm = pi.GetPermissions();
		BuildPermissionWidgets( m_EditorWrapper, m_PlayerRootPerm );

		m_SavePermBtn = UIActionManager.CreateButton( m_EditorWrapper, "Save Permissions", this, "OnClick_SavePlayerPermissions" );
		m_SavePermBtn.SetColor( COLOR_GREEN );

		// ---- Roles section -----------------------------------------------
		UIActionManager.CreatePanel( m_EditorWrapper, 0xFF444444, 2 );
		UIActionManager.CreateText( m_EditorWrapper, "Roles" );
		UIActionManager.CreatePanel( m_EditorWrapper, 0x44FFFFFF, 1 );

		array<JMRole> allRoles = new array<JMRole>();
		GetPermissionsManager().GetRolesAsList( allRoles );

		foreach ( JMRole role : allRoles )
		{
			Widget roleWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/role_widget.layout", m_EditorWrapper );
			if ( !roleWidget )
				continue;

			JMRoleRowWidget rScript;
			roleWidget.GetScript( rScript );
			if ( !rScript )
				continue;

			rScript.Show();
			rScript.InitRole( role.Name );
			rScript.SetChecked( pi.HasRole( role.Name ) );

			if ( role.Name == "everyone" )
				rScript.Disable();

			m_PlayerRoleWidgets.Insert( rScript );
		}

		m_SaveRolesBtn = UIActionManager.CreateButton( m_EditorWrapper, "Save Roles", this, "OnClick_SavePlayerRoles" );
		m_SaveRolesBtn.SetColor( COLOR_GREEN );

		m_RightScroller.UpdateScroller();
	}

	void OnClick_SavePlayerPermissions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_SelectedGUID == "" || !m_PlayerRootPerm )
			return;

		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		if ( !playerModule )
			return;

		playerModule.SetPermissions( m_PlayerRootPerm, { m_SelectedGUID } );

		COTCreateLocalAdminNotification( new StringLocaliser( "Saved permissions for player: " + m_SelectedPlayerName ) );
	}

	void OnClick_SavePlayerRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_SelectedGUID == "" )
			return;

		JMPlayerModule playerModule = CF_Modules<JMPlayerModule>.Get();
		if ( !playerModule )
			return;

		array<string> roles = new array<string>();
		foreach ( JMRoleRowWidget rWidget : m_PlayerRoleWidgets )
		{
			if ( rWidget.IsChecked() )
				roles.Insert( rWidget.Name );
		}

		playerModule.SetRoles( roles, { m_SelectedGUID } );

		COTCreateLocalAdminNotification( new StringLocaliser( "Saved roles for player: " + m_SelectedPlayerName ) );
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

	private void BuildPermissionWidgets( Widget parent, JMPermission permRoot )
	{
		array<JMPermission> permList = new array<JMPermission>();
		GetPermissionsManager().GetPermissionsAsList( permRoot, permList );

		string permWidgetLayout = "JM/COT/GUI/layouts/permission_widget.layout";

		foreach ( JMPermission perm : permList )
		{
			Widget permWidget = GetGame().GetWorkspace().CreateWidgets( permWidgetLayout, parent );
			if ( !permWidget )
				continue;

			JMPermissionRowWidget prScript;
			permWidget.GetScript( prScript );
			if ( !prScript )
				continue;

			prScript.InitPermission( perm );
			prScript.Enable();
		}
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

