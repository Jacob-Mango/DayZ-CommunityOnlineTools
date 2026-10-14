class COTModule : JMModuleBase
{
	protected JMCOTSideBar m_COTMenu;
	protected bool m_WasVisible;
	protected bool m_ForceHUD;
	protected bool m_GameActive;
	protected bool m_LeftMouseDown;

	//! How often open sessions are written through, in milliseconds.
	//! Playtime is only folded in on disconnect, so without this a server crash
	//! loses every connected player's session. Five minutes bounds that loss.
	static const int STATS_FLUSH_INTERVAL_MS = 300000;

	void COTModule()
	{
		MakeDirectory( JMConstants.DIR_COT );

		JMScriptInvokers.COT_ON_OPEN.Insert( SetMenuState );

		JMPermissions.Register( JMConstants.PERM_COT_VIEW );
	}

	void ~COTModule()
	{
		JMScriptInvokers.COT_ON_OPEN.Remove( SetMenuState );

		if ( m_COTMenu )
		{
			m_COTMenu.Destroy();
		}
	}

	void SetMenuState( bool show )
	{
		bool hasCotMenu = m_COTMenu != NULL;
		bool cotMenuShown = false;
		if ( hasCotMenu )
			cotMenuShown = m_COTMenu.IsShown();

		#ifdef COT_DEBUGLOGS
		//! ".ToString()" on a parenthesized "&&" expression, e.g.
		//! "(m_COTMenu && m_COTMenu.IsShown()).ToString()", natively crashes the
		//! engine at boot (lint does not catch it). Precompute plain bool locals.
		Print("[COT_DBG] SetMenuState(" + show.ToString() + ") - COT_ON_OPEN listener, m_COTMenu=" + hasCotMenu.ToString() + " IsShown=" + cotMenuShown.ToString());
		#endif

		if ( !m_COTMenu )
			return;

		if ( show )
		{
			if ( !m_COTMenu.IsShown() )
			{
				m_COTMenu.Show();
			}
		} else {
			if ( m_COTMenu.IsShown() )
				m_COTMenu.Hide();
		}
	}

	override void OnMissionLoaded()
	{
		auto trace = CF_Trace_0(this, "OnMissionLoaded");

		super.OnMissionLoaded();

		if ( IsMissionClient() )
		{
			if ( !JMStatics.ESP_CONTAINER )
			{
				JMStatics.ESP_CONTAINER = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_esp.layout", NULL );
			}

			if ( !JMStatics.WINDOWS_CONTAINER )
			{
				JMStatics.WINDOWS_CONTAINER = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen_windows.layout", NULL );

				//! Keeps its default sort on purpose: MapWidget/ItemPreviewWidget
				//! render nothing under a high-sort ancestor (SORT_WINDOW blanked
				//! every COT map/preview). If the HUD draws over windows again, pick
				//! a sort just above the HUD, not SORT_WINDOW.
			}

			if ( m_COTMenu == NULL )
			{
				Widget sidebarW = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_menu.layout" );
				if (sidebarW)
					sidebarW.GetScript( m_COTMenu );
			}
		}

		if ( IsMissionHost() )
		{
			EnsureDefaultRoles();

			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( FlushPlayerStats, STATS_FLUSH_INTERVAL_MS, true );
		}
	}

	//! Creates the "everyone"/"admin" default roles if they don't exist yet.
	//! Public so callers that can't rely on module lifecycle order (e.g.
	//! JM_COT_AUTOTEST from OnStart()) can force it. Idempotent.
	static void EnsureDefaultRoles()
	{
		GetPermissionsManager().CreateRole( "everyone" );
		GetPermissionsManager().CreateRoleEx( "admin", JMPermissionType.ALLOW );
	}

	//! Fold the in-progress session of every connected player into their total
	//! and write it out. AccumulateSession re-bases its own start time, so
	//! running this repeatedly does not double-count.
	void FlushPlayerStats()
	{
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
	}

	JMCOTSideBar GetCOTSidebar()
	{
		return m_COTMenu;
	}

	override void OnMissionFinish()
	{
		if ( m_COTMenu )
		{
			m_COTMenu.Destroy();
			m_COTMenu = null;
		}

		//! A graceful shutdown is the one crash-like event that CAN be handled:
		//! close every open session before the mission goes away.
		if ( IsMissionHost() )
		{
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( FlushPlayerStats );
			FlushPlayerStats();
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
	#ifdef COT_DEBUGLOGS
	protected static bool s_JM_LoggedInputBranch = false;
	#endif

	override void OnUpdate( float timeslice )
	{
		#ifdef COT_DEBUGLOGS
		if ( !s_JM_LoggedInputBranch )
		{
			s_JM_LoggedInputBranch = true;
			Print("[COT_DBG] OnUpdate compiled branch = legacy OnMouseDown/UpdateMouseControls polling");
		}
		#endif

		JMStatics.COT_MENU = null;
		if ( m_COTMenu )
		{
			JMStatics.COT_MENU = m_COTMenu.GetLayoutRoot();

			m_COTMenu.OnUpdate( timeslice );

			//! Hands mouse/camera control back to the game on outside clicks, so
			//! free-look works with the sidebar open - removing it breaks free-look.
			//! Only touches game/UI focus, never SetOpen()/m_IsOpen.
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
		else if (m_WasVisible)
		{
			m_WasVisible = false;
		}

		if ( m_ForceHUD )
		{
			g_Game.GetMission().GetHud().Show( false );
		}
	}
	#endif

	void UpdateMouseControls()
	{
		bool isMenuOpen = m_COTMenu && m_COTMenu.IsVisible();
		bool windowsOpen = GetCOTWindowManager().HasAnyActive();

		if ( !isMenuOpen && !windowsOpen )
			return;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] UpdateMouseControls m_GameActive=" + m_GameActive.ToString());
		#endif

		if ( m_GameActive )
		{
			g_Game.GetMission().RemoveActiveInputExcludes({"menu"});
			g_Game.GetUIManager().ShowUICursor( false );
		} else
		{
			g_Game.GetMission().AddActiveInputExcludes({"menu"});
			g_Game.GetUIManager().ShowUICursor( true );
		}
	}

	void COTForceHud( bool enable )
	{
		m_ForceHUD = enable;
		
		if ( !m_ForceHUD )
			g_Game.GetMission().GetHud().Show( true );
	}

	void CloseCOT( UAInput input )
	{
		if (!g_Game)
			return;

		if (!input.LocalPress())
			return;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] CloseCOT fired (UAUIBack press)");
		#endif

		//if (GetCommunityOnlineToolsBase())
		//{
			//if (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() || GetCOTWindowManager().PendingDeletionCount() > 0)
				//COT_Base.ForceDisableInputs(false);
		//}

		//! Priority: pop up (confirmation / context menu / dropdown / value
		//! prompt) closes first, then the frontmost module window, then COT's
		//! sidebar itself - each tier only acts if the one before it had nothing
		//! to close. Leaves UAUIBack alone entirely (falls through to the vanilla
		//! pause menu etc.) when COT has nothing open at all.
		bool cotOpen = GetCommunityOnlineToolsBase() && GetCommunityOnlineToolsBase().IsOpen();

		bool windowsActive = GetCOTWindowManager().HasAnyUnpinnedActive();

		GetCOTBase().CloseSharedMenu();

		if ( !cotOpen && !windowsActive )
			return;

		JMWindowBase topWindow = GetCOTWindowManager().GetTopUnpinnedActive();

		if ( topWindow && topWindow.HasOpenPopup() )
		{
			topWindow.CloseOpenPopup();
			return;
		}

		if ( topWindow )
		{
			if ( topWindow.GetModule() )
				topWindow.GetModule().Close();
			else
				topWindow.Destroy();
			return;
		}

		if ( cotOpen )
			ToggleMenu();
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !input.LocalPress() )
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

	//! Decides whether a click hands mouse/camera control back to the game
	//! (m_GameActive=true) or stays with COT's UI. Never touches m_IsOpen: a
	//! click misclassified as "outside COT" only looks like the sidebar closed.
	void OnMouseDown()
	{
		if ( g_Game.GetUIManager().GetMenu() )
			return;

		if ( m_COTMenu.IsVisible() || GetCOTWindowManager().HasAnyActive() )
		{
			bool canContinue = false;

			Widget clickedWidget = GetWidgetUnderCursor();

			#ifdef COT_DEBUGLOGS
			string clickedName = "NULL";
			if ( clickedWidget )
				clickedName = clickedWidget.GetName() + " (" + clickedWidget.ClassName() + ")";
			Print("[COT_DBG] OnMouseDown clicked=" + clickedName);
			#endif

			if ( !canContinue )
			{
				Widget parentWidget = clickedWidget;
				while ( parentWidget != NULL )
				{
					//! Any widget that doesn't ignore mouse buttons and isn't part of the vanilla HUD
					if ( (parentWidget.GetFlags() & WidgetFlags.IGNOREPOINTER) != WidgetFlags.IGNOREPOINTER && !IsHUDWidget( parentWidget ) )
					{
						canContinue = true;
						#ifdef COT_DEBUGLOGS
						Print("[COT_DBG] OnMouseDown canContinue=true ignorepointer 0 at " + parentWidget.GetName() + " (" + parentWidget.ClassName() + ")");
						#endif
						break;
					}

					parentWidget = parentWidget.GetParent();
				}
			}

			#ifdef COT_DEBUGLOGS
			if ( !canContinue )
				Print("[COT_DBG] OnMouseDown canContinue=false -> treating as click OUTSIDE COT, releasing to game");
			#endif

			if ( !canContinue )
			{
				m_GameActive = true;

				SetFocus( NULL );

				UpdateMouseControls();
			}
		}
	}

	bool IsHUDWidget( Widget widget )
	{
		switch ( widget.GetName() )
		{
			case "CursorIcons":
			case "HudFrameWidget":
			case "HudPanel":
			case "QuickbarGrid":
				return true;
		}

		return false;
	}

	void ToggleCOT( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] ToggleCOT fired (UACOTModuleToggleCOT press)");
		#endif

		if ( m_COTMenu == NULL )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_COT_VIEW ) )
			return;

		GetCommunityOnlineToolsBase().ToggleActive();
	}

	override void OnInvokeConnect( PlayerBase player, PlayerIdentity identity )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+COTModule::OnInvokeConnect - " + identity.GetId() );
		#endif

		auto roles = GetPermissionsManager().Roles;
		foreach ( string roleName, JMRole role: roles)
		{
			GetCommunityOnlineToolsBase().UpdateRole( role, identity );
		}
		
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientConnected( identity, instance ) )
		{
			instance.PlayerObject = player;

			JMPlayerStats connectStats = instance.GetStats();
			if ( connectStats )
			{
				connectStats.OnSessionStart();
				instance.Save();
			}

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
