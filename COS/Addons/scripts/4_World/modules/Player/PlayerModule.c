class PlayerModule: EditorModule
{
    ref array< ref PlayerData > Players;

    void PlayerModule()
    {
        Players = new ref array< ref PlayerData >;

        GetRPCManager().AddRPC( "COS_Admin", "ReloadList", this, SingeplayerExecutionType.Server );
        
        GetRPCManager().AddRPC( "COS_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS_Admin", "LoadPermissions", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.List.Reload" );
        GetPermissionsManager().RegisterPermission( "Admin.Permissions.Player.Set" );
        GetPermissionsManager().RegisterPermission( "Admin.Permissions.Player.Load" );
        GetPermissionsManager().RegisterPermission( "Admin.Permissions.Root.Load" );
    }

    override string GetLayoutRoot()
    {
        return "COS/gui/layouts/player/PlayerMenu.layout";
    }

    // TODO: Change to PlayerIdentity
    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Admin.Permissions.Player.Set" ) )
            return;
   
        if ( type == CallType.Server )
        {
            PlayerIdentity player = Man.Cast( target ).GetIdentity();

            Param1< ref array< string > > data;
            if ( !ctx.Read( data ) || !player ) return;

            ref array< string > perms;
            perms.Copy( data.param1 );

            ref AuthPlayer au = GetPermissionsManager().GetPlayer( player );

            if ( au )
            {
                au.ClearPermissions();

                for ( int i = 0; i < perms.Count(); i++ )
                {
                    au.AddPermission( perms[i] );
                }
            }
        }
    }

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

            Print( perms );

            ref PlayerMenu menu = PlayerMenu.Cast( m_MenuPopup );
            
            if ( menu )
            {
                menu.LoadPermissions( perms );
            }
        }
    }

    void ReloadList( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print( "PlayerModule::ReloadList" );

        if ( !GetPermissionsManager().HasPermission( sender, "Admin.List.Reload" ) )
            return;
            
        Print( "PlayerModule::ReloadList" );
        
        bool cont = false;

        if ( type == CallType.Server )
        {
            ref array<PlayerIdentity> identities = new ref array<PlayerIdentity>;
            GetGame().GetPlayerIndentities( identities );
            
            ref array<Man> ggplayers = new ref array<Man>;
            GetGame().GetPlayers( ggplayers );

            Players.Clear();
            
            for ( int i = 0; i < identities.Count(); i++ )
            {
                PlayerBase player = NULL;

                for ( int k = 0; k < ggplayers.Count(); k++ )
                {
                    if ( ggplayers[k].GetIdentity().GetId() == identities[i].GetId() )
                    {
                        player = ggplayers[k];
                    }
                }

                Players.Insert( new ref PlayerData( player, identities[i] ) );
            }

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COS_Admin", "ReloadList", new Param1< ref array< ref PlayerData > >( Players ), true, sender );
            } else
            {
                cont = true;
            }
        }

        if ( type == CallType.Client || cont )
        {
            if ( GetGame().IsMultiplayer() )
            {
                Param1< ref array< ref PlayerData > > data;
                if ( !ctx.Read( data ) ) return;

                Players = data.param1;
            }

            PlayerMenu menu = PlayerMenu.Cast( m_MenuPopup );

            if ( menu )
            {
                menu.m_ShouldUpdateList = true;
            }
        }
    }
}