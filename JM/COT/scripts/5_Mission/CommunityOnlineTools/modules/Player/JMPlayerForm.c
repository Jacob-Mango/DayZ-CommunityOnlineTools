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

	private UIActionEditableText m_PositionX;
	private UIActionEditableText m_PositionY;
	private UIActionEditableText m_PositionZ;
	private UIActionButton m_Position;

	private UIActionEditableText m_Health;
	private UIActionEditableText m_Blood;
	private UIActionEditableText m_Energy;
	private UIActionEditableText m_Water;
	private UIActionEditableText m_Shock;
	private UIActionEditableText m_Stamina;
	private UIActionCheckbox m_BloodyHands;
	private UIActionCheckbox m_GodMode;

	private UIActionButton m_TeleportToMe;
	private UIActionButton m_TeleportMeTo;
	private UIActionButton m_TeleportPrevious;

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
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 5, 1 );

		UIActionManager.CreateText( parent, "Position: ", "The world position of the player" );

		Widget positionActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget positionActionsVec = UIActionManager.CreateGridSpacer( positionActions, 1, 3 );

		m_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:" );
		m_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:" );
		m_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:" );

		m_PositionX.SetOnlyNumbers( true );
		m_PositionY.SetOnlyNumbers( true );
		m_PositionZ.SetOnlyNumbers( true );
		
		m_Position = UIActionManager.CreateButton( positionActions, "Set Position", this, "Click_SetPosition" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		UIActionManager.CreateText( parent, "Teleporting: ", "" );

		Widget teleportActions = UIActionManager.CreateGridSpacer( parent, 1, 3 );
		m_TeleportToMe = UIActionManager.CreateButton( teleportActions, "Teleport To Me", this, "Click_TeleportToMe" );
		m_TeleportMeTo = UIActionManager.CreateButton( teleportActions, "Teleport Me To", this, "Click_TeleportMeTo" );
		m_TeleportPrevious = UIActionManager.CreateButton( teleportActions, "Teleport Previous", this, "Click_TeleportPrevious" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	private Widget InitActionWidgetsStats( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );

		UIActionManager.CreateText( parent, "Player Variables: ", "" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 4, 2 );

		m_Health = UIActionManager.CreateEditableText( actions, "Health: ", this, "Click_SetHealth", "", "Set" );
		m_Shock = UIActionManager.CreateEditableText( actions, "Shock: ", this, "Click_SetShock", "", "Set" );
		m_Blood = UIActionManager.CreateEditableText( actions, "Blood: ", this, "Click_SetBlood", "", "Set" );
		m_Energy = UIActionManager.CreateEditableText( actions, "Food: ", this, "Click_SetEnergy", "", "Set" );
		m_Water = UIActionManager.CreateEditableText( actions, "Water: ", this, "Click_SetWater", "", "Set" );
		m_Stamina = UIActionManager.CreateEditableText( actions, "Stamina: ", this, "Click_SetStamina", "", "Set" );

		m_BloodyHands = UIActionManager.CreateCheckbox( actions, "Bloody Hands: ", this, "Click_SetBloodyHands", false );
		m_GodMode = UIActionManager.CreateCheckbox( actions, "Godmode: ", this, "Click_SetGodMode", false );

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

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( GetSelectedPlayers()[0] );

		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw;
			if ( !Class.CastTo( gsw, m_PermissionsRows.FindAnyWidget( "Content_Row_0" + i ) ) )
				continue;

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

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( GetSelectedPlayers()[0] );

		for ( int j = 0; j < roles.Count(); j++ )
		{
			if ( m_RoleList.Count() % 100 == 0 )
			{
				if ( !Class.CastTo( parentSpacer, m_RolesRows.FindAnyWidget( "Content_Row_0" + spacerIndex ) ) )
					return;

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

		m_Module.SetPermissions( permissions, GetSelectedPlayers() );
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

		m_Module.SetRoles( roles, GetSelectedPlayers() );
	}

	void Click_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector pos = "0 0 0";
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();

		m_Module.TeleportTo( pos, GetSelectedPlayers() );
	}

	void Click_StripPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Strip( GetSelectedPlayers() );
	}

	void Click_StopBleeding( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.StopBleeding( GetSelectedPlayers() );
	}

	void Click_HealPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Heal( GetSelectedPlayers() );
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
			if ( GetSelectedPlayers().Count() != 1 )
				return;

			m_Module.StartSpectating( GetSelectedPlayers()[0] );
		} else
		{
			m_Module.EndSpectating();
		}
	}

	void Click_RepairTransport( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.RepairTransport( GetSelectedPlayers() );
	}
	
	void Click_TeleportToMe( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector position = "0 0 0";

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

		m_Module.TeleportTo( position, GetSelectedPlayers() );
	}

	void Click_TeleportMeTo( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportSenderTo( GetSelectedPlayers()[0] );
	}

	void Click_TeleportPrevious( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportToPrevious( GetSelectedPlayers() );
	}

	void Click_SetHealth( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetHealth( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetShock( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetShock( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetBlood( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetBlood( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetEnergy( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetEnergy( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetWater( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetWater( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetStamina( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetStamina( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetBloodyHands( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetBloodyHands( action.IsChecked(), GetSelectedPlayers() );
	}

	void Click_SetGodMode( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.SetGodMode( action.IsChecked(), GetSelectedPlayers() );
	}

	void HideUI()
	{
		ShowIdentityWidgets();

		m_RightPanelDisable.Show( true );
	}

	void ShowUI()
	{
		m_RightPanelDisable.Show( false );
	}

	void UpdateUI()
	{
		if ( CountPlayersSelected() == 0 )
		{
			HideUI();
			return;
		}

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( GetSelectedPlayers()[0] );
		
		if ( !instance )
		{
			HideUI();
			return;
		}

		JMPlayerInformation data = instance.Data;

		if ( !data )
		{
			HideUI();
			return;
		}

		ShowUI();
		
		if ( CountPlayersSelected() == 1 )
		{
			ShowIdentityWidgets();

			m_GUID.SetText( data.SGUID );
			m_Name.SetText( data.SName );
			m_Steam64ID.SetText( data.SSteam64ID );

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

		m_PositionX.SetText( data.VPosition[0].ToString() );
		m_PositionY.SetText( data.VPosition[1].ToString() );
		m_PositionZ.SetText( data.VPosition[2].ToString() );

		m_Health.SetText( data.FHealth.ToString() );
		m_Blood.SetText( data.FBlood.ToString() );
		m_Energy.SetText( data.FEnergy.ToString() );
		m_Water.SetText( data.FWater.ToString() );
		m_Shock.SetText( data.FShock.ToString() );
		m_Stamina.SetText( data.FStamina.ToString() );

		m_BloodyHands.SetChecked( data.BBloodyHands );
		m_GodMode.SetChecked( data.BGodMode );
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
		m_PlayerListCount.SetText( "" + m_NumPlayerCount + " (" + GetSelectedPlayers().Count() + ")" );
	}

	void UpdatePlayerList()
	{
		if ( !IsMissionOffline() )
		{
			GetCommunityOnlineTools().RefreshClients();
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