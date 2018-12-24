class COTModule : Module
{
    static ref COTModule COTInstance;

    protected ref COTMenu m_COTMenu;

    protected bool m_PreventOpening; 
    protected bool m_ForceHUD; 

    void COTModule()
    {
        COTInstance = this;

        MakeDirectory( ROOT_COT_DIR );

        GetPermissionsManager().RegisterPermission( "COT.Show" );
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
                m_COTMenu.Init();
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

    void ShowMenu( bool force )
    {
        if ( m_PreventOpening ) 
        {
            if ( !force )
            {
                return;
            }
        }

        if ( !GetPermissionsManager().HasPermission( "COT.Show" ) )
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
        if ( m_COTMenu.IsVisible() )
        {
            CloseMenu( false );
        } else
        {
            ShowMenu( false );
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