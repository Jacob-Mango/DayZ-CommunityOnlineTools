class COTModule : JMModuleBase
{
	static ref COTModule COTInstance;

	protected ref JMCOTSideBar m_COTMenu;

	protected bool m_ForceHUD;

	protected bool m_GameActive;

	void COTModule()
	{
		COTInstance = this;

		MakeDirectory( JMConstants.DIR_COT );

		JMScriptInvokers.COT_ON_OPEN.Insert( SetMenuState );

		GetPermissionsManager().RegisterPermission( "COT.View" );
	}

	void ~COTModule()
	{
		m_COTMenu.Hide();

		JMScriptInvokers.COT_ON_OPEN.Remove( SetMenuState );

		delete m_COTMenu;
	}

	override void OnMissionLoaded()
	{
		if ( IsMissionClient() )
		{
			if ( m_COTMenu == NULL )
			{
				GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_menu.layout" ).GetScript( m_COTMenu );
			}
		}
	}

	override void OnMissionFinish()
	{
		if ( IsMissionClient() )
		{
			delete m_COTMenu;
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "ToggleMenu",		"UACOTToggleButtons",		true 	) );
		RegisterBinding( new JMModuleBinding( "FocusGame",		"UACOTModuleFocusGame",		true 	) );
		RegisterBinding( new JMModuleBinding( "FocusUI",		"UACOTModuleFocusUI",		true 	) );
		RegisterBinding( new JMModuleBinding( "ToggleCOT",		"UACOTModuleToggleCOT",		false 	) );
		RegisterBinding( new JMModuleBinding( "CloseCOT",		"UAUIBack",					true 	) );
	}

	override void OnUpdate( float timeslice )
	{
		if ( m_COTMenu )
		{
			m_COTMenu.OnUpdate( timeslice );
		}

		if ( m_ForceHUD )
		{
			GetGame().GetMission().GetHud().Show( false );
		}
	}

	void UpdateMouseControls()
	{
		bool isMenuOpen = m_COTMenu && m_COTMenu.IsVisible();
		bool windowsOpen = GetCOTWindowManager().Count() > 0;

		if ( !isMenuOpen && !windowsOpen )
			return;

		if ( m_GameActive )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
		} else
		{
			GetGame().GetInput().ChangeGameFocus( 1 );
			GetGame().GetUIManager().ShowUICursor( true );
		}
	}

	void COTForceHud( bool enable )
	{
		m_ForceHUD = enable;
		
		if ( !m_ForceHUD )
		{
			GetGame().GetMission().GetHud().Show( true );
		}
	}

	void SetMenuState( bool show )
	{
		if ( !m_COTMenu )
			return;

		if ( show )
		{
			if ( !m_COTMenu.IsVisible() )
			{
				m_COTMenu.Show();
			}
		} else
		{
			if ( m_COTMenu.IsVisible() )
			{
				m_COTMenu.Hide();
			}
		}
	}

	void CloseCOT( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		GetCommunityOnlineToolsBase().SetOpen( false );
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !( input.LocalPress() ) )
			return;

		GetCommunityOnlineToolsBase().ToggleOpen();
	}

	void FocusUI( UAInput input )
	{
		if ( !( input.LocalRelease() ) )
			return;

		if ( m_COTMenu == NULL )
			return;
		
		if ( m_GameActive )
		{
			m_GameActive = false;

			UpdateMouseControls();
		}
	}

	void FocusGame( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( GetGame().GetUIManager().GetMenu() )
			return;

		if ( m_COTMenu == NULL )
			return;

		if ( m_COTMenu.IsVisible() || GetCOTWindowManager().Count() > 0 )
		{
			bool canContinue = false;

			Widget parentWidget = GetWidgetUnderCursor();
			while ( parentWidget != NULL )
			{   
				if ( m_COTMenu && m_COTMenu.GetLayoutRoot() == parentWidget )
				{
					canContinue = true;
					break;
				}

				if ( GetCOTWindowManager().GetWindowFromWidget( parentWidget ) )
				{
					canContinue = true;
					break;
				}

				parentWidget = parentWidget.GetParent();
			}

			if ( !canContinue )
			{
				m_GameActive = true;

				UpdateMouseControls();
			}
		}
	}

	void ToggleCOT( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( m_COTMenu == NULL )
			return;

		if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
			return;

		GetCommunityOnlineToolsBase().ToggleActive();

		if ( GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLE", "STR_COT_on" ) );
		} else
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLE", "STR_COT_off" ) );
		}
	}

	override void OnInvokeConnect( PlayerBase player, PlayerIdentity identity )
	{
		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().Roles.GetElement( i ), identity );
		}
		
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientConnected( identity, instance ) )
		{
			instance.PlayerObject = player;
			GetCommunityOnlineToolsBase().SetClient( instance, identity );
		}
	}

	/**
	 * See: ClientRespawnEventTypeID
	 */
	override void OnClientRespawn( PlayerBase player, PlayerIdentity identity )
	{
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );
		if ( instance )
		{
			instance.PlayerObject = player;
		}
	}

	/**
	 * See: ClientReconnectEventTypeID
	 */
	override void OnClientReconnect( PlayerBase player, PlayerIdentity identity )
	{
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );
		if ( instance )
		{
			instance.PlayerObject = player;
		}
	}

	/**
	 * See: ClientDisconnectedEventTypeID
	 */
	override void OnClientLogout( PlayerBase player, PlayerIdentity identity, int logoutTime, bool authFailed )
	{
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );
		if ( instance )
		{
			instance.PlayerObject = player;
		}
	}

	/**
	 * See: MissionServer::PlayerDisconnected - Fires when the player has disconnected from the server (OnClientReconnect won't fire)
	 */
	override void OnClientDisconnect( PlayerBase player, PlayerIdentity identity, string uid )
	{
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientDisconnected( uid, instance ) )
		{
			GetCommunityOnlineToolsBase().RemoveClient( uid );
		}
	}

	/**
	 * See: LogoutCancelEventTypeID
	 */
	override void OnClientLogoutCancelled( PlayerBase player )
	{
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( player.GetIdentity().GetId() );
		if ( instance )
		{
			instance.PlayerObject = player;
		}
	}
}