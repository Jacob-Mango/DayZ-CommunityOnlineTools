class JMPlayerForm extends JMFormBase
{
	private autoptr array< JMPlayerRowWidget > m_PlayerList;
	private autoptr array< JMPermissionRowWidget > m_PermissionList;
	private autoptr array< JMRoleRowWidget > m_RoleList;

	private Widget m_LeftPanel;
	private Widget m_RightPanel;
	private Widget m_RightPanelDisable;

	private UIActionText m_PlayerListCount;
	private UIActionEditableText m_PlayerListFilter;
	private UIActionCheckbox m_PlayerListSort; //TODO: make a new ui action type thing
	
	private UIActionScroller m_PlayerListScroller;
	private Widget m_PlayerListRows;

	private UIActionScroller m_PermissionsListScroller;
	private Widget m_PermissionsRows;

	private UIActionScroller m_RolesListScroller;
	private Widget m_RolesRows;

	private UIActionScroller m_ActionListScroller;
	private Widget m_ActionsWrapper;

	private UIActionText m_IdentityHeading;
	private UIActionText m_GUID;
	private UIActionText m_Name;
	private UIActionText m_Steam64ID;

	private UIActionButton m_RefreshStats;
	private UIActionButton m_ApplyStats;
	private UIActionEditableText m_Health;
	private bool m_HealthUpdated;
	private UIActionEditableText m_Blood;
	private bool m_BloodUpdated;
	private UIActionEditableText m_Energy;
	private bool m_EnergyUpdated;
	private UIActionEditableText m_Water;
	private bool m_WaterUpdated;
	private UIActionEditableText m_Shock;
	private bool m_ShockUpdated;
	private UIActionEditableText m_Stamina;
	private bool m_StaminaUpdated;
	private UIActionCheckbox m_BloodyHands;
	private UIActionCheckbox m_GodMode;
	private UIActionCheckbox m_Freeze;
	private UIActionCheckbox m_Invisibility;
	private UIActionCheckbox m_UnlimitedAmmo;
	private UIActionCheckbox m_UnlimitedStamina;
	private UIActionCheckbox m_BrokenLegs;

	private UIActionButton m_TeleportToMe;
	private UIActionButton m_TeleportMeTo;
	private UIActionButton m_TeleportPrevious;

	private UIActionEditableText m_PositionX;
	private UIActionEditableText m_PositionY;
	private UIActionEditableText m_PositionZ;
	private UIActionButton m_PositionRefresh;
	private UIActionButton m_Position;

	private UIActionButton m_ModifyPermissions;
	private UIActionButton m_SavePermissions;
	private UIActionButton m_ModifyRoles;
	private UIActionButton m_SaveRoles;

	private UIActionButton m_RepairTransport;
	private UIActionButton m_SpectatePlayer;
	private UIActionButton m_HealPlayer;
	private UIActionButton m_StopBleeding;
	private UIActionButton m_StripPlayer;
	private UIActionButton m_DryPlayer;

	private int m_NumPlayerCount;

	private JMPlayerInstance m_SelectedInstance;

	private JMPlayerModule m_Module;

	void JMPlayerForm()
	{
		m_PlayerList = new array< JMPlayerRowWidget >;
		m_PermissionList = new array< JMPermissionRowWidget >;
		m_RoleList = new array< JMRoleRowWidget >;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerForm()
	{
		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		UpdatePermission( m_HealPlayer, "Admin.Player.Set" );
		UpdatePermission( m_Health, "Admin.Player.Set.Health" );
		UpdatePermission( m_Shock, "Admin.Player.Set.Shock" );
		UpdatePermission( m_Blood, "Admin.Player.Set.Blood" );
		UpdatePermission( m_Energy, "Admin.Player.Set.Energy" );
		UpdatePermission( m_Water, "Admin.Player.Set.Water" );
		UpdatePermission( m_Stamina, "Admin.Player.Set.Stamina" );
		UpdatePermission( m_BloodyHands, "Admin.Player.Set.BloodyHands" );

		UpdatePermission( m_ModifyPermissions, "Admin.Player.Permissions" );
		UpdatePermission( m_ModifyRoles, "Admin.Player.Roles" );

		UpdatePermission( m_Freeze, "Admin.Player.Freeze" );
		UpdatePermission( m_Invisibility, "Admin.Player.Invisibility" );
		UpdatePermission( m_UnlimitedAmmo, "Admin.Player.UnlimitedAmmo" );
		UpdatePermission( m_UnlimitedStamina, "Admin.Player.UnlimitedStamina" );
		UpdatePermission( m_BrokenLegs, "Admin.Player.BrokenLegs" );
		UpdatePermission( m_GodMode, "Admin.Player.Godmode" );
		UpdatePermission( m_SpectatePlayer, "Admin.Player.StartSpectating" );
		UpdatePermission( m_StopBleeding, "Admin.Player.StopBleeding" );
		UpdatePermission( m_StripPlayer, "Admin.Player.Strip" );
		UpdatePermission( m_DryPlayer, "Admin.Player.Dry" );

		UpdatePermission( m_PositionX, "Admin.Player.Teleport.Position" );
		UpdatePermission( m_PositionY, "Admin.Player.Teleport.Position" );
		UpdatePermission( m_PositionZ, "Admin.Player.Teleport.Position" );
		UpdatePermission( m_Position, "Admin.Player.Teleport.Position" ); 

		UpdatePermission( m_TeleportToMe, "Admin.Player.Teleport.Position" );
		UpdatePermission( m_TeleportMeTo, "Admin.Player.Teleport.SenderTo" );
		UpdatePermission( m_TeleportPrevious, "Admin.Player.Teleport.Previous" );

		UpdatePermission( m_RepairTransport, "Admin.Transport.Repair" );

		UpdateUI();
	}

	private void UpdatePermission( ref UIActionBase base, string permission )
	{
		if ( !base )
			return;

		base.UpdatePermission( permission );
	}

	override void OnInit()
	{
		InitWidgetsLeft();
		InitWidgetsRight();
	}

	private void InitWidgetsLeft()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::InitWidgetsLeft" );
		#endif

		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		Widget leftPanelGrid = UIActionManager.CreateGridSpacer( m_LeftPanel, 4, 1 );

		m_PlayerListCount = UIActionManager.CreateText( leftPanelGrid, "#STR_COT_PLAYER_MODULE_LEFT_PLAYER_COUNT", "" );
		m_PlayerListFilter = UIActionManager.CreateEditableText( leftPanelGrid, "#STR_COT_PLAYER_MODULE_LEFT_FILTER", this, "Event_UpdatePlayerList" );
		m_PlayerListSort = UIActionManager.CreateCheckbox( leftPanelGrid, "#STR_COT_PLAYER_MODULE_LEFT_SORT", this, "Event_UpdatePlayerList" );
		m_PlayerListScroller = UIActionManager.CreateScroller( leftPanelGrid );
		m_PlayerListRows = UIActionManager.CreateActionRows( m_PlayerListScroller.GetContentWidget() );
		
		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			gsw.Show( false ); //todo: verify this is correct

			for ( int j = 0; j < 100; j++ )
			{
				Widget prWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_widget.layout", gsw );
				
				if ( !prWidget )
					continue;

				JMPlayerRowWidget prScript;
				prWidget.GetScript( prScript );

				if ( !prScript )
					continue;

				prScript.SetPlayer( "" );
				prScript.Menu = this;

				m_PlayerList.Insert( prScript );
			}
		}

		m_PlayerListScroller.UpdateScroller();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::InitWidgetsLeft" );
		#endif
	}

	private void InitWidgetsRight()
	{
		m_RightPanel = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );

		m_ActionListScroller = UIActionManager.CreateScroller( m_RightPanel );
		m_ActionsWrapper = UIActionManager.CreateGridSpacer( m_ActionListScroller.GetContentWidget(), 9, 1 );
		
		InitActionWidgetsIdentity( m_ActionsWrapper );
		InitActionWidgetsPosition( m_ActionsWrapper );
		InitActionWidgetsStats( m_ActionsWrapper );
		InitActionWidgetsQuick( m_ActionsWrapper );
		InitActionWidgetsPermissions( m_ActionsWrapper );

		m_ActionListScroller.UpdateScroller();
	}

	private Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		m_IdentityHeading = UIActionManager.CreateText( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_HEADER", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 3, 1 );

		m_Name = UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_NAME", "" );
		m_GUID = UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_GUID", "" );
		m_Steam64ID = UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_STEAMID", "" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		ShowIdentityWidgets();

		return parent;
	}

	private void ShowIdentityWidgets()
	{
		m_IdentityHeading.SetText( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_HEADER_DESC" );

		m_Name.Show();
		m_GUID.Show();
		m_Steam64ID.Show();
	}

	private void HideIdentityWidgets()
	{
		m_IdentityHeading.SetText( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_HEADER_DESC_DISABLED" );

		m_Name.Hide();
		m_GUID.Hide();
		m_Steam64ID.Hide();
	}

	private Widget InitActionWidgetsPosition( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		UIActionManager.CreateText( parent, "Teleporting: ", "" );

		Widget positionActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget positionActionsVec = UIActionManager.CreateGridSpacer( positionActions, 1, 3 );

		m_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:" );
		m_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:" );
		m_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:" );

		m_PositionX.SetOnlyNumbers( true );
		m_PositionY.SetOnlyNumbers( true );
		m_PositionZ.SetOnlyNumbers( true );
		
		Widget positionActionsBut = UIActionManager.CreateGridSpacer( positionActions, 1, 2 );
		m_PositionRefresh = UIActionManager.CreateButton( positionActionsBut, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_REFRESH_COORDINATES", this, "Click_RefreshTeleports" );
		m_Position = UIActionManager.CreateButton( positionActionsBut, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES", this, "Click_SetPosition" );

		Widget teleportActions = UIActionManager.CreateGridSpacer( parent, 1, 3 );
		m_TeleportToMe = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_ME", this, "Click_TeleportToMe" );
		m_TeleportMeTo = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", this, "Click_TeleportMeTo" );
		m_TeleportPrevious = UIActionManager.CreateButton( teleportActions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_PREVIOUS", this, "Click_TeleportPrevious" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	private Widget InitActionWidgetsStats( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		Widget header = UIActionManager.CreateGridSpacer( parent, 1, 2 );
		UIActionManager.CreateText( header, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEADER", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 4, 2 );

		m_Health = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEALTH", this, "Click_SetHealth", "", "" );
		m_Shock = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_SHOCK", this, "Click_SetShock", "", "" );
		m_Blood = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOOD", this, "Click_SetBlood", "", "" );
		m_Energy = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FOOD", this, "Click_SetEnergy", "", "" );
		m_Water = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_WATER", this, "Click_SetWater", "", "" );
		m_Stamina = UIActionManager.CreateEditableText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_STAMINA", this, "Click_SetStamina", "", "" );

		m_ApplyStats = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_APPLY", this, "Click_ApplyStats" );
		m_RefreshStats = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_REFRESH", this, "Click_RefreshStats" );

		Widget actions2 = UIActionManager.CreateGridSpacer( parent, 4, 2 );

		m_BloodyHands = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOODY_HANDS", this, "Click_BloodyHands", false );
		m_GodMode = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_GODMODE", this, "Click_GodMode", false );
		m_Freeze = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FREEZE", this, "Click_Freeze", false );
		m_Invisibility = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_INVISIBLE", this, "Click_Invisible", false );
		m_UnlimitedAmmo = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_AMMO", this, "Click_UnlimitedAmmo", false );
		m_UnlimitedStamina = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_STAMINA", this, "Click_UnlimitedStamina", false );
		m_BrokenLegs = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BROKEN_LEGS", this, "Click_SetBrokenLegs", false );

		m_Health.SetOnlyNumbers( true );
		m_Shock.SetOnlyNumbers( true );
		m_Blood.SetOnlyNumbers( true );
		m_Energy.SetOnlyNumbers( true );
		m_Water.SetOnlyNumbers( true );
		m_Stamina.SetOnlyNumbers( true );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	private Widget InitActionWidgetsPermissions( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_HEADER", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 4, 1 );

		m_ModifyPermissions = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_PERMISSIONS", this, "Click_ModifyPermissions" );
		m_SavePermissions = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_SAVE_PERMISSIONS", this, "Click_SavePermissions" );
		m_PermissionsListScroller = UIActionManager.CreateScroller( actions );
		m_PermissionsListScroller.SetFixedHeight( 400 );
		
		m_ModifyRoles = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_ROLES", this, "Click_ModifyRoles" );
		m_SaveRoles = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_SAVE_ROLES", this, "Click_SaveRoles" );
		m_RolesListScroller = UIActionManager.CreateScroller( actions );
		m_RolesListScroller.SetFixedHeight( 400 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		HidePermissions();
		HideRoles();

		return parent;
	}

	private Widget InitActionWidgetsQuick( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEADER", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 3, 2 );

		m_RepairTransport = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_REPAIR_TRANSPORT", this, "Click_RepairTransport" );
		m_SpectatePlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SPECTATE", this, "Click_SpectatePlayer" );
		m_HealPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEAL", this, "Click_HealPlayer" );
		m_StopBleeding = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", this, "Click_StopBleeding" );
		m_StripPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", this, "Click_StripPlayer" );
		m_DryPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_DRY", this, "Click_DryPlayer" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	void Event_UpdatePlayerList( UIEvent eid, ref UIActionBase action )
	{
		UpdatePlayerList();
	}

	void HidePermissions()
	{
		m_ModifyPermissions.SetButton( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_SHOW_PERMISSIONS" );
		m_SavePermissions.Hide();
		m_PermissionsListScroller.Hide();

		if ( m_PermissionsRows )
			m_PermissionsRows.Unlink();

		m_PermissionList.Clear();

		m_ActionListScroller.UpdateScroller();
	}

	void ShowPermissions()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ShowPermissions" );
		#endif

		m_ModifyPermissions.SetButton( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_HIDE_PERMISSIONS" );
		m_SavePermissions.Show();
		m_PermissionsListScroller.Show();

		array< JMPermission > permissions = new array< JMPermission >();
		GetPermissionsManager().GetPermissionsAsList( permissions );

		int permissionIdx = 0;

		m_PermissionsRows = UIActionManager.CreateActionRows( m_PermissionsListScroller.GetContentWidget() );

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );

		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PermissionsRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			gsw.Show( true );

			for ( int j = 0; j < 100; j++ )
			{
				if ( permissionIdx >= permissions.Count() )
					break;

				permissionIdx++;

				Widget prWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/permission_widget.layout", gsw );
				
				if ( !prWidget )
					continue;

				JMPermissionRowWidget prScript;
				prWidget.GetScript( prScript );

				if ( !prScript )
					continue;

				prScript.InitPermission( permissions[permissionIdx - 1] );
				prScript.Enable();

				prScript.SetType( pi.GetRawPermissionType( permissions[permissionIdx - 1].GetFullName() ) );

				m_PermissionList.Insert( prScript );
			}

			if ( permissionIdx >= permissions.Count() )
				break;
		}		

		m_PermissionsListScroller.UpdateScroller();
		m_ActionListScroller.UpdateScroller();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ShowPermissions" );
		#endif
	}

	void HideRoles()
	{
		m_ModifyRoles.SetButton( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_SHOW_ROLES" );
		m_SaveRoles.Hide();
		m_RolesListScroller.Hide();

		if ( m_RolesRows )
			m_RolesRows.Unlink();

		m_RoleList.Clear();

		m_ActionListScroller.UpdateScroller();
	}

	void ShowRoles()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ShowRoles" );
		#endif

		m_ModifyRoles.SetButton( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_PERMISSIONS_HIDE_ROLES" );
		m_SaveRoles.Show();
		m_RolesListScroller.Show();

		m_RolesRows = UIActionManager.CreateActionRows( m_RolesListScroller.GetContentWidget() );

		GridSpacerWidget parentSpacer;
		int spacerIndex = 0;

		array< JMRole > roles = new array< JMRole >;

		GetPermissionsManager().GetRolesAsList( roles );

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );

		for ( int i = 0; i < 10; i++ )
		{
			if ( !Class.CastTo( parentSpacer, m_RolesRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			parentSpacer.Show( false );
		}

		for ( int j = 0; j < roles.Count(); j++ )
		{
			if ( m_RoleList.Count() % 100 == 0 )
			{
				if ( !Class.CastTo( parentSpacer, m_RolesRows.FindAnyWidget( "Content_Row_0" + spacerIndex ) ) )
					return;

				parentSpacer.Show( true );

				spacerIndex++;
			}

			Widget prWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/role_widget.layout", parentSpacer );

			if ( !prWidget )
				continue;

			JMRoleRowWidget prScript;
			prWidget.GetScript( prScript );

			if ( !prScript )
				continue;

			prScript.Show();
			prScript.InitRole( roles[j].Name );
			prScript.SetChecked( pi.HasRole( roles[j].Name ) );

			if ( roles[j].Name == "everyone" )
			{
				prScript.Disable();
			}

			m_RoleList.Insert( prScript );
		}

		m_RolesListScroller.UpdateScroller();
		m_ActionListScroller.UpdateScroller();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ShowRoles" );
		#endif
	}

	void Click_ModifyPermissions( UIEvent eid, ref UIActionBase action )
	{
		if ( m_PermissionsListScroller.IsVisible() )
		{
			HidePermissions();
		} else
		{
			ShowPermissions();
		}
	}
	
	void Click_ModifyRoles( UIEvent eid, ref UIActionBase action )
	{
		if ( m_RolesListScroller.IsVisible() )
		{
			HideRoles();
		} else
		{
			ShowRoles();
		}
	}

	void Click_SavePermissions( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		array< string > permissions = new array< string >();
		for ( int i = 0; i < m_PermissionList.Count(); ++i )
		{
			permissions.Insert( m_PermissionList[i].FullName + " " + m_PermissionList[i].Type );
		}

		m_Module.SetPermissions( permissions, JM_GetSelected().GetPlayers() );
	}
	
	void Click_SaveRoles( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		array< string > roles = new array< string >();
		for ( int i = 0; i < m_RoleList.Count(); ++i )
		{
			if ( m_RoleList[i].IsChecked() )
				roles.Insert( m_RoleList[i].Name );
		}

		m_Module.SetRoles( roles, JM_GetSelected().GetPlayers() );
	}

	void Click_StripPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Strip( JM_GetSelected().GetPlayers() );
	}

	void Click_DryPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Dry( JM_GetSelected().GetPlayers() );
	}

	void Click_StopBleeding( UIEvent eid, ref UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.StopBleeding( JM_GetSelected().GetPlayers() );
	}

	void Click_HealPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Heal( JM_GetSelected().GetPlayers() );
	}

	void Click_SpectatePlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		bool shouldSpectate = true;

		if ( CurrentActiveCamera )
		{
			JMSpectatorCamera cam = JMSpectatorCamera.Cast( CurrentActiveCamera );
			
			if ( cam )
			{
				shouldSpectate = false;
			}
		}

		if ( shouldSpectate )
		{
			if ( JM_GetSelected().GetPlayers().Count() != 1 )
				return;

			m_Module.StartSpectating( JM_GetSelected().GetPlayers()[0] );
		} else
		{
			m_Module.EndSpectating();
		}
	}

	void Click_RepairTransport( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.RepairTransport( JM_GetSelected().GetPlayers() );
	}

	void Click_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector pos = vector.Zero;
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();

		m_Module.TeleportTo( pos, JM_GetSelected().GetPlayers() );
	}

	void Click_TeleportToMe( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector position = vector.Zero;

		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
		{
			position = CurrentActiveCamera.GetPosition();
		} else if ( GetPlayer() )
		{
			position = GetPlayer().GetPosition();
		} else
		{
			return;
		}

		m_Module.TeleportTo( position, JM_GetSelected().GetPlayers() );
	}

	void Click_TeleportMeTo( UIEvent eid, ref UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportSenderTo( JM_GetSelected().GetPlayers()[0] );
	}

	void Click_TeleportPrevious( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportToPrevious( JM_GetSelected().GetPlayers() );
	}

	void Click_RefreshStats( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RefreshStats();
	}
	
	void Click_RefreshTeleports( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
			
		RefreshTeleports();
	}

	void RefreshStats()
	{
		if ( !m_SelectedInstance )
			return;
		
		if ( m_Health )
			m_Health.SetText( m_SelectedInstance.GetHealth().ToString() );
		
		if ( m_Blood )
			m_Blood.SetText( m_SelectedInstance.GetBlood().ToString() );
		
		if ( m_Energy )
			m_Energy.SetText( m_SelectedInstance.GetEnergy().ToString() );
		
		if ( m_Water )
			m_Water.SetText( m_SelectedInstance.GetWater().ToString() );
		
		if ( m_Shock )
			m_Shock.SetText( m_SelectedInstance.GetShock().ToString() );
		
		if ( m_Stamina )
			m_Stamina.SetText( m_SelectedInstance.GetStamina().ToString() );

		
		if ( m_BloodyHands )
			m_BloodyHands.SetChecked( m_SelectedInstance.HasBloodyHands() );
		
		if ( m_GodMode )
			m_GodMode.SetChecked( m_SelectedInstance.HasGodMode() );
		
		if ( m_Freeze )
			m_Freeze.SetChecked( m_SelectedInstance.IsFrozen() );
		
		if ( m_Invisibility )
			m_Invisibility.SetChecked( m_SelectedInstance.HasInvisibility() );
		
		if ( m_UnlimitedAmmo )
			m_UnlimitedAmmo.SetChecked( m_SelectedInstance.HasUnlimitedAmmo() );
		
		if ( m_UnlimitedStamina )
			m_UnlimitedStamina.SetChecked( m_SelectedInstance.HasUnlimitedStamina() );
		
		if ( m_BrokenLegs )
			m_BrokenLegs.SetChecked( m_SelectedInstance.HasBrokenLegs() );
	}

	void RefreshTeleports()
	{
		if ( !m_SelectedInstance )
			return;

		vector position = m_SelectedInstance.GetPosition();

		m_PositionX.SetText( position[0].ToString() );
		m_PositionY.SetText( position[1].ToString() );
		m_PositionZ.SetText( position[2].ToString() );
	}

	void Click_ApplyStats( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_HealthUpdated )
		{
			m_HealthUpdated = false;

			if ( m_Health )
				m_Module.SetHealth( ToFloat( m_Health.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_BloodUpdated )
		{
			m_BloodUpdated = false;

			if ( m_Blood )
				m_Module.SetBlood( ToFloat( m_Blood.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_EnergyUpdated )
		{
			m_EnergyUpdated = false;

			if ( m_Energy )
				m_Module.SetEnergy( ToFloat( m_Energy.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_WaterUpdated )
		{
			m_WaterUpdated = false;

			if ( m_Water )
				m_Module.SetWater( ToFloat( m_Water.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_ShockUpdated )
		{
			m_ShockUpdated = false;

			if ( m_Shock )
				m_Module.SetShock( ToFloat( m_Shock.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_StaminaUpdated )
		{
			m_StaminaUpdated = false;

			if ( m_Stamina )
				m_Module.SetStamina( ToFloat( m_Stamina.GetText() ), JM_GetSelected().GetPlayers() );
		}
	}

	void Click_SetHealth( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_HealthUpdated = true;
	}

	void Click_SetBlood( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_BloodUpdated = true;
	}

	void Click_SetEnergy( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_EnergyUpdated = true;
	}

	void Click_SetWater( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_WaterUpdated = true;
	}

	void Click_SetShock( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_ShockUpdated = true;
	}

	void Click_SetStamina( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_StaminaUpdated = true;
	}

	void Click_BloodyHands( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetBloodyHands( m_BloodyHands.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_GodMode( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetGodMode( m_GodMode.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_Freeze( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetFreeze( m_Freeze.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_SetBrokenLegs( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetBrokenLegs( m_BrokenLegs.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_Invisible( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetInvisible( m_Invisibility.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_UnlimitedAmmo( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetUnlimitedAmmo( m_UnlimitedAmmo.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void Click_UnlimitedStamina( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetUnlimitedStamina( m_UnlimitedStamina.IsChecked(), JM_GetSelected().GetPlayers() );
	}

	void HideUI()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::HideUI" );
		#endif

		ShowIdentityWidgets();

		m_RightPanelDisable.Show( true );

		m_SelectedInstance = NULL;

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::HideUI" );
		#endif
	}

	void ShowUI()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ShowUI" );
		#endif

		m_RightPanelDisable.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ShowUI" );
		#endif
	}

	void UpdateUI()
	{
		if ( JM_GetSelected().NumPlayers() == 0 )
		{
			HideUI();
			return;
		}

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );
		
		if ( !instance )
		{
			HideUI();
			return;
		}

		if ( IsMissionOffline() )
		{
			instance.Update();
		}

		ShowUI();
		
		if ( JM_GetSelected().NumPlayers() == 1 )
		{
			ShowIdentityWidgets();

			m_GUID.SetText( instance.GetGUID() );
			m_Name.SetText( instance.GetName() );
			m_Steam64ID.SetText( instance.GetSteam64ID() );

			m_TeleportMeTo.Enable();
		} else
		{
			HideIdentityWidgets();

			m_TeleportMeTo.Disable();
		}

		if ( IsMissionOffline() )
		{
			m_TeleportToMe.Disable();
			m_TeleportMeTo.Disable();
			m_TeleportPrevious.Disable();
			m_SpectatePlayer.Disable();
		}

		if ( m_SelectedInstance != instance )
		{
			m_SelectedInstance = instance;

			RefreshStats();

			RefreshTeleports();
		}
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayerList, 1500, true );

		UpdateUI();

		UpdatePlayerList();
	}

	override void OnHide() 
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePlayerList );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		return false;
	}

	void OnPlayer_Checked( string guid, bool checked )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].Checkbox.SetChecked( checked );
			}
		}

		UpdateUI();

		UpdatePlayerCount();
	}

	void OnPlayer_Button( string guid, bool check )
	{
		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( m_PlayerList[i].GetGUID() == guid )
			{
				m_PlayerList[i].Checkbox.SetChecked( check );
			} else
			{
				m_PlayerList[i].Checkbox.SetChecked( false );
			}
		}

		UpdateUI();

		UpdatePlayerCount();
	}

	private void SortPlayersArray( out array< JMPlayerInstance > players )
	{
		string pNames[ 1000 ];
		int pIndices[ 1000 ];

		for ( int i = 0; i < players.Count(); i++ )
		{
			pNames[ i ] = players[ i ].GetName();
		}

		Sort( pNames, players.Count() );

		for ( i = 0; i < players.Count(); i++ )
		{
			for ( int j = 0; j < players.Count(); j++ )
			{
				if ( pNames[ j ] == players[ i ].GetName() )
				{
					pIndices[ i ] = j;
				}
			}
		}

		array< JMPlayerInstance > playersTemp = new array< JMPlayerInstance >;

		for ( i = 0; i < players.Count(); i++ )
		{
			playersTemp.Insert( NULL );
		}

		for ( i = 0; i < players.Count(); i++ )
		{
			playersTemp.Set( pIndices[ i ], players[ i ] );
		}

		players.Clear();
		players.Copy( playersTemp );
	}

	void UpdatePlayerCount()
	{
		m_PlayerListCount.SetText( "" + m_NumPlayerCount + " (" + JM_GetSelected().GetPlayers().Count() + ")" );
	}

	void UpdatePlayerList()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdatePlayerList" );
		#endif

		if ( !IsMissionOffline() )
		{
			GetCommunityOnlineTools().RefreshClients();
		}

		GridSpacerWidget parentSpacer;
		int spacerIndex = 0;
		for ( int i = 0; i < 10; i++ )
		{
			if ( !Class.CastTo( parentSpacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			parentSpacer.Show( false );
		}

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

		if ( m_PlayerListSort.IsChecked() )
			SortPlayersArray( players );

		int idx = 0;
		int pIdx = 0;

		m_NumPlayerCount = 0;

		string filter = m_PlayerListFilter.GetText();
		bool isFiltering = filter.Length() > 0;
		filter.ToLower();

		JMPlayerInstance cPlayer;

		while ( idx < m_PlayerList.Count() )
		{
			if ( pIdx < players.Count() )
			{
				if ( idx % 100 == 0 )
				{
					if ( !Class.CastTo( parentSpacer, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + spacerIndex ) ) )
						return;

					parentSpacer.Show( true );

					spacerIndex++;
				}

				cPlayer = players[pIdx];

				pIdx++;

				string pName = cPlayer.GetName();
				pName.ToLower();

				if ( isFiltering && !pName.Contains( filter ) )
				{
					continue;
				}

				m_PlayerList[idx].SetPlayer( cPlayer.GetGUID() );

				m_NumPlayerCount++;
			} else
			{
				m_PlayerList[idx].SetPlayer( "" );
			}

			idx++;
		}

		UpdatePlayerCount();

		m_PlayerListScroller.UpdateScroller();	

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdatePlayerList" );
		#endif
	}

	override void OnFocus()
	{
		//CF_DumpWidgets( m_LeftPanel );
	}

	override void OnUnfocus()
	{

	}
}