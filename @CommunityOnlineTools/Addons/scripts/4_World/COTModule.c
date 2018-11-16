class COTModule : Module
{
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
        KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType(), "ShowCOMEditor", "[Y]", "Opens the COM Editor." );
    
        toggleCOMEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );

        RegisterKeyMouseBinding( toggleCOMEditor );
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

    void ShowCOMEditor()
    {
        if ( !GetPermissionsManager().HasPermission( "COT.Show", NULL ) )
            return;

        GetGame().GetUIManager().ShowScriptedMenu( new EditorMenu(), NULL );
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