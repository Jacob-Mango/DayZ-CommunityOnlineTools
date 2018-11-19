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
            GetRPCManager().SendRPC( "COT", "RequestPermissions", new Param, true );

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

    void ReceivePermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetGame().IsMultiplayer() )
            return;

        if ( type == CallType.Client )
        {
            ref Param1< ref PlayerData > data;
            if ( !ctx.Read( data ) ) return;

            ClientAuthPlayer = DeserializePlayer( data.param1 );
        }
    }

    void RequestPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {        
        if ( !GetGame().IsMultiplayer() )
            return;

        if ( type == CallType.Server )
        {
            ref array< ref AuthPlayer > players = GetPermissionsManager().GetPlayers();

            ref AuthPlayer player = NULL;

            for ( int i = 0; i < players.Count(); i++ )
            {
                if ( players[i].GetGUID() == sender.GetId() )
                {
                    player = players[i];
                }
            }

            if ( player )
            {
                if ( GetGame().IsMultiplayer() )
                {
                    GetRPCManager().SendRPC( "COT", "ReceivePermissions", new Param1< ref PlayerData >( SerializePlayer( player ) ), true, sender );
                } else
                {
                    ClientAuthPlayer = player;
                }
            }
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