class COTModule : Module
{
	static ref COTModule COTInstance;

	protected ref COTMenu m_COTMenu;

	protected bool m_PreventOpening; 
	protected bool m_ForceHUD;

	protected string m_Version;

	void COTModule( string version )
	{
		COTInstance = this;

		m_Version = version;

		MakeDirectory( ROOT_COT_DIR );

		GetPermissionsManager().RegisterPermission( "COT.View" );
	}

	void ~COTModule()
	{
		CloseMenu( false );

		delete m_COTMenu;
	}

	override void OnMissionLoaded()
	{
		if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{
			if ( m_COTMenu == NULL )
			{
				m_COTMenu = new ref COTMenu;
				m_COTMenu.Init( m_Version );
			}
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterKeyMouseBinding( new KeyMouseBinding( "ToggleMenu",		"UACOTModuleToggleMenu",	true 	) );
		RegisterKeyMouseBinding( new KeyMouseBinding( "FocusGame",		"UACOTModuleFocusGame",		true 	) );
		RegisterKeyMouseBinding( new KeyMouseBinding( "FocusUI",		"UACOTModuleFocusUI",		true 	) );
		RegisterKeyMouseBinding( new KeyMouseBinding( "ToggleCOT",		"UACOTModuleToggleCOT",		false 	) );
		RegisterKeyMouseBinding( new KeyMouseBinding( "CloseCOT",		"UAUIBack",					true 	) );
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

	void ShowMenu( bool force, bool checkForPerms = true )
	{
		if ( m_PreventOpening ) 
		{
			if ( !force )
			{
				return;
			}
		}

		if ( checkForPerms && !GetPermissionsManager().HasPermission( "COT.View" ) )
			return;
			
		m_COTMenu.Show();
	}

	void CloseMenu( bool prevent )
	{
		m_COTMenu.Hide();

		m_PreventOpening = prevent;
	}

	void ToggleMenu( UAInput input = NULL )
	{
		if ( input != NULL && !( input.LocalPress() ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
			return;

		if ( !COTIsActive )
		{
			Message( GetPlayer(), "Community Online Tools is currently toggled off." );
			return;
		}

		if ( m_COTMenu.IsVisible() )
		{
			CloseMenu( false );
		} else
		{
			ShowMenu( false, false );
		}
	}

	void FocusUI( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( m_COTMenu == NULL )
			return;
		
		if( m_COTMenu.IsVisible() )
		{
			m_COTMenu.SetInputFocus( false );
		}
	}

	void FocusGame( UAInput input )
	{
		if ( !( input.LocalRelease() ) )
			return;
			
		if ( m_COTMenu == NULL )
			return;

		if( m_COTMenu.IsVisible() )
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

		COTIsActive = !COTIsActive;

		string message = "Community Online Tools has been toggled ";
		string color;

		if (COTIsActive) {
			message += "on.";
			color = "colorFriendly";
		} else {
			message += "off.";
			color = "colorImportant";
		}
		
		GetPlayer().Message(message, color);
	}

	void CloseCOT( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( m_COTMenu.IsVisible() )
		{
			CloseMenu( false );
		}
	}
}

static void COTForceHud( bool enable )
{
	COTModule.COTInstance.COTForceHud( enable );
}

static void ShowCOTMenu( bool force = false )
{
	COTModule.COTInstance.ShowMenu( force );
}

static void CloseCOTMenu( bool prevent = false )
{
	COTModule.COTInstance.CloseMenu( prevent );
}

static void ToggleCOTMenu()
{
	COTModule.COTInstance.ToggleMenu();
}