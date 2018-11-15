class PlayerModule: EditorModule
{
    ref array< PlayerBase > m_GodModePlayers;

    void PlayerModule()
    {
        m_GodModePlayers = new ref array< PlayerBase >;

        GetRPCManager().AddRPC( "COT_Admin", "ReloadList", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "GodMode", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.List" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode.Enable" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode.Disable" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Set" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions.Retrieve" );
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/player/PlayerMenu.layout";
    }

    override void OnMissionLoaded()
    {
        GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateGodMode, 100, true );
    }

    void UpdateGodMode()
    {
        for ( int i = 0; i < m_GodModePlayers.Count(); i++ )
        {
            PlayerBase player = m_GodModePlayers.Get( i );
            player.SetAllowDamage( false );

            player.SetHealth( player.GetMaxHealth( "", "" ) );
            player.SetHealth( "","Blood", player.GetMaxHealth( "", "Blood" ) );
            player.SetHealth( "","Shock", player.GetMaxHealth( "", "Shock" ) );
            
            player.GetStaminaHandler().SyncStamina(1000, 1000);
            player.GetStatStamina().Set(player.GetStaminaHandler().GetStaminaCap());
            player.GetStatEnergy().Set(1000);
            player.GetStatWater().Set(1000);
            player.GetStatStomachVolume().Set(300);     
            player.GetStatStomachWater().Set(300);
            player.GetStatStomachEnergy().Set(300);
            player.GetStatHeatComfort().Set(0);
            
            EntityAI oWeapon = player.GetHumanInventory().GetEntityInHands();

            if( oWeapon )
            {
                Magazine oMag = ( Magazine ) oWeapon.GetAttachmentByConfigTypeName( "DefaultMagazine" );

                if( oMag && oMag.IsMagazine() )
                {
                    oMag.LocalSetAmmoMax();
                }
                
                Object oSupressor = ( Object ) oWeapon.GetAttachmentByConfigTypeName( "SuppressorBase" );

                if( oSupressor )
                {
                    oSupressor.SetHealth( oSupressor.GetMaxHealth( "", "" ) );
                }
            }
        }
    }

    void SetGodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        
        Param2< bool, ref array< ref PlayerData > > data;
        if ( !ctx.Read( data ) ) return;

        if ( data.param1 )
        {
            if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode.Enable", sender ) )
                return;
        } else
        {
            if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode.Disable", sender ) )
                return;
        }

        if( type == CallType.Server )
        {
            ref array< ref AuthPlayer > players = DeserializePlayers( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = PlayerBase.Cast( players[i].GetPlayerObject() );

                if ( player == NULL ) return;

                if ( m_GodModePlayers.Find( player ) > -1 )
                {
                    if ( !data.param1 )
                    {
                        m_GodModePlayers.RemoveItem( player );
                        GetGame().ChatMP( player, "You no longer have god mode.", "colorAction" );
                    }
                } else
                {
                    if ( data.param1 )
                    {
                        m_GodModePlayers.Insert( player );
                        GetGame().ChatMP( player, "You now have god mode.", "colorAction" );
                    }
                }
            }
        }
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