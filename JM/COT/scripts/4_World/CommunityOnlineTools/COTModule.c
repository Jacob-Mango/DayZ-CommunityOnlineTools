class COTModule : JMModuleBase
{
	static ref COTModule COTInstance;

	protected ref JMCOTSideBar m_COTMenu;

	protected bool m_ForceHUD;

	void COTModule()
	{
		COTInstance = this;

		MakeDirectory( JMConstants.DIR_COT );

		CommunityOnlineToolsBase.SI_OPEN.Insert( SetMenuState );

		GetPermissionsManager().RegisterPermission( "COT.View" );
	}

	void ~COTModule()
	{
		m_COTMenu.Hide();

		CommunityOnlineToolsBase.SI_OPEN.Remove( SetMenuState );

		delete m_COTMenu;
	}

	override void OnMissionLoaded()
	{
		if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{
			if ( m_COTMenu == NULL )
			{
				m_COTMenu = new JMCOTSideBar;
				m_COTMenu.Init();
			}
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "ToggleMenu",		"UACOTModuleToggleMenu",	true 	) );
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

	void ShowMenu( bool force, bool checkForPerms = true )
	{
		if ( checkForPerms && !GetPermissionsManager().HasPermission( "COT.View" ) )
			return;
			
		m_COTMenu.Show();
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
		
		if ( m_COTMenu.IsVisible() )
		{
			m_COTMenu.SetInputFocus( false );
		}
	}

	void FocusGame( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;
			
		if ( m_COTMenu == NULL )
			return;

		if ( m_COTMenu.IsVisible() )
		{
			Widget w = GetWidgetUnderCursor();

			if ( w.GetName() != "Windows" && w.GetName() != "map_editor_menu" )
			{
				return;
			}

			m_COTMenu.SetInputFocus( true );
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

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools has been toggled on." );
		} else
		{
			CreateLocalAdminNotification( "Community Online Tools has been toggled off." );
		}
	}
}