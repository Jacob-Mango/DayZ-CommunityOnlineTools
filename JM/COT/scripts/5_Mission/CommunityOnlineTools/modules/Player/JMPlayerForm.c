class JMPlayerForm extends JMFormBase
{
	private autoptr array< ref JMPlayerRowWidget > m_PlayerList;

	private Widget m_LeftPanel;
	private Widget m_RightPanel;
	private Widget m_RightPanelDisable;

	private UIActionText m_PlayerListCount;
	private UIActionEditableText m_PlayerListFilter;
	private UIActionCheckbox m_PlayerListSort; //TODO: make a new ui action type thing
	private UIActionScroller m_PlayerListScroller;

	private Widget m_PlayerListRows;

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
	private UIActionButton m_ModifyRoles;

	private UIActionButton m_RepairTransport;
	private UIActionButton m_SpectatePlayer;
	private UIActionButton m_HealPlayer;
	private UIActionButton m_StopBleeding;
	private UIActionButton m_StripPlayer;

	private int m_NumPlayerCount;

	void JMPlayerForm()
	{
		m_PlayerList = new array< ref JMPlayerRowWidget >;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Insert( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Insert( OnPlayer_Button );
	}

	void ~JMPlayerForm()
	{
		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
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
	}

	override void OnInit()
	{
		JMESPWidget.playerMenu = this;

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

		UIActionManager.CreateText( parent, "Permissions:", "Modify the permissions and roles" );

		Widget actions = UIActionManager.CreateGridSpacer( parent, 1, 2 );

		m_ModifyPermissions = UIActionManager.CreateButton( actions, "Permissions", this, "Click_ModifyPermissions" );
		m_ModifyRoles = UIActionManager.CreateButton( actions, "Roles", this, "Click_ModifyRoles" );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

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

	void Click_ModifyPermissions()
	{
	}
	
	void Click_ModifyRoles()
	{
	}

	void Click_SetPosition( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMPlayerModule module;
		if ( !Class.CastTo( module, GetModuleManager().GetModule( JMPlayerModule ) )
			return;

		vector pos = "0 0 0";
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();

		module.TeleportTo( pos, GetSelectedPlayers() );
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

		m_PositionX.SetText( data.VPosition[0].ToString() );
		m_PositionY.SetText( data.VPosition[0].ToString() );
		m_PositionZ.SetText( data.VPosition[0].ToString() );

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