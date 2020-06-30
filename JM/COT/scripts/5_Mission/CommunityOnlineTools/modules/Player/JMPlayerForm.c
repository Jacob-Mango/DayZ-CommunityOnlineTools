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
	private bool m_BloodyHandsUpdated;
	private UIActionCheckbox m_GodMode;
	private bool m_GodModeUpdated;

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

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		m_HealPlayer.UpdatePermission( "Admin.Player.Set" );
		m_Health.UpdatePermission( "Admin.Player.Set.Health" );
		m_Shock.UpdatePermission( "Admin.Player.Set.Shock" );
		m_Blood.UpdatePermission( "Admin.Player.Set.Blood" );
		m_Energy.UpdatePermission( "Admin.Player.Set.Energy" );
		m_Water.UpdatePermission( "Admin.Player.Set.Water" );
		m_Stamina.UpdatePermission( "Admin.Player.Set.Stamina" );
		m_BloodyHands.UpdatePermission( "Admin.Player.Set.BloodyHands" );

		m_ModifyPermissions.UpdatePermission( "Admin.Player.Permissions" );
		m_ModifyRoles.UpdatePermission( "Admin.Player.Roles" );

		m_GodMode.UpdatePermission( "Admin.Player.Godmode" );
		m_SpectatePlayer.UpdatePermission( "Admin.Player.StartSpectating" );
		m_StopBleeding.UpdatePermission( "Admin.Player.StopBleeding" );
		m_StripPlayer.UpdatePermission( "Admin.Player.Strip" );
		
		m_PositionX.UpdatePermission( "Admin.Player.Teleport.Position" );
		m_PositionY.UpdatePermission( "Admin.Player.Teleport.Position" );
		m_PositionZ.UpdatePermission( "Admin.Player.Teleport.Position" );
		m_Position.UpdatePermission( "Admin.Player.Teleport.Position" );

		m_TeleportToMe.UpdatePermission( "Admin.Player.Teleport.Position" );
		m_TeleportMeTo.UpdatePermission( "Admin.Player.Teleport.SenderTo" );
		m_TeleportPrevious.UpdatePermission( "Admin.Player.Teleport.Previous" );

		m_RepairTransport.UpdatePermission( "Admin.Transport.Repair" );

		UpdateUI();
	}

	override void OnInit()
	{
		InitWidgetsLeft();
		InitWidgetsRight();
	}

	private void InitWidgetsLeft()
	{
		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		Widget leftPanelGrid = UIActionManager.CreateGridSpacer( m_LeftPanel, 4, 1 );

		m_PlayerListCount = UIActionManager.CreateText( leftPanelGrid, "Player Count: ", "" );
		m_PlayerListFilter = UIActionManager.CreateEditableText( leftPanelGrid, "Filter: ", this, "Event_UpdatePlayerList" );
		m_PlayerListSort = UIActionManager.CreateCheckbox( leftPanelGrid, "Sort: ", this, "Event_UpdatePlayerList" );
		m_PlayerListScroller = UIActionManager.CreateScroller( leftPanelGrid );
		m_PlayerListRows = UIActionManager.CreateActionRows( m_PlayerListScroller.GetContentWidget() );
		
		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

			gsw.Show( false );

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
		InitActionWidgetsPermissions( m_ActionsWrapper );
		InitActionWidgetsQuick( m_ActionsWrapper );

		m_ActionListScroller.UpdateScroller();
	}

	private Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		m_IdentityHeading = UIActionManager.CreateText( parent, "Identity: ", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 3, 1 );

		m_Name = UIActionManager.CreateText( actions, "Name: ", "" );
		m_GUID = UIActionManager.CreateText( actions, "GUID: ", "" );
		m_Steam64ID = UIActionManager.CreateText( actions, "Steam: ", "" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		ShowIdentityWidgets();

		return parent;
	}

	private void ShowIdentityWidgets()
	{
		m_IdentityHeading.SetText( "The identifying information for the selected player" );

		m_Name.Show();
		m_GUID.Show();
		m_Steam64ID.Show();
	}

	private void HideIdentityWidgets()
	{
		m_IdentityHeading.SetText( "Can only see when 1 player is selected" );

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
		m_PositionRefresh = UIActionManager.CreateButton( positionActionsBut, "Refresh Coordinates", this, "Click_RefreshTeleports" );
		m_Position = UIActionManager.CreateButton( positionActionsBut, "Telport to Coordinates", this, "Click_SetPosition" );

		Widget teleportActions = UIActionManager.CreateGridSpacer( parent, 1, 3 );
		m_TeleportToMe = UIActionManager.CreateButton( teleportActions, "Teleport To Me", this, "Click_TeleportToMe" );
		m_TeleportMeTo = UIActionManager.CreateButton( teleportActions, "Teleport Me To", this, "Click_TeleportMeTo" );
		m_TeleportPrevious = UIActionManager.CreateButton( teleportActions, "Teleport Previous", this, "Click_TeleportPrevious" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	private Widget InitActionWidgetsStats( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		Widget header = UIActionManager.CreateGridSpacer( parent, 1, 2 );
		UIActionManager.CreateText( header, "Player Variables: ", "" );
		m_RefreshStats = UIActionManager.CreateButton( header, "Refresh", this, "Click_RefreshStats" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 4, 2 );

		m_Health = UIActionManager.CreateEditableText( actions, "Health: ", this, "Click_SetHealth", "", "" );
		m_Shock = UIActionManager.CreateEditableText( actions, "Shock: ", this, "Click_SetShock", "", "" );
		m_Blood = UIActionManager.CreateEditableText( actions, "Blood: ", this, "Click_SetBlood", "", "" );
		m_Energy = UIActionManager.CreateEditableText( actions, "Food: ", this, "Click_SetEnergy", "", "" );
		m_Water = UIActionManager.CreateEditableText( actions, "Water: ", this, "Click_SetWater", "", "" );
		m_Stamina = UIActionManager.CreateEditableText( actions, "Stamina: ", this, "Click_SetStamina", "", "" );

		m_BloodyHands = UIActionManager.CreateCheckbox( actions, "Bloody Hands: ", this, "Click_SetBloodyHands", false );
		m_GodMode = UIActionManager.CreateCheckbox( actions, "Godmode: ", this, "Click_SetGodMode", false );

		m_ApplyStats = UIActionManager.CreateButton( parent, "Apply", this, "Click_ApplyStats" );

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

		UIActionManager.CreateText( parent, "Permission Management:", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 4, 1 );

		m_ModifyPermissions = UIActionManager.CreateButton( actions, "Permissions", this, "Click_ModifyPermissions" );
		m_SavePermissions = UIActionManager.CreateButton( actions, "Save Permissions", this, "Click_SavePermissions" );
		m_PermissionsListScroller = UIActionManager.CreateScroller( actions );
		m_PermissionsListScroller.SetFixedHeight( 400 );
		
		m_ModifyRoles = UIActionManager.CreateButton( actions, "Roles", this, "Click_ModifyRoles" );
		m_SaveRoles = UIActionManager.CreateButton( actions, "Save Roles", this, "Click_SaveRoles" );
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

		UIActionManager.CreateText( parent, "Quick Actions:", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 3, 2 );

		m_RepairTransport = UIActionManager.CreateButton( actions, "Repair Transport", this, "Click_RepairTransport" );
		m_SpectatePlayer = UIActionManager.CreateButton( actions, "Spectate", this, "Click_SpectatePlayer" );
		m_HealPlayer = UIActionManager.CreateButton( actions, "Heal", this, "Click_HealPlayer" );
		m_StopBleeding = UIActionManager.CreateButton( actions, "Stop Bleeding", this, "Click_StopBleeding" );
		m_StripPlayer = UIActionManager.CreateButton( actions, "Clear Inventory", this, "Click_StripPlayer" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	void Event_UpdatePlayerList( UIEvent eid, ref UIActionBase action )
	{
		UpdatePlayerList();
	}

	void HidePermissions()
	{
		m_ModifyPermissions.SetButton( "Show Permissions" );
		m_SavePermissions.Hide();
		m_PermissionsListScroller.Hide();

		if ( m_PermissionsRows )
			m_PermissionsRows.Unlink();

		m_PermissionList.Clear();

		m_ActionListScroller.UpdateScroller();
	}

	void ShowPermissions()
	{
		m_ModifyPermissions.SetButton( "Hide Permissions" );
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
	}

	void HideRoles()
	{
		m_ModifyRoles.SetButton( "Show Roles" );
		m_SaveRoles.Hide();
		m_RolesListScroller.Hide();

		if ( m_RolesRows )
			m_RolesRows.Unlink();

		m_RoleList.Clear();

		m_ActionListScroller.UpdateScroller();
	}

	void ShowRoles()
	{
		m_ModifyRoles.SetButton( "Hide Roles" );
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

		m_HealthUpdated = false;
		m_BloodUpdated = false;
		m_EnergyUpdated = false;
		m_WaterUpdated = false;
		m_ShockUpdated = false;
		m_StaminaUpdated = false;
		m_BloodyHandsUpdated = false;
		m_GodModeUpdated = false;
		
		m_Health.SetText( m_SelectedInstance.GetHealth().ToString() );
		m_Blood.SetText( m_SelectedInstance.GetBlood().ToString() );
		m_Energy.SetText( m_SelectedInstance.GetEnergy().ToString() );
		m_Water.SetText( m_SelectedInstance.GetWater().ToString() );
		m_Shock.SetText( m_SelectedInstance.GetShock().ToString() );
		m_Stamina.SetText( m_SelectedInstance.GetStamina().ToString() );

		m_BloodyHands.SetChecked( m_SelectedInstance.HasBloodyHands() );
		m_GodMode.SetChecked( m_SelectedInstance.HasGodMode() );
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

			m_Module.SetHealth( ToFloat( m_Health.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_BloodUpdated )
		{
			m_BloodUpdated = false;

			m_Module.SetBlood( ToFloat( m_Blood.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_EnergyUpdated )
		{
			m_EnergyUpdated = false;

			m_Module.SetEnergy( ToFloat( m_Energy.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_WaterUpdated )
		{
			m_WaterUpdated = false;

			m_Module.SetWater( ToFloat( m_Water.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_ShockUpdated )
		{
			m_ShockUpdated = false;

			m_Module.SetShock( ToFloat( m_Shock.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_StaminaUpdated )
		{
			m_StaminaUpdated = false;

			m_Module.SetStamina( ToFloat( m_Stamina.GetText() ), JM_GetSelected().GetPlayers() );
		}

		if ( m_BloodyHandsUpdated )
		{
			m_BloodyHandsUpdated = false;

			m_Module.SetBloodyHands( m_BloodyHands.IsChecked(), JM_GetSelected().GetPlayers() );
		}

		if ( m_GodModeUpdated )
		{
			m_GodModeUpdated = false;

			m_Module.SetGodMode( m_GodMode.IsChecked(), JM_GetSelected().GetPlayers() );
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
		if ( eid != UIEvent.CLICK )
			return;

		m_BloodUpdated = false;
	}

	void Click_SetEnergy( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_EnergyUpdated = false;
	}

	void Click_SetWater( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_WaterUpdated = false;
	}

	void Click_SetShock( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_ShockUpdated = false;
	}

	void Click_SetStamina( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_StaminaUpdated = false;
	}

	void Click_SetBloodyHands( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_BloodyHandsUpdated = true;
	}

	void Click_SetGodMode( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_GodModeUpdated = true;
	}

	void HideUI()
	{
		ShowIdentityWidgets();

		m_RightPanelDisable.Show( true );

		m_SelectedInstance = NULL;
	}

	void ShowUI()
	{
		m_RightPanelDisable.Show( false );
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
	}

	override void OnFocus()
	{
		//CF_DumpWidgets( m_LeftPanel );
	}

	override void OnUnfocus()
	{

	}
}