class PlayerModule: EditorModule
{
    void PlayerModule()
    {
        GetRPCManager().AddRPC( "COT_Admin", "ReloadList", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.List" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Set" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Load" );
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/player/PlayerMenu.layout";
    }

    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Permissions.Set", sender ) )
            return;
   
        if ( type == CallType.Server )
        {
            Param2< ref array< string >, ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< string > perms = new ref array< string >;
            perms.Copy( data.param1 );

            ref array< string > guids = new ref array< string >;
            guids.Copy( data.param2 );

            for ( int i = 0; i < guids.Count(); i++ )
            {
                ref AuthPlayer player = GetPermissionsManager().GetPlayerByGUID( guids[i] );

                for ( int j = 0; j < perms.Count(); j++ )
                {
                    player.AddPermission( perms[j] );
                }
            }

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COT_Admin", "ReloadList", new Param1< ref array< ref PlayerData > >( SerializePlayers( GetPermissionsManager().GetPlayers() ) ), true, sender );
            }
        }
    }

    void KickPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print("PlayerModule::LoadPermissions");

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Kick", sender ) )
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

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", sender ) )
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

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.List", sender ) )
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

                ref AuthPlayer auth = GetPermissionsManager().GetPlayerByIdentity( identities[i] );
                auth.UpdatePlayerObject( player );

                auth.GetData().IPingMin = identities[i].GetPingMin();
                auth.GetData().IPingMax = identities[i].GetPingMax();
                auth.GetData().IPingAvg = identities[i].GetPingAvg();
            }

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COT_Admin", "ReloadList", new Param1< ref array< ref PlayerData > >( SerializePlayers( GetPermissionsManager().GetPlayers() ) ), true, sender );
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

                DeserializePlayers( data.param1 );
            }

            PlayerMenu menu = PlayerMenu.Cast( form );

            if ( menu )
            {
                menu.m_ShouldUpdateList = true;
            }
        }
    }
}