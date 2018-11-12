class PlayerModule: EditorModule
{
    void PlayerModule()
    {
        GetRPCManager().AddRPC( "COS_Admin", "ReloadList", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.List" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Set" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Load" );
    }

    override string GetLayoutRoot()
    {
        return "COS/gui/layouts/player/PlayerMenu.layout";
    }

    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Player.Permissions.Set" ) )
            return;
   
        if ( type == CallType.Server )
        {
            Param2< ref array< string >, ref array< ref PlayerData > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< ref AuthPlayer > players = DeserializePlayers( data.param2 );

            ref array< string > perms;
            perms.Copy( data.param1 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                ref AuthPlayer player = players[i];

                if ( player )
                {
                    player.ClearPermissions();

                    for ( int j = 0; j < perms.Count(); j++ )
                    {
                        player.AddPermission( perms[j] );
                    }
                }
            }
        }
    }

    void KickPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print("PlayerModule::LoadPermissions");

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Player.Kick" ) )
            return;

        if ( type == CallType.Server )
        {
            ref Param1< ref array< ref PlayerData > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< ref AuthPlayer > auPlayers = DeserializePlayers( data.param1 );

            for ( int i = 0; i < auPlayers.Count(); i++ )
            {
                auPlayers[i].Kick();
            }
        }
    }

    void BanPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print("PlayerModule::LoadPermissions");

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Player.Ban" ) )
            return;

        if ( type == CallType.Server )
        {
            ref Param1< ref array< ref PlayerData > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< ref AuthPlayer > auPlayers = DeserializePlayers( data.param1 );

            for ( int i = 0; i < auPlayers.Count(); i++ )
            {
                auPlayers[i].Ban();
            }
        }
    }

    void ReloadList( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print( "PlayerModule::ReloadList" );

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Player.List" ) )
            return;
        
        bool cont = false;

        if ( type == CallType.Server )
        {
            ref array<PlayerIdentity> identities = new ref array<PlayerIdentity>;
            GetGame().GetPlayerIndentities( identities );
            
            ref array<Man> ggplayers = new ref array<Man>;
            GetGame().GetPlayers( ggplayers );
            
            for ( int i = 0; i < identities.Count(); i++ )
            {
                Man player = NULL;

                for ( int k = 0; k < ggplayers.Count(); k++ )
                {
                    if ( ggplayers[k].GetIdentity().GetId() == identities[i].GetId() )
                    {
                        player = ggplayers[k];
                    }
                }

                ref AuthPlayer auth = GetPermissionsManager().GetPlayer( identities[i] );
                auth.UpdatePlayerObject( player );

                auth.GetData().IPingMin = identities[i].GetPingMin();
                auth.GetData().IPingMax = identities[i].GetPingMax();
                auth.GetData().IPingAvg = identities[i].GetPingAvg();
            }

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COS_Admin", "ReloadList", new Param1< ref array< ref PlayerData > >( SerializePlayers( GetPermissionsManager().GetPlayers() ) ), true, sender );
            } else
            {
                cont = true;
            }
        }

        if ( type == CallType.Client || cont )
        {
            if ( GetGame().IsMultiplayer() )
            {
                ref Param1< ref array< ref PlayerData > > data;
                if ( !ctx.Read( data ) ) return;

                //ref array< ref PlayerData > temp = new ref array< ref PlayerData >;
                //temp.Copy( data.param1 );

                ref array< ref AuthPlayer > auPlayers = DeserializePlayers( data.param1 );

                GetPermissionsManager().SetPlayers( auPlayers );
            }

            PlayerMenu menu = PlayerMenu.Cast( m_MenuPopup );

            if ( menu )
            {
                menu.m_ShouldUpdateList = true;
            }
        }
    }
}