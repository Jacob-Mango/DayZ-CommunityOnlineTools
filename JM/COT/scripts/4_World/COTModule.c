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
		KeyMouseBinding toggleEditor = new KeyMouseBinding( GetModuleType(), "ToggleMenu", "Opens the editor.", true );
		toggleEditor.AddBinding( "kY" );
		RegisterKeyMouseBinding( toggleEditor );

		KeyMouseBinding focusGame = new KeyMouseBinding( GetModuleType(), "FocusGame", "Focuses the game instead of the UI while in editor.", true );
		focusGame.AddBinding( "mBLeft" );
		focusGame.SetActionType( KeyMouseActionType.PRESS );
		RegisterKeyMouseBinding( focusGame );

		KeyMouseBinding focusUI = new KeyMouseBinding( GetModuleType(), "FocusUI", "Focuses the UI instead of the game while in editor.", true );
		focusUI.AddBinding( "mBLeft" );
		focusUI.SetActionType( KeyMouseActionType.RELEASE );
		RegisterKeyMouseBinding( focusUI );

		KeyMouseBinding toggleCOT = new KeyMouseBinding( GetModuleType(), "ToggleCOT", "Toggles the ability to use COT features.", false );
		toggleCOT.AddBinding( "kEnd" );
		RegisterKeyMouseBinding( toggleCOT );

		KeyMouseBinding closeCOT = new KeyMouseBinding( GetModuleType(), "CloseCOT", "Closes COT.", true );
		closeCOT.AddBinding( "kEscape" );
		RegisterKeyMouseBinding( closeCOT );
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

	void ToggleMenu()
	{
		if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
		{
			return;
		}

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

	void FocusUI()
	{
		if ( m_COTMenu == NULL ) return;
		
		if( m_COTMenu.IsVisible() )
		{
			m_COTMenu.SetInputFocus( false );
		}
	}

	void FocusGame()
	{
		if ( m_COTMenu == NULL ) return;

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

	void ToggleCOT()
	{
		if ( m_COTMenu == NULL ) return;

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

	void CloseCOT()
	{
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