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
			Print("[COT-TRACE] COTModule.OnMissionLoaded: client setup begin");
			if ( !JMStatics.ESP_CONTAINER )
			{
				Print("[COT-TRACE] COTModule: create screen_esp.layout");
				JMStatics.ESP_CONTAINER = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_esp.layout", NULL );
				Print("[COT-TRACE] COTModule: screen_esp.layout done");
			}

			#ifndef CF_WINDOWS
			if ( !JMStatics.WINDOWS_CONTAINER )
			{
				Print("[COT-TRACE] COTModule: create screen_windows.layout");
				JMStatics.WINDOWS_CONTAINER = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_windows.layout", NULL );

				//! The container deliberately keeps its default sort.
				//!
				//! MapWidget and ItemPreviewWidget render nothing when an
				//! ancestor carries a high sort, and this container used to be
				//! set to JMUILayout.SORT_WINDOW (920) - which is why every COT
				//! item preview and every COT map was blank everywhere, no
				//! matter what its own layout or style said. The base game has
				//! no counter-example: its one workspace-hosted preview,
				//! PluginItemDiagnostic, sits at priority 5.
				//!
				//! The sort was originally raised so the in-game HUD would not
				//! draw over module windows. If that returns, the fix is a sort
				//! high enough to beat the HUD and low enough to keep these
				//! widgets rendering - not SORT_WINDOW.
				Print("[COT-TRACE] COTModule: screen_windows.layout done");
			}
			#endif

			if ( m_COTMenu == NULL )
			{
				Print("[COT-TRACE] COTModule: create sidebar_menu.layout");
				Widget sidebarW = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_menu.layout" );
				Print("[COT-TRACE] COTModule: sidebar_menu.layout created=" + (sidebarW != null).ToString());
				if (sidebarW)
					sidebarW.GetScript( m_COTMenu );
				Print("[COT-TRACE] COTModule: m_COTMenu set=" + (m_COTMenu != null).ToString());
			}
			Print("[COT-TRACE] COTModule.OnMissionLoaded: client setup end");
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

		#ifndef CF_MODULE_PERMISSIONS
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( FlushPlayerStats, STATS_FLUSH_INTERVAL_MS, true );
		#endif
		}
	}

	//! How often open sessions are written through, in milliseconds.
	//!
	//! Playtime is only folded into the total on disconnect, so without this a
	//! server crash loses every connected player's session outright - and a
	//! crash is exactly when a server has been up longest. Five minutes bounds
	//! that loss without writing a file per player any more often than a role
	//! change already does.
	static const int STATS_FLUSH_INTERVAL_MS = 300000;

	//! Fold the in-progress session of every connected player into their total
	//! and write it out. AccumulateSession re-bases its own start time, so
	//! running this repeatedly does not double-count.
	void FlushPlayerStats()
	{
	#ifndef CF_MODULE_PERMISSIONS
		if ( !IsMissionHost() )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		if ( !players )
			return;

		foreach ( JMPlayerInstance instance : players )
		{
			if ( !instance )
				continue;

			JMPlayerStats stats = instance.GetStats();
			if ( !stats )
				continue;

			stats.AccumulateSession();
			instance.Save();
		}
	#endif
	}

	override void OnMissionFinish()
	{
		if ( m_COTMenu )
		{
			m_COTMenu.Destroy();
			m_COTMenu = null;
		}

	#ifndef CF_MODULE_PERMISSIONS
		//! A graceful shutdown is the one crash-like event that CAN be handled:
		//! close every open session before the mission goes away.
		if ( IsMissionHost() )
		{
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( FlushPlayerStats );
			FlushPlayerStats();
		}
	#endif
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
			g_Game.GetMission().GetHud().Show( false );
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
			g_Game.GetInput().ResetGameFocus();
			g_Game.GetUIManager().ShowUICursor( false );
		} else
		{
			g_Game.GetInput().ChangeGameFocus( 1 );
			g_Game.GetUIManager().ShowUICursor( true );
		}
	}
	#endif

	void COTForceHud( bool enable )
	{
		m_ForceHUD = enable;
		
		if ( !m_ForceHUD )
			g_Game.GetMission().GetHud().Show( true );
	}

	void SetMenuState( bool show )
	{
		Print("[COT-TRACE] COTModule.SetMenuState show=" + show.ToString() + " m_COTMenu=" + (m_COTMenu != null).ToString());
		if ( !m_COTMenu )
			return;

		if ( show )
		{
			if ( !m_COTMenu.IsVisible() )
			{
				Print("[COT-TRACE] COTModule.SetMenuState: calling m_COTMenu.Show");
				m_COTMenu.Show();
				Print("[COT-TRACE] COTModule.SetMenuState: m_COTMenu.Show returned");
			}
		} else {
			if ( m_COTMenu.IsVisible() )
				m_COTMenu.Hide();
		}
	}

	void CloseCOT( UAInput input )
	{
		if (!g_Game)
			return;

		if (!input.LocalPress())
			return;

		if (GetCommunityOnlineToolsBase())
		{
			if (GetCommunityOnlineToolsBase().IsOpen())
				g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( GetCommunityOnlineToolsBase().SetOpen, false );

			if (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() || GetCOTWindowManager().PendingDeletionCount() > 0)
				CommunityOnlineToolsBase.ForceDisableInputs(false);
		}
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
		if ( g_Game.GetUIManager().GetMenu() )
			return;

		if ( m_COTMenu.IsVisible() || GetCOTWindowManager().HasAnyActive() )
		{
			bool canContinue = false;

			Widget clickedWidget = GetWidgetUnderCursor();

			// Check sidebar root + all category flyouts in one call
			if ( m_COTMenu && m_COTMenu.ContainsWidget( clickedWidget ) )
				canContinue = true;

			if ( !canContinue )
			{
				Widget parentWidget = clickedWidget;
				while ( parentWidget != NULL )
				{
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

					if ( JMStatics.IsOverlay( parentWidget ) )
					{
						canContinue = true;
						break;
					}

					parentWidget = parentWidget.GetParent();
				}
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

		#ifndef CF_MODULE_PERMISSIONS
			JMPlayerStats connectStats = instance.GetStats();
			if ( connectStats )
			{
				connectStats.OnSessionStart();
				instance.Save();
			}
		#endif

			GetCommunityOnlineToolsBase().SetClient( instance, identity );

			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_INFO );

				msg.GetEmbed().SetTitle( "Player Joined" );
				msg.GetEmbed().SetDescription( instance.FormatSteamWebhook() + " has joined the server." );

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

	#ifndef CF_MODULE_PERMISSIONS
		//! Before OnClientDisconnected below, which drops the instance: closing
		//! the session afterwards would have nothing left to close.
		JMPlayerInstance leaving = GetPermissionsManager().GetPlayer( uid );
		if ( leaving )
		{
			JMPlayerStats leaveStats = leaving.GetStats();
			if ( leaveStats )
			{
				leaveStats.OnSessionEnd();
				leaving.Save();
			}
		}
	#endif

		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientDisconnected( uid, instance ) )
		{
			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessageColored( JMConstants.WEBHOOK_COLOR_NEUTRAL );

				msg.GetEmbed().SetTitle( "Player Left" );
				msg.GetEmbed().SetDescription( instance.FormatSteamWebhook() + " has left the server." );

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
}
