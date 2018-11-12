class PlayerModule: EditorModule
{
    void PlayerModule()
    {
        GetRPCManager().AddRPC( "COS_Admin", "ReloadList", this, SingeplayerExecutionType.Server );
        
        GetRPCManager().AddRPC( "COS_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        //GetRPCManager().AddRPC( "COS_Admin", "LoadPermissions", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.List.Reload" );
        GetPermissionsManager().RegisterPermission( "Admin.Permissions.Player.Set" );
        GetPermissionsManager().RegisterPermission( "Admin.Permissions.Player.Load" );
    }

    override string GetLayoutRoot()
    {
        return "COS/gui/layouts/player/PlayerMenu.layout";
    }

    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Permissions.Player.Set" ) )
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

/*
    void LoadPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print("PlayerModule::LoadPermissions");

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Permissions.Player.Load" ) )
            return;

        bool cont = false;
   
        ref array< string > perms = new ref array< string >;

        if ( type == CallType.Server )
        {
            ref Param1< string > sdata;
            if ( !ctx.Read( sdata ) ) return;

            ref AuthPlayer au = GetPermissionsManager().GetPlayer( sdata.param1 );

            Print( sdata.param1 );
            Print( au );
            if ( au )
            {
                perms = au.Serialize();

                Print( perms );

                if ( GetGame().IsMultiplayer() )
                {
                    GetRPCManager().SendRPC( "COS_Admin", "LoadPermissions", new Param1< ref array< string > >( perms ), true, sender );
                } else
                {
                    cont = true;
                }
            }
        }

        if ( type == CallType.Client || cont )
        {
            if ( GetGame().IsMultiplayer() )
            {
                Param1< ref array< string > > cdata;
                if ( !ctx.Read( cdata ) ) return;

                perms.Copy( cdata.param1 );
            }
            
            if ( GetSelectedPlayers().Count() != 1 ) return;

            ref Permission permission = new ref Permission( GetSelectedPlayers()[0].GetGUID() );

            for ( int i = 0; i < perms.Count(); i++ )
            {
                permission.AddPermission( perms[i] );
            }

            ref PlayerMenu menu = PlayerMenu.Cast( m_MenuPopup );
            
            if ( menu )
            {
                menu.LoadPermissions( permission );
            }
        }
    }
*/

    void ReloadList( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print( "PlayerModule::ReloadList" );

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.List.Reload" ) )
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