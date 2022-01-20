[CF_RegisterModule(COTSideBarModule)]
class COTSideBarModule : CF_ModuleWorld
{
	protected JMCOTSideBar m_COTMenu;

	protected bool m_WasVisible;

	protected bool m_ForceHUD;

	protected bool m_GameActive;

	protected bool m_LeftMouseDown;

	override void OnInit()
	{
		super.OnInit();
		
		MakeDirectory( JMConstants.DIR_COT );

		JMScriptInvokers.COT_ON_OPEN.Insert( SetMenuState );

		GetPermissionsManager().RegisterPermission( "COT.View" );
		
		Bind("ToggleMenu",		"UACOTToggleButtons",		true 	 );
		Bind("ToggleCOT",		"UACOTModuleToggleCOT",		false 	 );
		Bind("CloseCOT",		"UAUIBack",					true 	 );
	}

	void ~COTSideBarModule()
	{
		JMScriptInvokers.COT_ON_OPEN.Remove( SetMenuState );

		if ( m_COTMenu )
		{
			m_COTMenu.Hide();

			delete m_COTMenu;
		}
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
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

	override void OnMissionFinish(Class sender, CF_EventArgs args)
	{
		if (IsMissionClient())
		{
			delete m_COTMenu;
		}
	}

	override void OnUpdate(Class sender, CF_EventArgs args)
	{
		JMStatics.COT_MENU = null;
		if ( m_COTMenu )
		{
			JMStatics.COT_MENU = m_COTMenu.GetLayoutRoot();

			m_COTMenu.OnUpdate(CF_EventUpdateArgs.Cast(args).DeltaTime);

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
		}
		else if (m_WasVisible)
		{
			m_WasVisible = false;
			
			CF_Windows.OverrideInputState(false);
		}

		if ( m_ForceHUD )
		{
			GetGame().GetMission().GetHud().Show( false );
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
		if (!(input.LocalPress()))
			return;

		foreach (auto module : COTModule.s_All)
		{
			if (module.IsVisible())
			{
				module.Hide();
			}
		}

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Call(GetCommunityOnlineToolsBase().SetOpen, false);
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !( input.LocalPress() ) )
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

	override void OnInvokeConnect(Class sender, CF_EventArgs args)
	{
		auto cArgs = CF_EventPlayerArgs.Cast(args);
		
		Assert_Null( GetPermissionsManager() );

		#ifdef CF_MODULE_PERMISSIONS
		for ( int i = 0; i < GetPermissionsManager().RoleCount(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().GetRole( i ), cArgs.Identity );
		}
		#else
		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			GetCommunityOnlineToolsBase().UpdateRole( GetPermissionsManager().Roles.GetElement(i), cArgs.Identity );
		}
		#endif
		
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientConnected( cArgs.Identity, instance ) )
		{
			instance.PlayerObject = cArgs.Player;

			GetCommunityOnlineToolsBase().SetClient( instance, cArgs.Identity );

			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessage();

				msg.GetEmbed().AddField( "Players", "" + instance.FormatSteamWebhook() + " has joined the server.", false );

				m_Webhook.Post( "PlayerJoin", msg );
			}
		}

		//Print( "-COTSideBarModule::OnInvokeConnect - " + identity.GetId() );
	}

	/**
	 * See: ClientReconnectEventTypeID
	 */
	override void OnClientReconnect(Class sender, CF_EventArgs args)
	{
		//Print( "+COTSideBarModule::OnClientReconnect - " + identity.GetId() );

		Assert_Null( GetPermissionsManager() );
		
		auto cArgs = CF_EventPlayerArgs.Cast(args);

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( cArgs.Identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = cArgs.Player;
		}

		//Print( "+COTSideBarModule::OnClientReconnect - " + identity.GetId() );
	}

	/**
	 * See: ClientDisconnectedEventTypeID
	 */
	override void OnClientLogout(Class sender, CF_EventArgs args)
	{
		//Print( "+COTSideBarModule::OnClientLogout - " + identity.GetId() );

		Assert_Null( GetPermissionsManager() );
		
		auto cArgs = CF_EventPlayerDisconnectedArgs.Cast(args);

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( cArgs.Identity.GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = cArgs.Player;
		}

		//Print( "-COTSideBarModule::OnClientLogout - " + identity.GetId() );
	}

	/**
	 * See: MissionServer::PlayerDisconnected - Fires when the player has disconnected from the server (OnClientReconnect won't fire)
	 */
	override void OnClientDisconnect(Class sender, CF_EventArgs args)
	{
		//Print( "+COTSideBarModule::OnClientDisconnect - " + uid );

		Assert_Null( GetPermissionsManager() );
		
		auto cArgs = CF_EventPlayerDisconnectedArgs.Cast(args);

		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientDisconnected( cArgs.UID, instance ) )
		{
			if ( m_Webhook )
			{
				auto msg = m_Webhook.CreateDiscordMessage();

				msg.GetEmbed().AddField( "Players", "" + instance.FormatSteamWebhook() + " has left the server.", false );

				m_Webhook.Post( "PlayerLeave", msg );
			}

			GetCommunityOnlineToolsBase().RemoveClient( cArgs.UID );
		}

		//Print( "-COTSideBarModule::OnClientDisconnect - " + uid );
	}

	/**
	 * See: LogoutCancelEventTypeID
	 */
	override void OnClientLogoutCancelled(Class sender, CF_EventArgs args)
	{
		//Print( "+COTSideBarModule::OnClientLogoutCancelled" );

		Assert_Null( GetPermissionsManager() );
		
		auto cArgs = CF_EventPlayerDisconnectedArgs.Cast(args);

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( cArgs.Player.GetIdentity().GetId() );

		if ( !Assert_Null( instance ) )
		{
			instance.PlayerObject = cArgs.Player;
		}

		//Print( "-COTSideBarModule::OnClientLogoutCancelled" );
	}
}