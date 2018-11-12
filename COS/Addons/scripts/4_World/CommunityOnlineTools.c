class CommunityOnlineTools : Module
{
    void CommunityOnlineTools()
    {
        GetRPCManager().AddRPC( "COT", "GetPermission", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "COT.Show" );
    }

    void ~CommunityOnlineTools()
    {
    }

    override void onMissionLoaded()
    {
        RequestPermissions();
    }

    void RequestPermissions( int try = 0 )
    {
        if ( GetGame().IsClient() )
        {
            GetRPCManager().SendRPC( "COT", "GetPermission", new Param, true );
        }
    }

    override void RegisterKeyMouseBindings() 
    {
        KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType(), "ShowCOMEditor", "[Y]", "Opens the COM Editor." );
    
        toggleCOMEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );

        RegisterKeyMouseBinding( toggleCOMEditor );
    }

    override void onUpdate( float timeslice )
    {
    }

    void ShowCOMEditor()
    {
        if ( !GetPermissionsManager().HasPermission( "COT.Show", NULL ) )
            return;

        GetGame().GetUIManager().ShowScriptedMenu( new EditorMenu() , NULL );
    }

    void GetPermission( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print( "CommunityOnlineTools::GetPermission" );
        
        if ( !GetGame().IsMultiplayer() )
            return;

        if ( type == CallType.Server )
        {
            ref array< ref AuthPlayer > players = GetPermissionsManager().GetPlayers();

            ref array< ref AuthPlayer > player = new ref array< ref AuthPlayer >;

            for ( int i = 0; i < players.Count(); i++ )
            {
                if ( players[i].GetGUID() == sender.GetId() )
                {
                    player.Insert( players[i] );
                }
            }

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COT", "GetPermission", new Param1< ref array< ref PlayerData > >( SerializePlayers( player ) ), true, sender );
            } 
        }

        if ( type == CallType.Client )
        {
            ref Param1< ref array< ref PlayerData > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< ref AuthPlayer > auPlayers = DeserializePlayers( data.param1 );

            if ( auPlayers.Count() == 1 )
            {
                ClientAuthPlayer = auPlayers[0];
            } else 
            {
                RequestPermissions();
            }
        }
    }
}