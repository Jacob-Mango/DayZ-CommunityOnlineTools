class PlayerModule: EditorModule
{
    void PlayerModule()
    {
        GetRPCManager().AddRPC( "COT_Admin", "SetPermissions", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "KickPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "BanPlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "GodMode", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "SpectatePlayer", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "ToggleFreecam", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COT_Admin", "Player_SetHealth", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetBlood", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetEnergy", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetWater", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetShock", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetHeatComfort", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetWet", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetTremor", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetStamina", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetLifeSpanState", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_SetBloodyHands", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COT_Admin", "Player_KickTransport", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_RepairTransport", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_TeleportToMe", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Admin", "Player_TeleportMeTo", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Admin.Permissions" );
        
        GetPermissionsManager().RegisterPermission( "Admin.Player.Ban" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Godmode" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Spectate" );

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

        GetPermissionsManager().RegisterPermission( "Admin.Player.Transport.Kick" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Transport.Repair" );

        GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.ToMe" );
        GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.MeTo" );

        GetPermissionsManager().RegisterPermission( "Admin.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Admin.View" );
    }

    void ~PlayerModule()
    {
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/player/PlayerMenu.layout";
    }

    override void OnMissionLoaded()
    {
    }

    void Player_SetHealth( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< float, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.Health", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Health", data.param1 );

                COTLog( sender, "Set health to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Blood", data.param1 );

                COTLog( sender, "Set blood to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatEnergy().Set( data.param1 );

                COTLog( sender, "Set energy to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatWater().Set( data.param1 );

                COTLog( sender, "Set water to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.SetHealth( "GlobalHealth", "Shock", data.param1 );

                COTLog( sender, "Set shock to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatHeatComfort().Set( data.param1 );

                COTLog( sender, "Set heat comfort to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatWet().Set( data.param1 );

                COTLog( sender, "Set wetness to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatTremor().Set( data.param1 );

                COTLog( sender, "Set tremor to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.GetStatStamina().Set( data.param1 );

                COTLog( sender, "Set stamina to " + data.param1 + " for " + players[i].GetGUID() );
            }
        }
    }

    void Player_SetLifeSpanState( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< int, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Set.LastShaved", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                if ( data.param1 >= LifeSpanState.BEARD_NONE && data.param1 < LifeSpanState.COUNT )
                {
                    player.SetLifeSpanStateVisible( data.param1 );
                }

                COTLog( sender, "Set beard state to " + data.param1 + " for " + players[i].GetGUID() );
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
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                player.SetBloodyHands( data.param1 );

                COTLog( sender, "Set bloody hands to " + data.param1 + " for " + players[i].GetGUID() );
            }
        }
    }

    void Player_KickTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param1< ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Kick", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param1 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

                if ( vehCommand == NULL ) continue;

                Transport trans = vehCommand.GetTransport();
                
                if ( trans )
                {
                    Car car;
                    if ( Class.CastTo(car, trans) )
                    {
                        float speed = car.GetSpeedometer();
                        if ( speed <= 8 )
                        {
                            vehCommand.GetOutVehicle();
                        }
                        else
                        {
                            vehCommand.JumpOutVehicle();
                        }

                        COTLog( sender, "Kicked " + players[i].GetGUID() + " out of transport" );
                    }
                }
            }
        }
    }

    void Player_RepairTransport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param1< ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Transport.Repair", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< Transport > completedTransports = new array< Transport >;

            array< ref AuthPlayer > players = DeserializePlayersID( data.param1 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL || player.GetTransport() == NULL ) continue;

                Transport transport = player.GetTransport();

                if ( completedTransports.Find( transport ) > -1 )
                {
                    ItemBase radiator;

                    Class.CastTo( radiator, transport.FindAttachmentBySlotName("CarRadiator") );

                    if ( radiator )
                    {
                        radiator.SetHealth( "", "", 1 );
                    }

                    transport.SetHealth( "Engine", "", 1 );
                    transport.SetHealth( "FuelTank", "", 1 );

                    CarScript car = CarScript.Cast( transport );

                    if ( car )
                    {
                        car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ) );
                        car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ) );
                        car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ) );
                        car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ) );
                    }

                    completedTransports.Insert( transport );

                    COTLog( sender, "Repaired transport for " + players[i].GetGUID() );
                }
            }
        }
    }

    void Player_TeleportToMe( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param2< vector, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.ToMe", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                if ( player.GetTransport() != NULL ) continue;

                player.SetPosition( data.param1 );

                COTLog( sender, "Teleported " + players[i].GetGUID() + " to self" );
            }
        }
    }

    void Player_TeleportMeTo( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        Param1< ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.MeTo", sender ) )
            return;

        PlayerBase senderPlayer = PlayerBase.Cast( target );

        if ( senderPlayer == NULL || senderPlayer.GetTransport() != NULL ) return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param1 );

            if ( players.Count() != 1 ) return;

            PlayerBase player = players[0].PlayerObject;

            if ( player == NULL ) return;

            senderPlayer.SetPosition( player.GetPosition() );

            COTLog( sender, "Teleported self to " + players[0].GetGUID() );
        }
    }

    void SpectatePlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Spectate", sender ) )
            return;

        if( type == CallType.Server )
        {
            Param2< bool, ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            if ( !data.param1 )
            {
                GetGame().SelectPlayer( sender, target );
                GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param, true, sender );

                COTLog( sender, "Left spectating/free camera" );
                return;
            }

            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            if ( players.Count() != 1 ) return;

            PlayerBase player = players[0].PlayerObject;

            if ( player == NULL ) return;

            GetGame().SelectSpectator( sender, "SpectatorCamera", player.GetPosition() );

            GetGame().SelectPlayer( sender, NULL );

            GetRPCManager().SendRPC( "COT_Admin", "SpectatePlayer", new Param, true, sender, player );

            COTLog( sender, "Spectating " + players[0].GetGUID() );
        }    

        if( type == CallType.Client )
        {
            if ( GetGame().IsMultiplayer() )
            {
                CurrentActiveCamera = COTCamera.Cast( Camera.GetCurrentCamera() );
            }
            
            if ( CurrentActiveCamera )
            {
                CurrentActiveCamera.SelectedTarget( target );
                CurrentActiveCamera.SetActive( true );
                GetPlayer().GetInputController().SetDisabled( true );
            }
        }
    }

    void GodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        return;

        Param2< bool, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Godmode", sender ) )
            return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                PlayerBase player = players[i].PlayerObject;

                if ( player == NULL ) continue;

                // player.SetGodMode( data.param1 );
 
                COTLog( sender, "Set god mode to " + data.param1 + " for " + players[i].GetGUID() );
            }
        }
    }

    void SetPermissions( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Permissions", sender ) )
            return;
   
        if ( type == CallType.Server )
        {
            Param2< ref array< string >, ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            ref array< string > perms = new ref array< string >;
            perms.Copy( data.param1 );

            ref array< string > guids = new ref array< string >;
            guids.Copy( data.param2 );

            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            for ( int i = 0; i < guids.Count(); i++ )
            {
                for ( int k = 0; k < GetPermissionsManager().AuthPlayers.Count(); k++ )
                {
                    ref AuthPlayer player = GetPermissionsManager().AuthPlayers[k];
                    
                    if ( guids[i] == player.GetSteam64ID() )
                    {
                        player.ClearPermissions();

                        for ( int j = 0; j < perms.Count(); j++ )
                        {
                            player.AddPermission( perms[j] );
                        }

                        GetRPCManager().SendRPC( "PermissionsFramework", "UpdatePlayerData", new Param1< ref PlayerData >( SerializePlayer( player ) ), true, player.IdentityPlayer );

                        player.Save();

                        COTLog( sender, "Set and saved permissions for " + players[i].GetSteam64ID() );
                    }
                }
            }
        }
    }

    void KickPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Kick", sender ) )
            return;

        if ( type == CallType.Server )
        {
            ref Param1< ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            array< ref AuthPlayer > auPlayers = DeserializePlayersID( data.param1 );

            for ( int i = 0; i < auPlayers.Count(); i++ )
            {
                auPlayers[i].Kick();
                COTLog( sender, "Kicked " + auPlayers[i].GetGUID() );
            }
        }
    }

    void BanPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Admin.Player.Ban", sender ) )
            return;

        if ( type == CallType.Server )
        {
            ref Param1< ref array< string > > data;
            if ( !ctx.Read( data ) ) return;

            array< ref AuthPlayer > auPlayers = DeserializePlayersID( data.param1 );

            for ( int i = 0; i < auPlayers.Count(); i++ )
            {
                auPlayers[i].Ban();
                COTLog( sender, "Banned " + auPlayers[i].GetGUID() );
            }
        }
    }
}