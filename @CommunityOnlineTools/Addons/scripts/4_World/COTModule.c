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
    }

    override void OnUpdate( float timeslice )
    {
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

        if ( !GetPermissionsManager().HasPermission( "COT.Show", NULL ) )
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