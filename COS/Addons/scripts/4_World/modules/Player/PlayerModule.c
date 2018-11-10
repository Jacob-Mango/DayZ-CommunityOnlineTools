class PlayerModule: EditorModule
{
    void PlayerModule()
    {
        GetRPCManager().AddRPC( "COS_Player", "ReloadList", this, SingeplayerExecutionType.Server );
        
        GetRPCManager().AddRPC( "COS_Player", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS_Player", "LoadPermissions", this, SingeplayerExecutionType.Server );
    }

    override string GetLayoutRoot()
    {
        return "COS/gui/layouts/player/PlayerMenu.layout";
    }

    // TODO: Change to PlayerIdentity
    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Perms.Set" ) )
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

        if ( !GetPermissionsManager().HasPermission( sender, "Perms.Load" ) )
            return;

        bool cont = false;
   
        ref array< string > perms = new ref array< string >;

        Print("Test -1")

        if ( type == CallType.Server )
        {
            Print("Test 1");
            ref Param1<PlayerIdentity> sdata;
            if ( !ctx.Read( sdata ) ) return;
            Print("Test 2");

            ref AuthPlayer au = GetPermissionsManager().GetPlayer( sdata.param1 );

            Print( sdata.param1 );
            Print( au );
            if ( au )
            {
                Print("Test 4");

                perms = au.Serialize();

                Print( perms );

                if ( GetGame().IsMultiplayer() )
                {
                    GetRPCManager().SendRPC( "COS_Player", "LoadPermissions", new Param1< ref array< string > >( perms ), true, sender );
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
        if ( !GetPermissionsManager().HasPermission( sender, "Player.ReloadList" ) )
            return;
        
        bool cont = false;

        if ( type == CallType.Server )
        {
            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COS_Player", "ReloadList", new Param, true, sender );
            } else
            {
                cont = true;
            }
        }

        if ( type == CallType.Client || cont )
        {
            PlayerMenu menu = PlayerMenu.Cast( m_MenuPopup );
            
            if ( menu )
            {
                menu.m_ShouldUpdateList = true;
            }
        }
    }
}