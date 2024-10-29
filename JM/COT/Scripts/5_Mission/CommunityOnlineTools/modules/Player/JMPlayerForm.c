class JMPlayerForm: JMFormBase
{
	private autoptr array< JMPlayerRowWidget > m_PlayerList;
	private autoptr array< JMPermissionRowWidget > m_PermissionList;
	private autoptr array< JMRoleRowWidget > m_RoleList;
	private ref JMPermission m_RootPermission;

	private Widget m_LeftPanel;
	private Widget m_RightPanel;
	private Widget m_RightPanelDisable;

	private UIActionText m_PlayerListCount;
	private UIActionEditableTextPreview m_PlayerListFilter;

	private UIActionButtonToggle m_PlayerListSort;
	private UIActionButton m_PlayerListSelectAll;
	private UIActionButton m_PlayerListDeSelectAll;

	private UIActionButton m_PlayerPrefSave;
	private UIActionButton m_PlayerPrefLoad;

	private UIActionScroller m_PlayerListScroller;
	private Widget m_PlayerListRows;

	private UIActionScroller m_PermissionsListScroller;
	private Widget m_PermissionsRows;

	private UIActionScroller m_RolesListScroller;
	private Widget m_RolesRows;

	private UIActionScroller m_ActionListScroller;
	private Widget m_ActionsWrapper;

	private UIActionButton m_GUID;
	private UIActionButton m_Name;
	private UIActionButton m_Steam64ID;
	private UIActionButton m_SteamProfile;

	private UIActionButton m_RefreshStats;
	private UIActionButton m_ApplyStats;
	private UIActionSlider m_Health;
	private bool m_HealthUpdated;
	private UIActionSlider m_Blood;
	private bool m_BloodUpdated;
	private UIActionSlider m_Energy;
	private bool m_EnergyUpdated;
	private UIActionSlider m_Water;
	private bool m_WaterUpdated;
	private UIActionSlider m_Shock;
	private bool m_ShockUpdated;
	private UIActionSlider m_Stamina;
	private bool m_StaminaUpdated;
	private UIActionSlider m_HeatComfort;
	private UIActionSlider m_HeatBuffer;
	private bool m_HeatBufferUpdated;
	private UIActionCheckbox m_BloodyHands;
	private UIActionCheckbox m_GodMode;
	private UIActionCheckbox m_Freeze;
	private UIActionCheckbox m_Invisibility;
	private UIActionCheckbox m_UnlimitedAmmo;
	private UIActionCheckbox m_AdminNVG;
	private UIActionCheckbox m_UnlimitedStamina;
	private UIActionCheckbox m_BrokenLegs;
	private UIActionCheckbox m_ReceiveDmgDealt;
	private UIActionCheckbox m_CannotBeTargetedByAI;
	private UIActionCheckbox m_RemoveCollision;

    private UIActionButton m_CopyRotationPlayer;
    private UIActionButton m_CopyPositionPlayer;
    private UIActionButton m_PastePositionPlayer;
	private UIActionButton m_TeleportToMe;
	private UIActionButton m_TeleportMeTo;
	private UIActionButton m_TeleportPrevious;

	private UIActionEditableText m_PositionX;
	private bool m_PositionXUpdated;
	private UIActionEditableText m_PositionY;
	private bool m_PositionYUpdated;
	private UIActionEditableText m_PositionZ;
	private bool m_PositionZUpdated;
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

	private UIActionButton m_VomitPlayer;
	private UIActionButton m_SetScalePlayer;

	private UIActionButton m_KillPlayer;
	private UIActionButton m_SendMessage;
	private UIActionButton m_KickPlayer;
	private UIActionButton m_BanPlayer;

	private int m_NumPlayerCount;

	private JMPlayerInstance m_SelectedInstance;

	private JMPlayerModule m_Module;

	private int m_LastChangeTime;
	
	private autoptr TStringArray m_HeatBufferStates = {"", "+", "++", "+++"};
	private autoptr TStringArray m_PlayersPref1 = {};
	private autoptr TStringArray m_PlayersPref2 = {};

	private bool m_AutoSelect = true;  //! Auto-select player when first shown

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
		if (!g_Game)
			return;

		JMScriptInvokers.MENU_PLAYER_CHECKBOX.Remove( OnPlayer_Checked );
		JMScriptInvokers.MENU_PLAYER_BUTTON.Remove( OnPlayer_Button );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		super.OnClientPermissionsUpdated();

		UpdatePermission( m_HealPlayer, "Admin.Player.Heal" );
		UpdatePermission( m_Health, "Admin.Player.Set.Health" );
		UpdatePermission( m_KillPlayer, "Admin.Player.Set.Health" );
		UpdatePermission( m_Shock, "Admin.Player.Set.Shock" );
		UpdatePermission( m_Blood, "Admin.Player.Set.Blood" );
		UpdatePermission( m_Energy, "Admin.Player.Set.Energy" );
		UpdatePermission( m_Water, "Admin.Player.Set.Water" );
		UpdatePermission( m_Stamina, "Admin.Player.Set.Stamina" );
		
		UpdatePermission( m_HeatBuffer, "Admin.Player.Set.HeatBuffer" );

		UpdatePermission( m_BloodyHands, "Admin.Player.Set.BloodyHands" );

		UpdatePermission( m_ModifyPermissions, "Admin.Player.Permissions" );
		UpdatePermission( m_ModifyRoles, "Admin.Player.Roles" );

		UpdatePermission( m_Freeze, "Admin.Player.Freeze" );
		UpdatePermission( m_Invisibility, "Admin.Player.Invisibility" );
		UpdatePermission( m_UnlimitedAmmo, "Admin.Player.UnlimitedAmmo" );
		UpdatePermission( m_AdminNVG, "Admin.Player.AdminNVG" );
		UpdatePermission( m_UnlimitedStamina, "Admin.Player.UnlimitedStamina" );
		UpdatePermission( m_BrokenLegs, "Admin.Player.BrokenLegs" );
		UpdatePermission( m_GodMode, "Admin.Player.Godmode" );
		UpdatePermission( m_SpectatePlayer, "Admin.Player.Spectate" );
		UpdatePermission( m_StopBleeding, "Admin.Player.StopBleeding" );
		UpdatePermission( m_StripPlayer, "Admin.Player.Strip" );
		UpdatePermission( m_DryPlayer, "Admin.Player.Dry" );
		UpdatePermission( m_ReceiveDmgDealt, "Admin.Player.ReceiveDamageDealt" );
		UpdatePermission( m_KickPlayer, "Admin.Player.Kick" );
		UpdatePermission( m_VomitPlayer, "Admin.Player.Vomit" );
		UpdatePermission( m_SetScalePlayer, "Admin.Player.Scale" );
		UpdatePermission( m_CannotBeTargetedByAI, "Admin.Player.CannotBeTargetedByAI" );
		UpdatePermission( m_RemoveCollision, "Admin.Player.RemoveCollision" );

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

	private void UpdatePermission( UIActionBase base, string permission )
	{
		if ( !base )
			return;

		base.UpdatePermission( permission );
	}

	override void OnInit()
	{
		InitWidgetsLeft();
		InitWidgetsRight();

		RefreshStats(true);  //! Show correct state on reinit
	}

	private void InitWidgetsLeft()
	{
		m_LeftPanel = layoutRoot.FindAnyWidget( "panel_left" );

		Widget leftPanelGrid = UIActionManager.CreateGridSpacer( m_LeftPanel, 5, 1 );

		m_PlayerListCount 		= UIActionManager.CreateText( leftPanelGrid, "#STR_COT_PLAYER_MODULE_LEFT_PLAYER_COUNT" );
		m_PlayerListFilter 		= UIActionManager.CreateEditableTextPreview( leftPanelGrid, "#STR_COT_PLAYER_MODULE_LEFT_FILTER", this, "Event_UpdatePlayerList" );
		
		Widget leftGroupsPanelGrid 	= UIActionManager.CreateGridSpacer( leftPanelGrid, 1, 2 );
		m_PlayerPrefSave 	= UIActionManager.CreateButton( leftGroupsPanelGrid, "#STR_COT_GENERIC_SAVE", this, "OnClick_PlayerPrefSave" );
		m_PlayerPrefLoad 	= UIActionManager.CreateButton( leftGroupsPanelGrid, "#STR_COT_GENERIC_LOAD", this, "OnClick_PlayerPrefLoad" );

		Widget navbarleftPanelGrid = UIActionManager.CreateWrapSpacer( leftPanelGrid );
		m_PlayerListSort 		= UIActionManager.CreateButtonToggle( navbarleftPanelGrid, "A/Z", "Z/A", this, "Event_UpdatePlayerList" );
		m_PlayerListSelectAll 	= UIActionManager.CreateButton( navbarleftPanelGrid, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", this, "Event_SelectAllPlayerList" );
		m_PlayerListDeSelectAll = UIActionManager.CreateButton( navbarleftPanelGrid, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", this, "Event_DeSelectAllPlayerList" );
		
		m_PlayerListSort.SetWidth(0.14);
		m_PlayerListSelectAll.SetWidth(0.36);
		m_PlayerListDeSelectAll.SetWidth(0.46);
		
		m_PlayerListScroller 	= UIActionManager.CreateScroller( leftPanelGrid );
		m_PlayerListRows 		= UIActionManager.CreateActionRows( m_PlayerListScroller.GetContentWidget() );
		
		for ( int i = 0; i < 2; i++ )
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
		InitActionWidgetsQuick( m_ActionsWrapper );
		InitActionWidgetsPermissions( m_ActionsWrapper );

		m_ActionListScroller.UpdateScroller();
	}

	private Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 3, 1 );
	
		Widget actions = UIActionManager.CreatePanel( parent, 0x00000000, 32 );
		UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_NAME", "" );
		m_Name = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_NAME", this, "Click_CopyPlayerName" );
		m_Name.SetWidth( 0.8 );
		m_Name.SetPosition( 0.2 );

		actions = UIActionManager.CreatePanel( parent, 0x00000000, 32 );
		UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_GUID", "" );
		m_GUID = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_GUID", this, "Click_CopyPlayerGUID" );
		m_GUID.SetWidth( 0.8 );
		m_GUID.SetPosition( 0.2 );

		actions = UIActionManager.CreatePanel( parent, 0x00000000, 32 );
		UIActionManager.CreateText( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_STEAMID", "" );
		m_Steam64ID = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_IDENTITY_STEAMID", this, "Click_CopyPlayerSteam64ID" );
		m_Steam64ID.SetWidth( 0.4 );
		m_Steam64ID.SetPosition( 0.2 );

		m_SteamProfile = UIActionManager.CreateButton( actions, "Profile", this, "Click_OpenPlayerSteamProfile" );
		m_SteamProfile.SetWidth( 0.4 );
		m_SteamProfile.SetPosition( 0.6 );

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		ShowIdentityWidgets();

		return parent;
	}

	private void ShowIdentityWidgets()
	{
		m_Name.Show();
		m_GUID.Show();
		m_Steam64ID.Show();
		m_SteamProfile.Show();
	}

	private void HideIdentityWidgets()
	{
		m_Name.Hide();
		m_GUID.Hide();
		m_Steam64ID.Hide();
		m_SteamProfile.Hide();
	}

	private Widget InitActionWidgetsPosition( Widget actionsParent )
	{
		Widget parent = UIActionManager.CreateGridSpacer( actionsParent, 4, 1 );

		Widget positionHeader = UIActionManager.CreateGridSpacer( parent, 1, 3 );

		m_CopyRotationPlayer = UIActionManager.CreateButton(positionHeader, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_ORIENTATION_COPY", this, "Click_CopyPlayerRotation");
        m_CopyPositionPlayer = UIActionManager.CreateButton(positionHeader, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_COPY", this, "Click_CopyPlayerPostion");
        m_PastePositionPlayer = UIActionManager.CreateButton(positionHeader, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_PASTE", this, "Click_PastePlayerPostion");

		Widget positionActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget positionActionsVec = UIActionManager.CreateGridSpacer( positionActions, 1, 3 );

		m_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:", this, "Change_PositionX" );
		m_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:", this, "Change_PositionY" );
		m_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:", this, "Change_PositionZ" );

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

		Widget actions = UIActionManager.CreateGridSpacer( parent, 5, 2 );

		m_Health = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEALTH", 0, 100, this, "Click_SetHealth" );
		m_Water = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_WATER", 0, PlayerConstants.SL_WATER_MAX, this, "Click_SetWater" );
		
		m_Shock = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_SHOCK", 0, 100, this, "Click_SetShock" );
		m_Energy = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FOOD", 0, PlayerConstants.SL_ENERGY_MAX, this, "Click_SetEnergy" );
		
		m_Blood = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOOD", 0, 5000, this, "Click_SetBlood" );
		m_Stamina = UIActionManager.CreateSlider( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_STAMINA", 0, CfgGameplayHandler.GetStaminaMax(), this, "Click_SetStamina" );

		Widget headerTemos = UIActionManager.CreateGridSpacer( parent, 1, 2 );
		UIActionManager.CreateText( headerTemos, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_TEMPERATURES");

		Widget actionsTemps = UIActionManager.CreateGridSpacer( parent, 2, 2 );
		m_HeatComfort = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATCOMFORT", -0.75, 0.75, this );
		m_HeatComfort.SetFormat("");
		m_HeatComfort.Disable(); // You cant force the game, so this slider is READ ONLY
		m_HeatComfort.SetDisableAlpha(0.0);

		m_HeatBuffer = UIActionManager.CreateSlider( actionsTemps, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_HEATBUFFER", 0, 3, this, "Click_SetHeatBuffer" );
		m_HeatBuffer.SetStepValue(0.1);

		m_Health.SetSliderWidth(0.5);
		m_Shock.SetSliderWidth(0.5);
		m_Blood.SetSliderWidth(0.5);
		m_Energy.SetSliderWidth(0.5);
		m_Water.SetSliderWidth(0.5);
		m_Stamina.SetSliderWidth(0.5);
		m_HeatComfort.SetSliderWidth(0.5);
		m_HeatBuffer.SetSliderWidth(0.5);

		Widget actionButtons = UIActionManager.CreateGridSpacer( parent, 1, 2 );
		m_ApplyStats = UIActionManager.CreateButton( actionButtons, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_APPLY", this, "Click_ApplyStats" );
		m_RefreshStats = UIActionManager.CreateButton( actionButtons, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_REFRESH", this, "Click_RefreshStats" );

		Widget actions2 = UIActionManager.CreateGridSpacer( parent, 5, 2 );

		m_RemoveCollision = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_REMOVE_COLLISION", this, "Click_RemoveCollision", false );
		m_GodMode = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_GODMODE", this, "Click_GodMode", false );
		
		m_Freeze = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_FREEZE", this, "Click_Freeze", false );
		m_Invisibility = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_INVISIBLE", this, "Click_Invisible", false );
		
		m_BloodyHands = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BLOODY_HANDS", this, "Click_BloodyHands", false );
		m_CannotBeTargetedByAI = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_IGNORED_BY_AI", this, "Click_CannotBeTargetedByAI", false );
		
		m_BrokenLegs = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_BROKEN_LEGS", this, "Click_SetBrokenLegs", false );
		m_UnlimitedStamina = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_STAMINA", this, "Click_UnlimitedStamina", false );
		
		m_ReceiveDmgDealt = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_RECEIVE_DAMAGE_DEALT", this, "Click_SetReceiveDamageDealt", false );
		m_UnlimitedAmmo = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_UNLIMITED_AMMO", this, "Click_UnlimitedAmmo", false );
		
		m_AdminNVG = UIActionManager.CreateCheckbox( actions2, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_NVG", this, "Click_AdminNVG", false );

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

		// Friendly actions at the top
		m_HealPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEAL", this, "Click_HealPlayer" );
		m_RepairTransport = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_REPAIR_TRANSPORT", this, "Click_RepairTransport" );
		m_StopBleeding = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", this, "Click_StopBleeding" );
		m_DryPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_DRY", this, "Click_DryPlayer" );
		
		// Misc actions inbetween
		m_SpectatePlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SPECTATE", this, "Click_SpectatePlayer" );
		m_SendMessage 	 = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SEND_MESSAGE", this, "Click_SendMessage" );
		m_VomitPlayer 	 = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_VOMIT", this, "Click_VomitPlayer" );
		m_SetScalePlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SETSCALE", this, "Click_ScalePlayer" );

		// Destructive actions at the bottom
		m_KillPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KILL", this, "Click_KillPlayer" );
		m_StripPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", this, "Click_StripPlayer" );
		m_KickPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", this, "Click_KickPlayer" );
		m_BanPlayer = UIActionManager.CreateButton( actions, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", this, "Click_BanPlayer" );

		m_KillPlayer.SetColor(COLOR_RED);
		m_StripPlayer.SetColor(COLOR_RED);
		m_KickPlayer.SetColor(COLOR_RED);
		m_BanPlayer.SetColor(COLOR_RED);

		UIActionManager.CreatePanel( parent, 0xFF000000, 3 );

		return parent;
	}

	void OnClick_PlayerPrefSave( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_SAVE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_SavePlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_SavePlayerListPref02" );
	}

	void OnClick_SavePlayerListPref01()
	{
		m_PlayersPref1 = new TStringArray;
		SavePlayerListPref(m_PlayersPref1);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_SavePlayerListPref02()
	{
		m_PlayersPref2 = new TStringArray;
		SavePlayerListPref(m_PlayersPref2);
		
		if (m_PlayersPref1.Count() == 0 && m_PlayersPref2.Count() == 0)
			m_PlayerPrefLoad.Disable();
		else
			m_PlayerPrefLoad.Enable();
	}

	void OnClick_PlayerPrefLoad( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_HEADER", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_LOAD_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 01", "OnClick_LoadPlayerListPref01", "#STR_COT_PLAYER_MODULE_SET_PLAYERPREF_GROUP 02", "OnClick_LoadPlayerListPref02" );
	}

	void OnClick_LoadPlayerListPref01()
	{
		LoadPlayerListPref(m_PlayersPref1);
	}

	void OnClick_LoadPlayerListPref02()
	{
		LoadPlayerListPref(m_PlayersPref2);
	}

	void SavePlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			if (player.Checkbox.IsChecked())
				playerArr.Insert(player.GetGUID());
		}
	}

	void LoadPlayerListPref(out TStringArray playerArr)
	{
		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			bool state = playerArr.Find(player.GetGUID()) != -1;
			
			if (state)
				JM_GetSelected().AddPlayer(player.GetGUID());
			else
				JM_GetSelected().RemovePlayer(player.GetGUID());
			
			player.Checkbox.SetChecked(state);
		}
		
		UpdateUI();
		UpdatePlayerCount();
	}

	void Event_UpdatePlayerList( UIEvent eid, UIActionBase action )
	{
		UpdatePlayerList();
	}

	void Event_DeSelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(false);
	}

	void Event_SelectAllPlayerList( UIEvent eid, UIActionBase action )
	{
		SelectAllPlayerList(true);
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

		int permissionIdx = 0;

		m_PermissionsRows = UIActionManager.CreateActionRows( m_PermissionsListScroller.GetContentWidget() );

		JMPlayerInstance pi = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );

		m_RootPermission = pi.GetPermissions();
		array< JMPermission > permissions = new array< JMPermission >();
		GetPermissionsManager().GetPermissionsAsList( m_RootPermission, permissions );

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

	void Click_ModifyPermissions( UIEvent eid, UIActionBase action )
	{
		if ( m_PermissionsListScroller.IsVisible() )
		{
			HidePermissions();
		} else
		{
			ShowPermissions();
		}
	}
	
	void Click_ModifyRoles( UIEvent eid, UIActionBase action )
	{
		if ( m_RolesListScroller.IsVisible() )
		{
			HideRoles();
		} else
		{
			ShowRoles();
		}
	}

	void Click_SavePermissions( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetPermissions( m_RootPermission, {JM_GetSelected().GetPlayers()[0]} );
	}
	
	void Click_SaveRoles( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		array< string > roles = new array< string >();
		for ( int i = 0; i < m_RoleList.Count(); ++i )
		{
			if ( m_RoleList[i].IsChecked() )
				roles.Insert( m_RoleList[i].Name );
		}

		UpdateLastChangeTime();

		m_Module.SetRoles( roles, {JM_GetSelected().GetPlayers()[0]} );
	}

	void Click_StripPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_CLEAR_INVENTORY", "Strip");
	}

	void Strip(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.Strip( {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.Strip( JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.Strip( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}		
	}

	void Click_DryPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_DRY", "Dry", false);
	}

	void Dry(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.Dry( {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.Dry( JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.Dry( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_KillPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KILL", "KillPlayer");
	}

	void KillPlayer(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.SetHealth( 0, {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.SetHealth( 0, JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.SetHealth( 0, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_VomitPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		VomitPlayer();

		//! TODO: Figure this one out - Multi Options Options wasnt part of my plan x)
		//CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_VOMIT", "VomitPlayer");
	}

	void VomitPlayer()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_HEADER", "#STR_COT_PLAYER_MODULE_SET_VOMIT_DURATION_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "VomitPlayerConfirm" );
	}

	void VomitPlayerConfirm(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text == "" )
			return;

		float value = text.ToFloat();
		if (value < 1 || value > 120)
			return;

		m_Module.Vomit(value, JM_GetSelected().GetPlayers() );
	}

	void Click_ScalePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		ScalePlayer();

		//! TODO: Figure this one out - Multi Options Options wasnt part of my plan x)
		//CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SETSCALE", "ScalePlayer", false);
	}

	void ScalePlayer()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_SET_SCALE_HEADER", "#STR_COT_PLAYER_MODULE_SET_SCALE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "ScalePlayerConfim" );
	}

	void ScalePlayerConfim(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text == "" )
			return;

		float value = text.ToFloat();
		if (value < 0.1 || value > 10)
			value = 1;

		m_Module.SetScale( value, JM_GetSelected().GetPlayers() );
	}

	void Click_SendMessage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SendMessages();

		//! TODO: Figure this one out - Multi Options Options wasnt part of my plan x)
		//CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_SEND_MESSAGE", "SendMessage", false);
	}

	void SendMessages()
	{
		CreateConfirmation_Three( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_CONFIRM_MESSAGE", "SendMessagesConfirm", "#STR_COT_CONFIRM_NOTIFICATION", "SendNotifs" );
	}

	void SendMessagesConfirm(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text == "" )
			return;

		m_Module.DoMessage( JM_GetSelected().GetPlayers(), text);
	}

	void SendNotifs(JMConfirmation confirmation)
	{
		string text = confirmation.GetEditBoxValue();
		if ( text == "" )
			return;

		m_Module.DoNotif( JM_GetSelected().GetPlayers(), text);
	}

	void Click_KickPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayer");
	}

	void KickPlayer(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();

		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_KICK_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_KICK", "KickPlayerConfirm" );
	}

	void KickPlayersConfirm(JMConfirmation confirmation)
	{
		switch(confirmation.GetSelectedID())
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.Kick( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue() );
			break;
			// Everyone
			case 3:
				m_Module.Kick( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue() );
			break;
			// Self
			default:
			case -1:
			case 6:
				//! TODO: Add a error notification here !
				// You cant kick yourself
			break;
		}
	}

	void Click_BanPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayer");
	}

	void BanPlayer()
	{
		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_HEADER", "#STR_COT_PLAYER_MODULE_BAN_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_BAN", "BanPlayerConfirm" );
	}

	void BanPlayerConfirm(JMConfirmation confirmation)
	{
		switch(confirmation.GetSelectedID())
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.Ban( {JM_GetSelected().GetPlayers()[0]}, confirmation.GetEditBoxValue() );
			break;
			// Everyone
			case 3:
				m_Module.Ban( JM_GetSelected().GetPlayers(), confirmation.GetEditBoxValue() );
			break;
			// Self
			default:
			case -1:
			case 6:
				//! TODO: Add a error notification here !
				// You cant kick yourself
			break;
		}
	}

	void Click_StopBleeding( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_STOP_BLEEDING", "StopBleeding", false);
	}

	void StopBleeding(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.StopBleeding( {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.StopBleeding( JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.StopBleeding( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_HealPlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_QUICK_ACTIONS_HEAL", "Heal", false);
	}

	void Heal(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.Heal( {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.Heal( JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.Heal( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_SpectatePlayer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		bool shouldSpectate = true;

		if ( CurrentActiveCamera )
		{
			if ( CurrentActiveCamera.IsInherited(JMSpectatorCamera) )
			{
				shouldSpectate = false;
			}
			else if ( COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera) )
			{
				shouldSpectate = false;
			}
		}

		if ( shouldSpectate )
		{
			if ( JM_GetSelected().GetPlayers().Count() != 1 )
				return;

			action.Disable();
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 1000);

			m_Module.StartSpectating( JM_GetSelected().GetPlayers()[0] );
		} else
		{
			action.Disable();
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(action.Enable, 3000);

			m_Module.EndSpectating();
		}
	}

	void Click_RepairTransport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.RepairTransport( JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_COORDINATES", "SetPositionTeleport", false);
	}

	void SetPositionTeleport(JMConfirmation confirmation = NULL)
	{
		UpdateLastChangeTime();

		vector pos = vector.Zero;
		pos[0] = m_PositionX.GetText().ToFloat();
		pos[1] = m_PositionY.GetText().ToFloat();
		pos[2] = m_PositionZ.GetText().ToFloat();

		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.TeleportTo(pos, {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.TeleportTo(pos, JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.TeleportTo(pos, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_TeleportToMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_TO_ME", "TeleportToMe", false);
	}

	void TeleportToMe(JMConfirmation confirmation = NULL)
	{
		vector pos = vector.Zero;

		if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
		{
			pos = CurrentActiveCamera.GetPosition();
		} else if ( GetPlayer() )
		{
			pos = GetPlayer().GetPosition();
		} else
		{
			return;
		}

		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.TeleportTo(pos, {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.TeleportTo(pos, JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.TeleportTo(pos, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

    void Click_CopyPlayerName( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard(m_Name.GetButton());
	}

    void Click_CopyPlayerGUID( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard(m_GUID.GetButton());
	}

    void Click_CopyPlayerSteam64ID( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard(m_Steam64ID.GetButton());
	}

    void Click_OpenPlayerSteamProfile( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().OpenURL("https://steamcommunity.com/profiles/" + m_Steam64ID.GetButton());
	}

    void Click_CopyPlayerPostion( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard("<" + m_PositionX.GetText() + ", " + m_PositionY.GetText() + ", " + m_PositionZ.GetText() + ">");
	}

    void Click_CopyPlayerRotation( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_SelectedInstance )
			return;

		vector rotation = m_SelectedInstance.GetOrientation();

		GetGame().CopyToClipboard("<" + rotation[0] + ", " + rotation[1] + ", " + rotation[2] + ">");
	}

	void Click_PastePlayerPostion( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		GetGame().CopyFromClipboard(clipboard);

		vector pos = clipboard.BeautifiedToVector();

		if (pos != vector.Zero)
		{
			m_PositionX.SetText(pos[0].ToString());
			m_PositionY.SetText(pos[1].ToString());
			m_PositionZ.SetText(pos[2].ToString());

			Click_SetPosition(eid, action);
		}
	}

	void Click_TeleportMeTo( UIEvent eid, UIActionBase action )
	{
		if ( JM_GetSelected().GetPlayers().Count() != 1 )
			return;

		if ( eid != UIEvent.CLICK )
			return;

		m_Module.TeleportSenderTo( JM_GetSelected().GetPlayers()[0] );
	}

	void Click_TeleportPrevious( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_PREVIOUS", "TeleportToPrevious", false);
	}

	void TeleportToPrevious(JMConfirmation confirmation = NULL)
	{
		int btnId = -1;
		if (confirmation)
			btnId = confirmation.GetSelectedID();
		
		switch(btnId)
		{
			// Abort
			case 1:
			case 4:
				return;
			break;
			// Selected
			case 2:
			case 5:
				m_Module.TeleportToPrevious( {JM_GetSelected().GetPlayersOrSelf()[0]} );
			break;
			// Everyone
			case 3:
				m_Module.TeleportToPrevious( JM_GetSelected().GetPlayers() );
			break;
			// Self
			default:
			case -1:
			case 6:
				m_Module.TeleportToPrevious( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
			break;
		}
	}

	void Click_RefreshStats( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RefreshStats(true);
	}
	
	void Click_RefreshTeleports( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
			
		RefreshTeleports(true);
	}
	
	void Change_PositionX( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionXUpdated = true;
	}
	
	void Change_PositionY( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionYUpdated = true;
	}
	
	void Change_PositionZ( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		UpdateLastChangeTime();

		m_PositionZUpdated = true;
	}

	void RefreshStats(bool force = false)
	{
		if ( !m_SelectedInstance )
			return;
		
		if (GetGame().IsClient() && m_SelectedInstance.GetDataLastUpdatedTime() < m_LastChangeTime)
			return;

		RefreshTeleports(force);

		if (force)
		{
			m_HealthUpdated = false;
			m_BloodUpdated = false;
			m_EnergyUpdated = false;
			m_WaterUpdated = false;
			m_ShockUpdated = false;
			m_StaminaUpdated = false;
			m_HeatBufferUpdated = false;
		}

		if ( m_Health && !m_HealthUpdated )
			m_Health.SetCurrent( m_SelectedInstance.GetHealth() );
		
		if ( m_Blood && !m_BloodUpdated )
			m_Blood.SetCurrent( m_SelectedInstance.GetBlood() );
		
		if ( m_Energy && !m_EnergyUpdated )
			m_Energy.SetCurrent( m_SelectedInstance.GetEnergy() );
		
		if ( m_Water && !m_WaterUpdated )
			m_Water.SetCurrent( m_SelectedInstance.GetWater() );
		
		if ( m_Shock && !m_ShockUpdated )
			m_Shock.SetCurrent( m_SelectedInstance.GetShock() );
		
		if ( m_Stamina && !m_StaminaUpdated )
			m_Stamina.SetCurrent( m_SelectedInstance.GetStamina() );
		
		if ( m_HeatComfort )
			m_HeatComfort.SetCurrent( m_SelectedInstance.GetHeatComfort() );

		if ( m_HeatBuffer && !m_HeatBufferUpdated )
		{
			m_HeatBuffer.SetCurrent( m_SelectedInstance.GetHeatBuffer() / 10 );
			int heatbufferState = m_HeatBuffer.GetCurrent();
			m_HeatBuffer.SetText(m_HeatBufferStates[heatbufferState] + " "+ m_HeatBuffer.GetCurrent());
		}

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
		
		if ( m_AdminNVG )
			m_AdminNVG.SetChecked( m_SelectedInstance.HasAdminNVG() );

		if ( m_BrokenLegs )
			m_BrokenLegs.SetChecked( m_SelectedInstance.HasBrokenLegs() );
		
		if ( m_ReceiveDmgDealt )
			m_ReceiveDmgDealt.SetChecked( m_SelectedInstance.GetReceiveDmgDealt() );
		
		if ( m_CannotBeTargetedByAI )
			m_CannotBeTargetedByAI.SetChecked( m_SelectedInstance.GetCannotBeTargetedByAI() );
		
		if ( m_RemoveCollision )
			m_RemoveCollision.SetChecked( m_SelectedInstance.GetRemoveCollision() );

		if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_HIGH )
		{
			m_Health.SetColor( Colors.COLOR_PRISTINE );
			m_Health.SetAlpha( 1.0 );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_NORMAL )
		{
			m_Health.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_LOW )
		{
			m_Health.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Health.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_HIGH )
		{
			m_Blood.SetColor( Colors.COLOR_PRISTINE );
			m_Blood.SetAlpha( 1.0 );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_NORMAL )
		{
			m_Blood.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_LOW )
		{
			m_Blood.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Blood.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_HIGH )
		{
			m_Energy.SetColor( Colors.COLOR_PRISTINE );
			m_Energy.SetAlpha( 1.0 );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_NORMAL )
		{
			m_Energy.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_LOW )
		{
			m_Energy.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Energy.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_HIGH )
		{
			m_Water.SetColor( Colors.COLOR_PRISTINE );
			m_Water.SetAlpha( 1.0 );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_NORMAL )
		{
			m_Water.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_LOW )
		{
			m_Water.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Water.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		if ( m_Shock.GetCurrent() >= 75 )
		{
			m_Shock.SetColor( Colors.COLOR_PRISTINE );
			m_Shock.SetAlpha( 1.0 );
		}
		else if ( m_Shock.GetCurrent() >= PlayerConstants.CONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Shock.GetCurrent() < PlayerConstants.UNCONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( ARGB( 255, 220, 0, 0 ) );
		}
		else
		{
			m_Shock.SetColor( ARGB( 255, 220, 220, 0 ) );
		}

		if ( m_Stamina.GetCurrent() >= 75 )
		{
			m_Stamina.SetColor( Colors.COLOR_PRISTINE );
			m_Stamina.SetAlpha( 1.0 );
		}
		else if ( m_Stamina.GetCurrent() >= 50 )
		{
			m_Stamina.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Stamina.GetCurrent() >= 25 )
		{
			m_Stamina.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Stamina.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		// COLD
		if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_ONE );
		}
		// NEUTRAL
		else if ( m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY && m_HeatComfort.GetCurrent() < PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_NEUTAL );
		}
		// HOT
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_ONE );
		}
		m_HeatComfort.SetAlpha( 1.0 );

		if ( m_HeatBuffer.GetCurrent() >= 3 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_PRISTINE );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 2 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 1 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else
		{
			m_HeatBuffer.SetColor(0x00FFFFFF);
		}
		m_HeatBuffer.SetAlpha( 1.0 );
	}

	void RefreshTeleports(bool force = false)
	{
		if ( !m_SelectedInstance )
			return;

		if ( IsMissionOffline() )
			m_SelectedInstance.Update();

		vector position = m_SelectedInstance.GetPosition();

		if (force)
		{
			m_PositionXUpdated = false;
			m_PositionYUpdated = false;
			m_PositionZUpdated = false;
		}

		if ( m_PositionX && !m_PositionXUpdated )
			m_PositionX.SetText( position[0].ToString() );

		if ( m_PositionY && !m_PositionYUpdated )
			m_PositionY.SetText( position[1].ToString() );

		if ( m_PositionZ && !m_PositionZUpdated )
			m_PositionZ.SetText( position[2].ToString() );
	}

	void Click_ApplyStats( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		if ( m_HealthUpdated )
		{
			m_HealthUpdated = false;

			if ( m_Health )
				m_Module.SetHealth( m_Health.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_BloodUpdated )
		{
			m_BloodUpdated = false;

			if ( m_Blood )
				m_Module.SetBlood( m_Blood.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_EnergyUpdated )
		{
			m_EnergyUpdated = false;

			if ( m_Energy )
				m_Module.SetEnergy( m_Energy.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_WaterUpdated )
		{
			m_WaterUpdated = false;

			if ( m_Water )
				m_Module.SetWater( m_Water.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_ShockUpdated )
		{
			m_ShockUpdated = false;

			if ( m_Shock )
				m_Module.SetShock( m_Shock.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_StaminaUpdated )
		{
			m_StaminaUpdated = false;

			if ( m_Stamina )
				m_Module.SetStamina( m_Stamina.GetCurrent(), JM_GetSelected().GetPlayersOrSelf() );
		}

		if ( m_HeatBufferUpdated )
		{
			m_HeatBufferUpdated = false;

			if ( m_HeatBuffer )
				m_Module.SetHeatBuffer( m_HeatBuffer.GetCurrent() * 10, JM_GetSelected().GetPlayersOrSelf() );
		}
	}

	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();
	
		if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_HIGH )
		{
			m_Health.SetColor( Colors.COLOR_PRISTINE );
			m_Health.SetAlpha( 1.0 );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_NORMAL )
		{
			m_Health.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Health.GetCurrent() >= PlayerConstants.SL_HEALTH_LOW )
		{
			m_Health.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Health.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		m_HealthUpdated = true;
	}

	void Click_SetBlood( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_HIGH )
		{
			m_Blood.SetColor( Colors.COLOR_PRISTINE );
			m_Blood.SetAlpha( 1.0 );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_NORMAL )
		{
			m_Blood.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Blood.GetCurrent() >= PlayerConstants.SL_BLOOD_LOW )
		{
			m_Blood.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Blood.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		m_BloodUpdated = true;
	}

	void Click_SetEnergy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_HIGH )
		{
			m_Energy.SetColor( Colors.COLOR_PRISTINE );
			m_Energy.SetAlpha( 1.0 );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_NORMAL )
		{
			m_Energy.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Energy.GetCurrent() >= PlayerConstants.SL_ENERGY_LOW )
		{
			m_Energy.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Energy.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		m_EnergyUpdated = true;
	}

	void Click_SetWater( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_HIGH )
		{
			m_Water.SetColor( Colors.COLOR_PRISTINE );
			m_Water.SetAlpha( 1.0 );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_NORMAL )
		{
			m_Water.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Water.GetCurrent() >= PlayerConstants.SL_WATER_LOW )
		{
			m_Water.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Water.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		m_WaterUpdated = true;
	}

	void Click_SetShock( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Shock.GetCurrent() >= 75 )
		{
			m_Shock.SetColor( Colors.COLOR_PRISTINE );
			m_Shock.SetAlpha( 1.0 );
		}
		else if ( m_Shock.GetCurrent() >= PlayerConstants.CONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Shock.GetCurrent() < PlayerConstants.UNCONSCIOUS_THRESHOLD )
		{
			m_Shock.SetColor( ARGB( 255, 220, 0, 0 ) );
		}
		else
		{
			m_Shock.SetColor( ARGB( 255, 220, 220, 0 ) );
		}

		m_ShockUpdated = true;
	}

	void Click_SetStamina( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_Stamina.GetCurrent() >= 75 )
		{
			m_Stamina.SetColor( Colors.COLOR_PRISTINE );
			m_Stamina.SetAlpha( 1.0 );
		}
		else if ( m_Stamina.GetCurrent() >= 50 )
		{
			m_Stamina.SetColor( ARGB( 255, 220, 220, 220 ) );
		}
		else if ( m_Stamina.GetCurrent() >= 25 )
		{
			m_Stamina.SetColor( ARGB( 255, 220, 220, 0 ) );
		}
		else
		{
			m_Stamina.SetColor( ARGB( 255, 220, 0, 0 ) );
		}

		m_StaminaUpdated = true;
	}

	void Click_SetHeatBuffer( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateLastChangeTime();

		if ( m_HeatBuffer.GetCurrent() >= 3 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_PRISTINE );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 2 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else if ( m_HeatBuffer.GetCurrent() >= 1 )
		{
			m_HeatBuffer.SetColor( Colors.COLOR_WORN );
		}
		else
		{
			m_HeatBuffer.SetColor(0x00FFFFFF);
		}

		int heatbufferState = m_HeatBuffer.GetCurrent();
		m_HeatBuffer.SetText(m_HeatBufferStates[heatbufferState] +" "+ m_HeatBuffer.GetCurrent());

		m_HeatBufferUpdated = true;
		m_HeatBuffer.SetAlpha( 1.0 );
		
		// COLD
		if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_FOUR );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() <= PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_COLD_LVL_ONE );
		}
		// NEUTRAL
		else if ( m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_EMPTY && m_HeatComfort.GetCurrent() > PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_NEUTAL );
		}
		// HOT
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_EMPTY )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_ONE );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_TWO );
		}
		else if ( m_HeatComfort.GetCurrent() >= PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_CRITICAL )
		{
			m_HeatComfort.SetColor( Colors.TEMPERATURE_HOT_LVL_FOUR );
		}
		m_HeatComfort.SetAlpha( 1.0 );
	}

	void Click_BloodyHands( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetBloodyHands( m_BloodyHands.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_GodMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetGodMode( m_GodMode.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_Freeze( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetFreeze( m_Freeze.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetBrokenLegs( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetBrokenLegs( m_BrokenLegs.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_SetReceiveDamageDealt( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetReceiveDamageDealt( m_ReceiveDmgDealt.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_CannotBeTargetedByAI( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetCannotBeTargetedByAI( m_CannotBeTargetedByAI.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_Invisible( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetInvisible( m_Invisibility.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_RemoveCollision( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetRemoveCollision( m_RemoveCollision.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_UnlimitedAmmo( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetUnlimitedAmmo( m_UnlimitedAmmo.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_AdminNVG( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetAdminNVG( m_AdminNVG.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
	}

	void Click_UnlimitedStamina( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UpdateLastChangeTime();

		m_Module.SetUnlimitedStamina( m_UnlimitedStamina.IsChecked(), JM_GetSelected().GetPlayersOrSelf() );
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
		if ( JM_GetSelected().NumPlayers(m_AutoSelect) == 0 )
		{
			HideUI();
			return;
		}

		m_AutoSelect = false;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers(false)[0] );
		
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
		
		if ( JM_GetSelected().NumPlayers(false) == 1 )
		{
			UpdatePermission( m_TeleportMeTo, "Admin.Player.Teleport.SenderTo" );
		} else {
			m_TeleportMeTo.Disable();
		}

		ShowIdentityWidgets();
		m_GUID.SetButton( instance.GetGUID() );
		m_Name.SetButton( instance.GetName() );
		m_Steam64ID.SetButton( instance.GetSteam64ID() );

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
		}

		RefreshStats();
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayerList, 1500, true );
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RefreshStats, 100, true, false );

		UpdateUI();

		UpdatePlayerList();
	}

	override void OnHide() 
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePlayerList );
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Remove( RefreshStats );
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

	private void SortPlayersArray( out array< JMPlayerInstance > players, bool isReversed )
	{
		TStringArray pNames = new TStringArray;
		TIntArray pIndices = new TIntArray;

		for ( int i = 0; i < players.Count(); i++ )
			pNames.Insert(players[ i ].GetName());
		
		pNames.Sort(isReversed);

		for ( i = 0; i < players.Count(); i++ )
		{
			for ( int j = 0; j < players.Count(); j++ )
			{
				if ( pNames[ j ] == players[ i ].GetName() )
				{
					pIndices.Insert(j);
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
		m_PlayerListCount.SetText( "" + m_NumPlayerCount + " (" + JM_GetSelected().GetPlayers(false).Count() + ")" );
	}

	void SelectAllPlayerList(bool state = true)
	{
		string guid;

		foreach(JMPlayerRowWidget player: m_PlayerList)
		{
			guid = player.GetGUID();

			if (guid == string.Empty)
				continue;

			if (state)
				JM_GetSelected().AddPlayer(guid);
			else
				JM_GetSelected().RemovePlayer(guid);
			
			player.Checkbox.SetChecked(state);
		}

		UpdateUI();
		UpdatePlayerCount();
	}

	void UpdatePlayerList()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "UpdatePlayerList" );
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

		SortPlayersArray( players, m_PlayerListSort.IsToggled() );

		int idx = 0;
		int pIdx = 0;

		m_NumPlayerCount = 0;

		string filter = m_PlayerListFilter.GetText();
		bool isFiltering = filter.Length() > 0;
		filter.ToLower();
		
		TStringArray suggestions = new TStringArray;
		string closestMatch;

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

				if ( isFiltering )
				{
					if (!pName.Contains( filter ))
						continue;

					if ( pName == filter )
					{
						suggestions.Clear();
						closestMatch = pName;
					}
					else if ( pName.IndexOf(filter) == 0 )
					{
						if (!closestMatch)
							suggestions.Insert(pName);
					}
				}

				m_PlayerList[idx].SetPlayer( cPlayer.GetGUID() );

				m_NumPlayerCount++;
			} else
			{
				m_PlayerList[idx].SetPlayer( "" );
			}

			idx++;
		}
		
		if (suggestions.Count())
		{
			suggestions.Sort();
			closestMatch = suggestions[0];
		}
		
		m_PlayerListFilter.SetTextPreview(closestMatch);

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

	void UpdateLastChangeTime()
	{
		m_LastChangeTime = GetGame().GetTime();
	}
};
