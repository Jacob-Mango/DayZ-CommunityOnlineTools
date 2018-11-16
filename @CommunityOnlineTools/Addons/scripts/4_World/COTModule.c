class COTModule : Module
{
    protected ref EditorMenu m_EditorMenu;

    void COTModule()
    {
        GetRPCManager().AddRPC( "COT", "RequestPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT", "ReceivePermissions", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "COT.Show" );
    }

    void ~COTModule()
    {
    }

    override void OnMissionLoaded()
    {
        if ( GetGame().IsClient() )
        {
            GetRPCManager().SendRPC( "COT", "RequestPermissions", new Param, true );

        }
    }

    override void RegisterKeyMouseBindings() 
    {
        KeyMouseBinding toggleEditor = new KeyMouseBinding( GetModuleType(), "ToggleEditor", "[Y]", "Opens the editor." );
        toggleEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );
        RegisterKeyMouseBinding( toggleEditor );

        KeyMouseBinding closeEditor = new KeyMouseBinding( GetModuleType(), "CloseEditor", "[ESCAPE]", "Closes the editor." );
        closeEditor.AddKeyBind( KeyCode.KC_ESCAPE, KeyMouseBinding.KB_EVENT_RELEASE );
        RegisterKeyMouseBinding( closeEditor );
    }

    override void OnUpdate( float timeslice )
    {
        /*
		if( GetGame().GetInput().GetActionDown( UAUIOpenCOT, false ) )
		{
            ShowCOMEditor();
        }
        */
    }

    void CloseEditor()
    {
        if ( m_EditorMenu && m_EditorMenu.IsVisible() )
        {
            m_EditorMenu.Hide();
        }
    }

    void ToggleEditor()
    {
        if ( !GetPermissionsManager().HasPermission( "COT.Show", NULL ) )
            return;

        if ( m_EditorMenu == NULL )
        {
            m_EditorMenu = new ref EditorMenu;
            m_EditorMenu.Init();
        }

        if ( m_EditorMenu.IsVisible() )
        {
            m_EditorMenu.Hide();
        } else
        {
            m_EditorMenu.Show();
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