class COTModule : Module
{
    static ref COTModule COTInstance;

    protected ref COTMenu m_COTMenu;

    protected bool m_PreventOpening; 
    protected bool m_ForceHUD; 

    void COTModule()
    {
        COTInstance = this;

        GetRPCManager().AddRPC( "COT", "RequestPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT", "ReceivePermissions", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "COT.Show" );

        if ( GetGame().IsClient() )
        {
            GetRPCManager().SendRPC( "COT", "RequestPermissions", new Param, true );
        }
    }

    void ReceivePermissions( CallType type, ref ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        Print("Setting permissions!");
        if ( type == CallType.Client )
        {
            Param1< ref PlayerData > data;
            if ( !ctx.Read( data ) ) return;

            ClientAuthPlayer = DeserializePlayer( data.param1 );

            ClientAuthPlayer.DebugPrint();
        }
    }

    void RequestPermissions( CallType type, ref ParamsReadContext ctx, PlayerIdentity sender, Object target )
    {
        Print("RequestPermissions");
        if ( type == CallType.Server )
        {
            GetRPCManager().SendRPC( "COT", "ReceivePermissions", new Param1< ref PlayerData >( SerializePlayer( GetPermissionsManager().GetPlayerByIdentity( sender ) ) ), true, sender );
        }
    }

    void ~COTModule()
    {
        CloseMenu( false );

        delete m_COTMenu;
    }

    override void OnMissionLoaded()
    {
        if ( GetGame().IsClient() )
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
        KeyMouseBinding toggleEditor = new KeyMouseBinding( GetModuleType(), "ToggleMenu", "[Y]", "Opens the editor." );
        toggleEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );
        RegisterKeyMouseBinding( toggleEditor );
    }

    override void OnUpdate( float timeslice )
    {
        /*
		if( GetGame().GetInput().GetActionDown( UAUIOpenCOT, false ) )
		{
            ToggleEditor();
        }
        */

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