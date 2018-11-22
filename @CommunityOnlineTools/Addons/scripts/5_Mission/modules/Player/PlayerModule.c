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

        GetRPCManager().AddRPC( "COT_Admin", "Player_SetHealth", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetBlood", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetEnergy", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetWater", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetShock", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetHeatComfort", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetWet", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetTremor", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetStamina", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetLastShaved", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetBloodyHands", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Permissions" );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Health" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Shock" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Blood" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Energy" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Water" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.HeatComfort" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Wet" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Tremor" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.Stamina" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.LastShaved" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Set.BloodyHands" );
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/player/PlayerMenu.layout";
    }

    override void OnMissionLoaded()
    {
        GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateGodMode, 100, true );

        GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.ReloadPlayerList, 1000, true );
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

    void ReloadPlayerList()
    {
        if ( GetGame().IsClient() ) return;

/*
// OLD CODE
        array<PlayerIdentity> identities = new array<PlayerIdentity>;
        GetGame().GetPlayerIndentities( identities );
        
        array<Man> ggplayers = new array<Man>;
        GetGame().GetPlayers( ggplayers );
        
        for ( int i = 0; i < identities.Count(); i++ )
        {
            int oindex = -1;
            int pindex = -1;
            
            for ( int k = 0; k < ggplayers.Count(); k++ )
            {
                if ( ggplayers[k].GetIdentity().GetId() == identities[i].GetId() )
                {
                    pindex = k;
                    break;
                }
            }

            for ( int j = 0; j < GetPermissionsManager().AuthPlayers.Count(); j++ )
            {
                if ( GetPermissionsManager().AuthPlayers[j].GetGUID() == identities[i].GetId() )
                {
                    pindex = j;
                    break;
                }
            }
            
            if ( pindex > -1 && oindex > -1 )
            {
                GetPermissionsManager().AuthPlayers[ pindex ].UpdatePlayerData( PlayerBase.Cast( ggplayers[ oindex ] ) );
            } else 
            {
                Print( "ReloadList: Player " + identities[i].GetId() + " ref does not exist!" );
            }
        }
*/

        for ( int j = 0; j < GetPermissionsManager().AuthPlayers.Count(); j++ )
        {
            GetPermissionsManager().AuthPlayers[j].UpdatePlayerData();
        }
    }

    void Player_SetHealth( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Health", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Health", data.param1 );
            }
        }
    }

    void Player_SetBlood( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Blood", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Blood", data.param1 );
            }
        }
    }

    void Player_SetEnergy( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Energy", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatEnergy().Set( data.param1 );
            }
        }
    }

    void Player_SetWater( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Water", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatWater().Set( data.param1 );
            }
        }
    }

    void Player_SetShock( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Shock", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Shock", data.param1 );
            }
        }
    }

    void Player_SetHeatComfort( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.HeatComfort", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatHeatComfort().Set( data.param1 );
            }
        }
    }

    void Player_SetWet( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Wet", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatWet().Set( data.param1 );
            }
        }
    }

    void Player_SetTremor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Tremor", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatTremor().Set( data.param1 );
            }
        }
    }

    void Player_SetStamina( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Stamina", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.GetStatStamina().Set( data.param1 );
            }
        }
    }

    void Click_SetLastShaved( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.LastShaved", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.SetLastShavedSeconds( data.param1 );
            }
        }
    }

    void Player_SetBloodyHands( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< bool, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.BloodyHands", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                player.SetBloodyHands( data.param1 );
            }
        }
    }

    void SetGodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< bool, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].GetPlayerObject();

                if ( player == NULL ) continue;

                if ( m_GodModePlayers.Find( player ) > -1 )
                {
                    if ( !data.param1 )
                    {
                        m_GodModePlayers.RemoveItem( player );
                        Message( player, "You no longer have god mode." );
                    }
                } else
                {
                    if ( data.param1 )
                    {
                        m_GodModePlayers.Insert( player );
                        Message( player, "You now have god mode." );
                    }
                }
            }
        }
    }

    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Permissions", sender ) )
            return;
   
        if ( type == CallType.Server )
        {
            Param2< ref array< string >, ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< string > perms = new ref array< string >;
            perms.Copy( data.param1 );

            ref array< string > guids = new ref array< string >;
            guids.Copy( data.param2 );

            array< ref AuthPlayer > players = DeserializePlayersGUID( data.param2 );

            for ( int i = 0; i < guids.Count(); i++ )
            {
                for ( int k = 0; k < GetPermissionsManager().AuthPlayers.Count(); k++ )
                {
                    ref AuthPlayer player = GetPermissionsManager().AuthPlayers[k];
                    
                    if ( guids[i] == player.GetGUID() )
                    {
                        player.ClearPermissions();

                        for ( int j = 0; j < perms.Count(); j++ )
                        {
                            player.AddPermission( perms[j] );
                        }

                        player.Save();
                    }
                }
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
            ref Param1< ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            array< ref AuthPlayer > auPlayers = DeserializePlayersGUID( data.param1 );

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
            ref Param1< ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            array< ref AuthPlayer > auPlayers = DeserializePlayersGUID( data.param1 );

            for ( int i = 0; i < auPlayers.Count(); i++ )
            {
                auPlayers[i].Ban();
            }
        }
    }

    void ReloadList( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Print( "PlayerModule::ReloadList" );

        //if ( !GetPermissionsManager().HasPermission( "Admin.Player.List", sender ) )
        //    return;
        
        bool cont = false;

        if ( type == CallType.Server )
        {
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