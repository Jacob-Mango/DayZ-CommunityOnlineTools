class COTModule : JMModuleBase
{
	protected JMCOTSideBar m_COTMenu;

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
			m_COTMenu.Hide();

			delete m_COTMenu;
		}
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
		
		if ( IsMissionClient() )
		{
			if ( !JMStatics.ESP_CONTAINER )
				JMStatics.ESP_CONTAINER = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_esp.layout", NULL );
			
			if ( !JMStatics.WINDOWS_CONTAINER )
				JMStatics.WINDOWS_CONTAINER = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_windows.layout", NULL );
			
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

			if ( !GetPermissionsManager().RoleExists( "admin" ) )
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
		if ( IsMissionClient() )
		{
			delete m_COTMenu;
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "ToggleMenu",		"UACOTToggleButtons",		true 	) );
		RegisterBinding( new JMModuleBinding( "ToggleCOT",		"UACOTModuleToggleCOT",		false 	) );
		RegisterBinding( new JMModuleBinding( "CloseCOT",		"UAUIBack",					true 	) );
	}

	override void OnUpdate( float timeslice )
	{
		if ( m_COTMenu )
		{
			m_COTMenu.OnUpdate( timeslice );

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

		////Print( "+COTModule::CloseCOT" );

		array< JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();
		for ( int i = 0; i < modules.Count(); ++i )
		{
			//Print( "  +" + modules[i].Type().ToString() );
			//Print( "  visible=" + modules[i].IsVisible() );
			if ( modules[i].IsVisible() )
			{
				modules[i].Hide();
			}
			//Print( "  -" + modules[i].Type().ToString() );
		}

		//Print( "  +SetOpen" );
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Call( GetCommunityOnlineToolsBase().SetOpen, false );

		//Print( "  -SetOpen" );

		////Print( "-COTModule::CloseCOT" );
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !( input.LocalPress() ) )
			return;

		GetCommunityOnlineToolsBase().ToggleOpen();
	}

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

	override void OnInvokeConnect( PlayerBase player, PlayerIdentity identity )
	{
		//Print( "+COTModule::OnInvokeConnect - " + identity.GetId() );

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		for ( int i = 0; i < GetPermissionsManager().RoleCount(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().GetRole( i ), identity );
		}
		
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

		//Print( "-COTModule::OnInvokeConnect - " + identity.GetId() );
	}

	/**
	 * See: ClientReconnectEventTypeID
	 */
	override void OnClientReconnect( PlayerBase player, PlayerIdentity identity )
	{
		//Print( "+COTModule::OnClientReconnect - " + identity.GetId() );

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = player;
		}

		//Print( "+COTModule::OnClientReconnect - " + identity.GetId() );
	}

	/**
	 * See: ClientDisconnectedEventTypeID
	 */
	override void OnClientLogout( PlayerBase player, PlayerIdentity identity, int logoutTime, bool authFailed )
	{
		//Print( "+COTModule::OnClientLogout - " + identity.GetId() );

		Assert_Null( GetPermissionsManager() );
		Assert_Null( identity );

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = player;
		}

		//Print( "-COTModule::OnClientLogout - " + identity.GetId() );
	}

	/**
	 * See: MissionServer::PlayerDisconnected - Fires when the player has disconnected from the server (OnClientReconnect won't fire)
	 */
	override void OnClientDisconnect( PlayerBase player, PlayerIdentity identity, string uid )
	{
		//Print( "+COTModule::OnClientDisconnect - " + uid );

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

		//Print( "-COTModule::OnClientDisconnect - " + uid );
	}

	/**
	 * See: LogoutCancelEventTypeID
	 */
	override void OnClientLogoutCancelled( PlayerBase player )
	{
		//Print( "+COTModule::OnClientLogoutCancelled" );

		Assert_Null( GetPermissionsManager() );
		Assert_Null( player );
		Assert_Null( player.GetIdentity() );

		Print( "  COTModule::OnClientLogoutCancelled - " + player.GetIdentity().GetId() );

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( player.GetIdentity().GetId() );

		Assert_Null( instance );
		if ( instance )
		{
			instance.PlayerObject = player;
		}

		//Print( "-COTModule::OnClientLogoutCancelled" );
	}
}