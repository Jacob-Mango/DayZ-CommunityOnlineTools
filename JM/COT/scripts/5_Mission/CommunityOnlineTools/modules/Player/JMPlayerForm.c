class JMPlayerForm extends JMFormBase
{
	ref array< ref JMPlayerRowWidget >		m_PlayerList;
	ref array< ref JMPermissionRowWidget >	m_PermissionList;
	ref array< ref JMRoleRowWidget >		m_RolesList;

	ref UIActionScroller m_PlayerListScroller;
	ref Widget m_PlayerListRows;

	ref Widget m_LeftPanel;
	ref Widget m_RightPanel;

	ref TextWidget m_PlayerCount;

	ref Widget m_ActionsWrapper;
	ref Widget m_ActionsForm;

	ref Widget m_PermissionsWrapper;

	ref ButtonWidget m_SetPermissionsButton;
	ref ButtonWidget m_PermissionsBackButton;

	ref ButtonWidget m_ActionsRefresh;

	ref Widget m_RolesWrapper;
	ref Widget m_RolesContainer;
	ref ButtonWidget m_SetRolesButton;
	ref ButtonWidget m_RolesBackButton;

	ref JMPermission m_LoadedPermission;
	ref JMPermissionRowWidget m_PermissionUI;

	ref UIActionText m_GUID;
	ref UIActionText m_Name;
	ref UIActionText m_Steam64ID;

	ref UIActionText m_PosX;
	ref UIActionText m_PosY;
	ref UIActionText m_PosZ;

	ref UIActionText m_PingMin;
	ref UIActionText m_PingMax;
	ref UIActionText m_PingAvg;

	ref UIActionEditableText m_Health;
	ref UIActionEditableText m_Blood;
	ref UIActionEditableText m_Energy;
	ref UIActionEditableText m_Water;
	ref UIActionEditableText m_Shock;
	ref UIActionEditableText m_HeatComfort;
	ref UIActionEditableText m_Wet;
	ref UIActionEditableText m_Tremor;
	ref UIActionEditableText m_Stamina;
	ref UIActionSelectBox m_LastShaved;
	ref UIActionCheckbox m_BloodyHands;

	ref UIActionButton m_KickTransport;
	ref UIActionButton m_RepairTransport;
	ref UIActionButton m_TeleportToMe;
	ref UIActionButton m_TeleportMeTo;
	ref UIActionButton m_TeleportPrevious;

	ref UIActionButton m_ModifyPermissions;
	ref UIActionButton m_ModifyRoles;
	ref UIActionButton m_BanPlayer;
	ref UIActionButton m_KickPlayer;
	ref UIActionCheckbox m_Freecam;
	ref UIActionCheckbox m_GodMode;
	ref UIActionCheckbox m_Invisibility;
	ref UIActionButton m_SpectatePlayer;
	ref UIActionButton m_HealPlayer;
	ref UIActionButton m_StopBleeding;
	ref UIActionButton m_StripPlayer;

	float plwidth = -1;
	float plheight = -1;
	float awidth = -1;
	float aheight = -1;

	private string m_DataUpdateGUID;

	void JMPlayerForm()
	{
		m_PlayerList = new array< ref JMPlayerRowWidget >;
		m_PermissionList = new array< ref JMPermissionRowWidget >;
		m_RolesList = new array< ref JMRoleRowWidget >;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerForm()
	{
		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	override void OnInit()
	{
		JMESPWidget.playerMenu = this;

		m_RightPanel = layoutRoot.FindAnyWidget( "panel_right" );
		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		m_PlayerListScroller = UIActionManager.CreateScroller( m_LeftPanel.FindAnyWidget( "actions_form" ) );

		m_PlayerListRows = m_PlayerListScroller.GetContentWidget();

		m_PlayerCount = TextWidget.Cast( m_LeftPanel.FindAnyWidget("player_count"));

		m_ActionsForm = m_RightPanel.FindAnyWidget("actions_form");
		m_ActionsWrapper = m_ActionsForm.FindAnyWidget("actions_wrapper");
		
		m_GUID = UIActionManager.CreateText( m_ActionsWrapper, "GUID: ", "" );
		m_Name = UIActionManager.CreateText( m_ActionsWrapper, "Name: ", "" );
		m_Steam64ID = UIActionManager.CreateText( m_ActionsWrapper, "Steam64: ", "" );

		ref Widget playerPosition = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );
		m_PosX = UIActionManager.CreateText( playerPosition, "Pos X: ", "" );
		m_PosY = UIActionManager.CreateText( playerPosition, "Pos Y: ", "" );
		m_PosZ = UIActionManager.CreateText( playerPosition, "Pos Z: ", "" );

		ref Widget pings = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );
		m_PingMin = UIActionManager.CreateText( pings, "Ping Min: ", "" );
		m_PingMax = UIActionManager.CreateText( pings, "Ping Max: ", "" );
		m_PingAvg = UIActionManager.CreateText( pings, "Ping Avg: ", "" );
		pings.Show( false );

		ref Widget playerActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 6, 2 );
		m_Health = UIActionManager.CreateEditableText( playerActions, "Health: ", this, "Click_SetHealth", "", "Set" );
		m_Health.SetOnlyNumbers( true );
		m_Shock = UIActionManager.CreateEditableText( playerActions, "Shock: ", this, "Click_SetShock", "", "Set" );
		m_Shock.SetOnlyNumbers( true );
		m_Blood = UIActionManager.CreateEditableText( playerActions, "Blood: ", this, "Click_SetBlood", "", "Set" );
		m_Blood.SetOnlyNumbers( true );
		m_Energy = UIActionManager.CreateEditableText( playerActions, "Energy: ", this, "Click_SetEnergy", "", "Set" );
		m_Energy.SetOnlyNumbers( true );
		m_Water = UIActionManager.CreateEditableText( playerActions, "Water: ", this, "Click_SetWater", "", "Set" );
		m_Water.SetOnlyNumbers( true );
		m_Stamina = UIActionManager.CreateEditableText( playerActions, "Stamina: ", this, "Click_SetStamina", "", "Set" );
		m_Stamina.SetOnlyNumbers( true );
		m_BloodyHands = UIActionManager.CreateCheckbox( playerActions, "Bloody Hands: ", this, "Click_SetBloodyHands", false );
		m_RepairTransport = UIActionManager.CreateButton( playerActions, "Repair Transport", this, "Click_RepairTransport" );
		m_TeleportToMe = UIActionManager.CreateButton( playerActions, "Teleport To Me", this, "Click_TeleportToMe" );
		m_TeleportMeTo = UIActionManager.CreateButton( playerActions, "Teleport Me To", this, "Click_TeleportMeTo" );
		m_TeleportPrevious = UIActionManager.CreateButton( playerActions, "Teleport Previous", this, "Click_TeleportPrevious" );

		ref Widget serverActions = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 4, 2 );
		m_ModifyPermissions = UIActionManager.CreateButton( serverActions, "Modify Permissions", this, "Click_ModifyPermissions" );
		m_ModifyRoles = UIActionManager.CreateButton( serverActions, "Modify Roles", this, "Click_ModifyRoles" );
		m_GodMode = UIActionManager.CreateCheckbox( serverActions, "Godmode", this, "Click_SetGodMode", false );
		m_SpectatePlayer = UIActionManager.CreateButton( serverActions, "Spectate", this, "Click_SpectatePlayer" );
		m_HealPlayer = UIActionManager.CreateButton( serverActions, "Heal", this, "Click_HealPlayer" );
		m_StopBleeding = UIActionManager.CreateButton( serverActions, "Stop Bleeding", this, "Click_StopBleeding" );
		m_StripPlayer = UIActionManager.CreateButton( serverActions, "Strip", this, "Click_StripPlayer" );

		m_PermissionsWrapper = m_RightPanel.FindAnyWidget("permissions_wrapper");

		m_SetPermissionsButton = ButtonWidget.Cast( m_RightPanel.FindAnyWidget("permissions_set_button"));
		m_PermissionsBackButton = ButtonWidget.Cast( m_RightPanel.FindAnyWidget("permissions_back_button"));
		
		m_RolesWrapper = m_RightPanel.FindAnyWidget("roles_wrapper");
		m_RolesContainer = m_RightPanel.FindAnyWidget("roles_container");
		m_SetRolesButton = ButtonWidget.Cast( m_RightPanel.FindAnyWidget("roles_set_button"));
		m_RolesBackButton = ButtonWidget.Cast( m_RightPanel.FindAnyWidget("roles_back_button"));

		m_ActionsRefresh = ButtonWidget.Cast( m_RightPanel.FindAnyWidget("actions_refresh_button"));

		layoutRoot.GetSize( awidth, aheight );

		m_LeftPanel.GetSize( plwidth, plheight );

		plwidth = plwidth * awidth;
		plheight = plheight * aheight;

		CreatePermissionsUI();
		CreateRolesUI();
		CreatePlayerList();
	}

	void Click_ModifyPermissions()
	{
		m_RolesWrapper.Show( false );
		m_ActionsForm.Show( false );
		m_PermissionsWrapper.Show( true );
	}
	
	void Click_ModifyRoles()
	{
		RefreshRolesUI();

		m_ActionsForm.Show( false );
		m_RolesWrapper.Show( true );
		m_PermissionsWrapper.Show( false );
	}

	void Click_StripPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.Strip( GetSelectedPlayers() );
	}

	void Click_StopBleeding( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.StopBleeding( GetSelectedPlayers() );
	}

	void Click_HealPlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.Heal( GetSelectedPlayers() );
	}

	void Click_SpectatePlayer( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
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

			module.StartSpectating( GetSelectedPlayers()[0] );
		} else
		{
			module.EndSpectating();
		}
	}

	void Click_RepairTransport( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.RepairTransport( GetSelectedPlayers() );
	}
	
	void Click_TeleportToMe( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
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

		module.TeleportTo( position, GetSelectedPlayers() );
	}

	void Click_TeleportMeTo( UIEvent eid, ref UIActionBase action )
	{
		if ( GetSelectedPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.TeleportSenderTo( GetSelectedPlayers()[0] );
	}

	void Click_TeleportPrevious( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.TeleportToPrevious( GetSelectedPlayers() );
	}

	void Click_SetHealth( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetHealth( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetShock( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetShock( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetBlood( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetBlood( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetEnergy( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetEnergy( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetWater( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetWater( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetStamina( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetStamina( ToFloat( action.GetText() ), GetSelectedPlayers() );
	}

	void Click_SetBloodyHands( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetBloodyHands( action.IsChecked(), GetSelectedPlayers() );
	}

	void Click_SetGodMode( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetGodMode( action.IsChecked(), GetSelectedPlayers() );
	}

	void HideSide()
	{
		LoadPermissions( NULL );
		LoadRoles( NULL );

		SetSize( plwidth, plheight );
	}

	void ShowSide()
	{
		if ( CountPlayersSelected() == 0 )
		{
			m_DataUpdateGUID = "";
			HideSide();
			return;
		}

		m_DataUpdateGUID = GetSelectedPlayers()[0];

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( m_DataUpdateGUID );
		
		if ( !instance )
		{
			m_DataUpdateGUID = "";
			HideSide();
			return;
		}

		SetSize( awidth, plheight );

		UpdateActionsFields( instance );
	}

	void UpdateActionsFields( JMPlayerInstance instanceUpdateActionsFields )
	{
		JMPlayerInformation data = instanceUpdateActionsFields.Data;
		
		LoadPermissions( data.APermissions );
		LoadRoles( data.ARoles );

		m_GUID.SetText( data.SGUID );
		m_Name.SetText( data.SName );
		m_Steam64ID.SetText( data.SSteam64ID );

		m_Health.SetText( data.FHealth.ToString() );
		m_Blood.SetText( data.FBlood.ToString() );
		m_Energy.SetText( data.FEnergy.ToString() );
		m_Water.SetText( data.FWater.ToString() );
		m_Shock.SetText( data.FShock.ToString() );
		//m_HeatComfort.SetText( data.FHeatComfort.ToString() );
		//m_Wet.SetText( data.FWet.ToString() );
		//m_Tremor.SetText( data.FTremor.ToString() );
		m_Stamina.SetText( data.FStamina.ToString() );
		//m_LastShaved.SetSelection( data.ILifeSpanState );
		m_BloodyHands.SetChecked( data.BBloodyHands );
		m_GodMode.SetChecked( data.BGodMode );
		//m_Invisibility.SetChecked(data.BInvisibility);
		
		m_PosX.SetText( "" + data.VPosition[0] );
		m_PosY.SetText( "" + data.VPosition[1] );
		m_PosZ.SetText( "" + data.VPosition[2] );

		m_PingMin.SetText( data.IPingMin.ToString() );
		m_PingMax.SetText( data.IPingMax.ToString() );
		m_PingAvg.SetText( data.IPingAvg.ToString() );

		m_ActionsForm.FindAnyWidget( "disabled" ).Show( false );
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayerList, 1500, true );

		m_PermissionsWrapper.Show( false );
		m_RolesWrapper.Show( false );
		m_ActionsWrapper.Show( true );
		
		ShowSide();

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

		if ( w == m_SetPermissionsButton )
		{
			SetPermissions();
			return true;
		}

		if ( w == m_SetRolesButton )
		{
			SetRoles();
			return true;
		}

		if ( w == m_ActionsRefresh )
		{
			ShowSide();
			return true;
		}

		if ( w == m_PermissionsBackButton )
		{
			m_PermissionsWrapper.Show( false );
			m_RolesWrapper.Show( false );
			m_ActionsForm.Show( true );
			return true;
		}
		
		if ( w == m_RolesBackButton )
		{
			m_PermissionsWrapper.Show( false );
			m_RolesWrapper.Show( false );
			m_ActionsForm.Show( true );
			return true;
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

		if ( PlayerSelectedIndex( guid ) == 0 )
		{
			ShowSide();
		}

		if ( CountPlayersSelected() == 0 )
		{
			m_DataUpdateGUID = "";

			HideSide();
		}
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

		if ( PlayerSelectedIndex( guid ) == 0 )
		{
			ShowSide();
		}

		if ( CountPlayersSelected() == 0 )
		{
			m_DataUpdateGUID = "";

			HideSide();
		}
	}

	void CreatePermissionsUI()
	{
		JMPermission rootPerm = GetPermissionsManager().GetRootPermission();

		Widget permRow = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/permission_widget.layout", m_PermissionsWrapper.FindAnyWidget( "list_00" ) );

		permRow.GetScript( m_PermissionUI );

		permRow.Show( false );
		permRow.SetSize( 0, 0 );

		if ( m_PermissionUI )
		{
			m_PermissionList.Clear();

			m_PermissionUI.InitPermission( rootPerm, 0 );

			CreatePermissionUIRow( rootPerm, 0, m_PermissionUI );
		}
	}

	void ResetPermissionUI()
	{
		m_PermissionUI.Disable();
	}

	ref array< string > SerializePermissionUI()
	{
		ref array< string > output = new array< string >;
		m_PermissionUI.Serialize( output );
		return output;
	}

	private void CreatePermissionUIRow( ref JMPermission perm, int depth, ref JMPermissionRowWidget parentRow )
	{
		for ( int i = 0; i < perm.Children.Count(); i++ )
		{
			Widget parentWidget = m_PermissionsWrapper.FindAnyWidget( "list_0" + Math.Floor( m_PermissionList.Count() / 100 ) );

			Widget permRow = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/permission_widget.layout", parentWidget );

			JMPermissionRowWidget rowScript;
			permRow.GetScript( rowScript );

			if ( rowScript )
			{
				m_PermissionList.Insert( rowScript );
				rowScript.InitPermission( perm.Children[i], depth );

				parentRow.Children.Insert( rowScript );
				rowScript.Parent = parentRow;

				CreatePermissionUIRow( perm.Children[i], depth + 1, rowScript );
			}
		}
	}

	void LoadPermissions( ref array< string > permissions = NULL )
	{
		if ( permissions == NULL )
		{
			for ( int i = 0; i < m_PermissionList.Count(); i++ )
			{
				m_PermissionList[i].Disable();
			}
		} else 
		{
			for ( int j = 0; j < permissions.Count(); j++ )
			{
				m_PermissionUI.SetPermission( permissions[j] );
			}

			for ( int k = 0; k < m_PermissionList.Count(); k++ )
			{
				m_PermissionList[k].Enable();
			}
		}
	}

	void SetPermissions()
	{
		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetPermissions( SerializePermissionUI(), GetSelectedPlayers() );
	}

	void CreateRolesUI()
	{
		for ( int i = 0; i < 100; i++ )
		{
			CreateRoleUIRow();
		}

		RefreshRolesUI();
	}

	private void CreateRoleUIRow()
	{
		Widget permRow = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/role_widget.layout", m_RolesContainer );

		ref JMRoleRowWidget rowScript;
		permRow.GetScript( rowScript );

		if ( rowScript )
		{
			m_RolesList.Insert( rowScript );
		}
	}

	array< string > SerializeRolesUI()
	{
		array< string > output = new array< string >;
		for ( int i = 0; i < m_RolesList.Count(); i++ )
		{
			if ( m_RolesList[i].IsChecked() )
			{
				output.Insert( m_RolesList[i].Name );
			}
		}
		return output;
	}

	void RefreshRolesUI()
	{
		if ( m_RolesWrapper.IsVisible() )
		{
			return;
		}
		
		for ( int i = 0; i < 100; i++ )
		{
			m_RolesList[i].Hide();
		}

		for ( int j = 0; j < GetPermissionsManager().Roles.Count(); j++ )
		{
			string role = GetPermissionsManager().Roles.GetKey( j );
			if ( role == "everyone" )
				continue;

			m_RolesList[j].Show();
			m_RolesList[j].InitRole( role );
		}
	}

	void LoadRoles( ref array< string > roles = NULL )
	{
		if ( m_RolesWrapper.IsVisible() )
		{
			return;
		}

		for ( int k = 0; k < m_RolesList.Count(); k++ )
		{
			m_RolesList[k].SetChecked( false );
		}

		if ( roles != NULL )
		{
			for ( int j = 0; j < roles.Count(); j++ )
			{
				for ( int i = 0; i < m_RolesList.Count(); i++ )
				{
					if ( m_RolesList[i].Name == roles[j] )
					{
						m_RolesList[i].SetChecked( true );
					}
				}
			}
		}
	}

	void SetRoles()
	{
		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		module.SetRoles( SerializeRolesUI(), GetSelectedPlayers() );
	}

	void CreatePlayerList()
	{
		ref Widget rwWidget = NULL;
		ref JMPlayerRowWidget rwScript = NULL;

		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw = GridSpacerWidget.Cast( m_PlayerListRows.FindAnyWidget( "Content_Row_0" + i ) );
			
			if ( !gsw )
				continue;

			for ( int j = 0; j < 100; j++ )
			{
				rwWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/player_widget.layout", gsw );
				
				if ( rwWidget == NULL )
				{
					continue;
				}

				rwWidget.GetScript( rwScript );

				if ( rwScript == NULL )
				{
					continue;
				}

				rwScript.SetPlayer( "" );

				rwScript.Menu = this;

				m_PlayerList.Insert( rwScript );
			}
		}		
	}

	void UpdatePlayerList()
	{
		GetCommunityOnlineTools().RefreshClients();

		//if ( COT_IsUsingTestSort() )
		//	GetPermissionsManager().SortPlayersArray();

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

		for ( int i = 0; i < m_PlayerList.Count(); i++ )
		{
			if ( i < players.Count() )
			{
				m_PlayerList[i].SetPlayer( players[i].GetGUID() );
			} else
			{
				m_PlayerList[i].SetPlayer( "" );
			}
		}

		int playerCount = players.Count();

		if ( playerCount == 1 )
		{
		//	m_PlayerCount.SetText( "1 Player" );
		} else 
		{
		//	m_PlayerCount.SetText( "" + playerCount + " Players" );
		}
	}

	override void OnFocus()
	{
		//CF_DumpWidgets( m_LeftPanel );
	}

	override void OnUnfocus()
	{

	}
}