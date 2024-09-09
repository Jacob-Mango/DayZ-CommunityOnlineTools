class COTModule : JMModuleBase
{
	protected JMCOTSideBar m_COTMenu;

	protected bool m_WasVisible;

	protected bool m_ForceHUD;

	protected bool m_GameActive;

	protected bool m_LeftMouseDown;

	void COTModule()
	{
		MakeDirectory( JMConstants.DIR_COT );

		JMScriptInvokers.COT_ON_OPEN.Insert( SetMenuState );

		GetPermissionsManager().RegisterPermission( "COT.View" );
	}

	void ~COTModule()
	{
		JMScriptInvokers.COT_ON_OPEN.Remove( SetMenuState );

		if ( m_COTMenu )
		{
			m_COTMenu.Destroy();
		}
	}

	override void OnMissionLoaded()
	{
		auto trace = CF_Trace_0(this, "OnMissionLoaded");

		super.OnMissionLoaded();
		
		if ( IsMissionClient() )
		{
			if ( !JMStatics.ESP_CONTAINER )
				JMStatics.ESP_CONTAINER = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_esp.layout", NULL );
			
			#ifndef CF_WINDOWS
			if ( !JMStatics.WINDOWS_CONTAINER )
				JMStatics.WINDOWS_CONTAINER = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_windows.layout", NULL );
			#endif

			if ( m_COTMenu == NULL )
			{
				GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_menu.layout" ).GetScript( m_COTMenu );
			}
		}

		if ( IsMissionHost() )
		{
			array< string > data = GetPermissionsManager().Serialize();

			if ( !GetPermissionsManager().RoleExists( "everyone" ) )
				GetPermissionsManager().CreateRole( "everyone", data );

			//if ( !GetPermissionsManager().RoleExists( "admin" ) )
			{
				for ( int i = 0; i < data.Count(); i++ )
				{
					string s = data[i];
					s.Replace( "0", "2" );
					data.Remove( i );
					data.InsertAt( s, i );
				}

				GetPermissionsManager().CreateRole( "admin", data );
			}
		}
	}

	override void OnMissionFinish()
	{
		if ( m_COTMenu )
		{
			m_COTMenu.Destroy();
			m_COTMenu = null;
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		Bind( new JMModuleBinding( "ToggleMenu",	"UACOTToggleButtons",		true 	) );
		Bind( new JMModuleBinding( "ToggleCOT",		"UACOTModuleToggleCOT",		false 	) );
		Bind( new JMModuleBinding( "CloseCOT",		"UAUIBack",					true 	) );
	}

	#ifdef SERVER
	override void EnableUpdate()
	{
	}
	#else
	override void OnUpdate( float timeslice )
	{
		JMStatics.COT_MENU = null;
		if ( m_COTMenu )
		{
			JMStatics.COT_MENU = m_COTMenu.GetLayoutRoot();

			m_COTMenu.OnUpdate( timeslice );

			#ifdef CF_WINDOWS
			if (m_COTMenu.IsVisible())
			{
				if (!m_WasVisible)
				{
					m_WasVisible = true;
				}
				
				CF_Windows.OverrideInputState(true, CF_WindowsFocusState.WINDOW);
			} else if (!m_COTMenu.IsVisible() && m_WasVisible)
			{
				m_WasVisible = false;
				CF_Windows.OverrideInputState(false);
			}
			#else
			if ( m_LeftMouseDown )
			{
				if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
				{
					OnMouseUp();

					m_LeftMouseDown = false;
				}
			} else
			{
				if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
				{
					OnMouseDown();
					
					m_LeftMouseDown = true;
				}
			}
			#endif
		}
		else if (m_WasVisible)
		{
			m_WasVisible = false;
			#ifdef CF_WINDOWS
			CF_Windows.OverrideInputState(false);
			#endif
		}

		if ( m_ForceHUD )
		{
			GetGame().GetMission().GetHud().Show( false );
		}
	}
	#endif

	#ifndef CF_WINDOWS
	void UpdateMouseControls()
	{
		bool isMenuOpen = m_COTMenu && m_COTMenu.IsVisible();
		bool windowsOpen = GetCOTWindowManager().HasAnyActive();

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
	#endif

	void COTForceHud( bool enable )
	{
		m_ForceHUD = enable;
		
		if ( !m_ForceHUD )
			GetGame().GetMission().GetHud().Show( true );
	}

	void SetMenuState( bool show )
	{
		if ( !m_COTMenu )
			return;

		if ( show )
		{
			if ( !m_COTMenu.IsVisible() )
				m_COTMenu.Show();
		} else {
			if ( m_COTMenu.IsVisible() )
				m_COTMenu.Hide();
		}
	}

	void CloseCOT( UAInput input )
	{
		if (!GetGame())
			return;

		if (!input.LocalPress())
			return;

		if (GetCommunityOnlineToolsBase() && GetCommunityOnlineToolsBase().IsOpen())
			GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Call( GetCommunityOnlineToolsBase().SetOpen, false );
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !input.LocalPress() )
			return;

		GetCommunityOnlineToolsBase().ToggleOpen();
	}

	#ifndef CF_WINDOWS
	void OnMouseUp()
	{
		if ( m_GameActive )
		{
			m_GameActive = false;

			UpdateMouseControls();
		}
	}

	void OnMouseDown()
	{
		if ( GetGame().GetUIManager().GetMenu() )
			return;

		if ( m_COTMenu.IsVisible() || GetCOTWindowManager().HasAnyActive() )
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

				if ( JMStatics.ESP_CONTAINER && JMStatics.ESP_CONTAINER == parentWidget )
				{
					canContinue = true;
					break;
				}

				parentWidget = parentWidget.GetParent();
			}

			if ( !canContinue )
			{
				m_GameActive = true;

				SetFocus( NULL );

				UpdateMouseControls();
			}
		}
	}
	#endif

	void ToggleCOT( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( m_COTMenu == NULL )
			return;

		if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
			return;

		GetCommunityOnlineToolsBase().ToggleActive();
	}

	//TODO: URGENT: MOVE TO COMMUNITY FRAMEWORK, FULLY DECOUPLE FROM COMMUNITY ONLINE TOOLS

	override void OnInvokeConnect( PlayerBase player, PlayerIdentity identity )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnInvokeConnect - " + identity.GetId() );
		#endif

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		#ifdef CF_MODULE_PERMISSIONS
		for ( int i = 0; i < GetPermissionsManager().RoleCount(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().GetRole( i ), identity );
		}
		#else
		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().Roles.GetElement(i), identity );
		}
		#endif
		
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientConnected( identity, instance ) )
		{
			instance.PlayerObject = player;

			GetCommunityOnlineToolsBase().SetClient( instance, identity );

			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessage();

				msg.GetEmbed().AddField( "Players", "" + instance.FormatSteamWebhook() + " has joined the server.", false );

				m_Webhook.Post( "PlayerJoin", msg );
			}
		}

		#ifdef COT_DEBUGLOGS
		Print( "-COTModule::OnInvokeConnect - " + identity.GetId() );
		#endif
	}

	/**
	 * See: ClientReconnectEventTypeID
	 */
	override void OnClientReconnect( PlayerBase player, PlayerIdentity identity )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnClientReconnect - " + identity.GetId() );
		#endif

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = player;
		}

		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnClientReconnect - " + identity.GetId() );
		#endif
	}

	/**
	 * See: ClientDisconnectedEventTypeID
	 */
	override void OnClientLogout( PlayerBase player, PlayerIdentity identity, int logoutTime, bool authFailed )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnClientLogout - " + identity.GetId() );
		#endif

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = player;
		}

		#ifdef COT_DEBUGLOGS
		Print( "-COTModule::OnClientLogout - " + identity.GetId() );
		#endif
	}

	/**
	 * See: MissionServer::PlayerDisconnected - Fires when the player has disconnected from the server (OnClientReconnect won't fire)
	 */
	override void OnClientDisconnect( PlayerBase player, PlayerIdentity identity, string uid )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnClientDisconnect - " + uid );
		#endif

		Assert_Null( GetPermissionsManager() );

		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientDisconnected( uid, instance ) )
		{
			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessage();

				msg.GetEmbed().AddField( "Players", "" + instance.FormatSteamWebhook() + " has left the server.", false );

				m_Webhook.Post( "PlayerLeave", msg );
			}

			GetCommunityOnlineToolsBase().RemoveClient( uid );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-COTModule::OnClientDisconnect - " + uid );
		#endif
	}

	/**
	 * See: LogoutCancelEventTypeID
	 */
	override void OnClientLogoutCancelled( PlayerBase player )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnClientLogoutCancelled" );
		#endif

		Assert_Null( GetPermissionsManager() );
		Assert_Null( player );
		Assert_Null( player.GetIdentity() );

		#ifdef COT_DEBUGLOGS
		Print( "  COTModule::OnClientLogoutCancelled - " + player.GetIdentity().GetId() );
		#endif

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( player.GetIdentity().GetId() );

		Assert_Null( instance );
		if ( instance )
		{
			instance.PlayerObject = player;
		}

		#ifdef COT_DEBUGLOGS
		Print( "-COTModule::OnClientLogoutCancelled" );
		#endif
	}
};
