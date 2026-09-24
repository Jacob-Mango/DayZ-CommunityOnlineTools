//! #define scope in Enforce is per-file, NOT per compiled module - a file in
//! the same 4_World module (e.g. CommunityOnlineToolsBase.c) defining this
//! does NOT make it visible here. Every file that wants COT_DBG output needs
//! its own copy of this line. Confirmed by zero COT_DBG output from this
//! file for an entire debug session despite the ifdefs compiling in fine.
#define COT_DEBUGLOGS

class COTModule : JMModuleBase
{
	protected JMCOTSideBar m_COTMenu;
	protected bool m_WasVisible;
	protected bool m_ForceHUD;
	protected bool m_GameActive;
	protected bool m_LeftMouseDown;

	//! How often open sessions are written through, in milliseconds.
	//!
	//! Playtime is only folded into the total on disconnect, so without this a
	//! server crash loses every connected player's session outright - and a
	//! crash is exactly when a server has been up longest. Five minutes bounds
	//! that loss without writing a file per player any more often than a role
	//! change already does.
	static const int STATS_FLUSH_INTERVAL_MS = 300000;

	//! Own down/up edge tracking for the sidebar toggle, polled from raw
	//! hardware KeyState() in OnUpdate() instead of going through the
	//! UAInput/CF Bind() dispatch system (see PollToggleMenuKey()). Two
	//! LocalPress()/LocalHold()-based attempts before this one both still
	//! misfired: ForceDisable(true)/(false) - which every context menu and
	//! list-row widget cycles on focus change via UIActionBase::Update - does
	//! not just gate whether our callback gets invoked, it gates what the
	//! UAInput itself REPORTS. So if Y was already held down when some
	//! unrelated widget force-disabled inputs, the read on re-enable comes
	//! back as a fresh "press" no matter which Local*() method or how the
	//! edge bookkeeping is done - any state derived from that UAInput is
	//! equally poisoned. Raw KeyState() reads the physical key directly and
	//! is not part of COT's own ForceDisable plumbing, so it can't be
	//! perturbed by a module or context menu opening/closing.
	protected bool m_ToggleMenuKeyDown;

	//! Same reasoning as m_ToggleMenuKeyDown: Escape used to go through
	//! CloseCOT() (Bind()'d to UAUIBack), which is exactly the ForceDisable-
	//! poisoned path the Y fix moved away from. Polled raw instead so it can't
	//! misfire from a context menu or module window churning input-disable
	//! state, and so it can implement the close priority below instead of the
	//! single global toggle UAUIBack gave it.
	protected bool m_EscapeKeyDown;

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
		//! Bug history: "(m_COTMenu != NULL).ToString()" is fine on its own,
		//! but ".ToString()" on the PARENTHESIZED "&&" expression itself -
		//! "(m_COTMenu && m_COTMenu.IsShown()).ToString()" - crashed the real
		//! engine natively at boot every time (SetMenuState is a COT_ON_OPEN
		//! listener, invoked unconditionally from OnStart()), despite passing
		//! the custom lint tool and despite the plain-bool fields above being
		//! fine. Precomputing into plain bool locals first avoids the pattern
		//! entirely instead of relying on '.ToString()' placement.
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

			#ifndef CF_WINDOWS
			if ( !JMStatics.WINDOWS_CONTAINER )
			{
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
			}
			#endif

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

		#ifndef CF_MODULE_PERMISSIONS
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( FlushPlayerStats, STATS_FLUSH_INTERVAL_MS, true );
		#endif
		}
	}

	//! Creates the "everyone"/"admin" default roles if they don't exist yet.
	//!
	//! Split out of OnMissionLoaded() so callers that cannot rely on module
	//! lifecycle ordering (e.g. JM_COT_AUTOTEST, which runs from
	//! CommunityOnlineTools.OnStart() and previously found "everyone" missing
	//! when it ran ahead of this module's own OnMissionLoaded) can force
	//! creation deterministically. Idempotent - safe to call more than once.
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
		//! UACOTToggleButtons ("Y") is intentionally NOT bound here anymore -
		//! see ToggleMenu()/OnUpdate() below for why it's polled from raw
		//! hardware state instead of the CF dispatch system.
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
			#ifdef CF_WINDOWS
			Print("[COT_DBG] OnUpdate compiled branch = CF_WINDOWS (click-outside handled entirely by CF_Windows.OverrideInputState, NOT by our OnMouseDown/ContainsWidget code)");
			#else
			Print("[COT_DBG] OnUpdate compiled branch = legacy OnMouseDown/UpdateMouseControls polling");
			#endif
		}
		#endif

		PollToggleMenuKey();
		PollEscapeKey();

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
					#ifdef COT_DEBUGLOGS
					Print("[COT_DBG] CF_WINDOWS: sidebar became visible -> OverrideInputState(true, WINDOW)");
					#endif
				}

				CF_Windows.OverrideInputState(true, CF_WindowsFocusState.WINDOW);
			} else if (!m_COTMenu.IsVisible() && m_WasVisible)
			{
				m_WasVisible = false;
				#ifdef COT_DEBUGLOGS
				Print("[COT_DBG] CF_WINDOWS: sidebar no longer visible -> OverrideInputState(false)");
				#endif
				CF_Windows.OverrideInputState(false);
			}
			#else
			//! Not just "does a click close COT" - this is also what hands
			//! mouse/camera control back to the game (ResetGameFocus +
			//! ShowUICursor(false)) so right-click-drag can look around while
			//! the sidebar stays open. Removing it broke free-look entirely,
			//! not just the false-positive closes. NEVER calls SetOpen() -
			//! it only ever touches game/UI focus, so it cannot be the cause
			//! of the sidebar's m_IsOpen actually flipping to false; that is
			//! tracked separately (see CommunityOnlineToolsBase.SetOpen's
			//! DumpStackString instrumentation).
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
	#endif

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

		//! Only Y (ToggleMenu/UACOTToggleButtons) is allowed to close the
		//! sidebar - UAUIBack (vanilla Escape/back) used to also SetOpen(false)
		//! here, which closed the whole sidebar just for backing out of e.g.
		//! the object spawner menu. Still cleans up input-disable state for
		//! any open/closing windows, which is not "closing the sidebar".
		if (GetCommunityOnlineToolsBase())
		{
			if (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() || GetCOTWindowManager().PendingDeletionCount() > 0)
				CommunityOnlineToolsBase.ForceDisableInputs(false);
		}
	}

	//! Client-only, called every frame from OnUpdate(). Bypasses Bind() /
	//! ForceDisableInputs entirely - see m_ToggleMenuKeyDown's comment for why.
	void PollToggleMenuKey()
	{
		if ( !g_Game )
			return;

		//! Same guard CF_InputBindings uses for UAUIMenu: don't treat typing
		//! "y" into a chat/name/message box as the sidebar hotkey.
		Widget focus = GetFocus();
		if ( focus && ( focus.IsInherited( EditBoxWidget ) || focus.IsInherited( MultilineEditBoxWidget ) ) && focus.IsVisible() )
			return;

		bool keyDown = KeyState( KeyCode.KC_Y ) > 0;
		bool isEdge = keyDown && !m_ToggleMenuKeyDown;
		m_ToggleMenuKeyDown = keyDown;

		if ( !isEdge )
			return;

		ToggleMenu();
	}

	void ToggleMenu()
	{
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] ToggleMenu fired (raw KC_Y press)");
		#endif

		GetCommunityOnlineToolsBase().ToggleOpen();
	}

	//! Client-only, called every frame from OnUpdate().
	void PollEscapeKey()
	{
		if ( !g_Game )
			return;

		bool keyDown;
		if (KeyState( KeyCode.KC_ESCAPE ) > 0)
			keyDown = true;
		bool isEdge = keyDown && !m_EscapeKeyDown;
		m_EscapeKeyDown = keyDown;

		if ( !isEdge )
			return;

		HandleEscape();
	}

	//! Priority: pop up (confirmation / context menu / dropdown / value
	//! prompt) closes first, then the frontmost module window, then COT's
	//! sidebar itself - each tier only acts if the one before it had nothing
	//! to close. Leaves Escape alone entirely (falls through to the vanilla
	//! pause menu etc.) when COT has nothing open at all.
	void HandleEscape()
	{
		bool cotOpen = GetCommunityOnlineToolsBase() && GetCommunityOnlineToolsBase().IsOpen();

		#ifndef CF_WINDOWS
		bool windowsActive = GetCOTWindowManager().HasAnyActive();

		if ( !cotOpen && !windowsActive )
			return;

		JMWindowBase topWindow = GetCOTWindowManager().GetTopActive();

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
		#else
		if ( !cotOpen )
			return;
		#endif

		if ( cotOpen )
			ToggleMenu();
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

	//! Decides whether a click should hand mouse/camera control back to the
	//! game (m_GameActive=true) or leave it with COT's UI. NEVER closes the
	//! sidebar itself (never touches SetOpen/m_IsOpen) - it only changes
	//! game focus/cursor visibility. A click misclassified as "outside COT"
	//! here looks like the sidebar closing because the UI becomes unusable,
	//! but m_IsOpen stays true the whole time.
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

			// Check sidebar root + all category flyouts in one call
			if ( m_COTMenu && m_COTMenu.ContainsWidget( clickedWidget ) )
			{
				canContinue = true;
				#ifdef COT_DEBUGLOGS
				Print("[COT_DBG] OnMouseDown canContinue=true via m_COTMenu.ContainsWidget");
				#endif
			}

			if ( !canContinue )
			{
				Widget parentWidget = clickedWidget;
				while ( parentWidget != NULL )
				{
					if ( GetCOTWindowManager().GetWindowFromWidget( parentWidget ) )
					{
						canContinue = true;
						#ifdef COT_DEBUGLOGS
						Print("[COT_DBG] OnMouseDown canContinue=true via GetWindowFromWidget at " + parentWidget.GetName() + " (" + parentWidget.ClassName() + ")");
						#endif
						break;
					}

					if ( JMStatics.ESP_CONTAINER && JMStatics.ESP_CONTAINER == parentWidget )
					{
						canContinue = true;
						#ifdef COT_DEBUGLOGS
						Print("[COT_DBG] OnMouseDown canContinue=true via ESP_CONTAINER match");
						#endif
						break;
					}

					if ( JMStatics.IsOverlay( parentWidget ) )
					{
						canContinue = true;
						#ifdef COT_DEBUGLOGS
						Print("[COT_DBG] OnMouseDown canContinue=true via IsOverlay at " + parentWidget.GetName() + " (" + parentWidget.ClassName() + ")");
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
	#endif

	void ToggleCOT( UAInput input )
	{
		if ( !( input.LocalPress() ) )
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
