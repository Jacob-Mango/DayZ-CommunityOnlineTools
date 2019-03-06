class TeleportModule: EditorModule
{
    protected ref TeleportSettings settings;
    
    void TeleportModule()
    {
        GetRPCManager().AddRPC( "COT_Teleport", "Cursor", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Teleport", "Predefined", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Teleport", "LoadData", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Teleport.Cursor" );
        GetPermissionsManager().RegisterPermission( "Teleport.Predefined" );
    
        GetPermissionsManager().RegisterPermission( "Teleport.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "Teleport.View" );
    }
    
    override void ReloadSettings()
    {
        super.ReloadSettings();

        settings = TeleportSettings.Load();
    }

    override void OnMissionLoaded()
    {
        super.OnMissionLoaded();

        if ( GetGame().IsClient() )
            GetRPCManager().SendRPC( "COT_Teleport", "LoadData", new Param, true );
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();

        if ( GetGame().IsServer() )
            settings.Save();
    }

    override void RegisterKeyMouseBindings() 
    {
        KeyMouseBinding teleport = new KeyMouseBinding( GetModuleType(), "TeleportCursor", "Teleport to cursor position.", true );
        teleport.AddBinding( "kH" );
        teleport.SetActionType( KeyMouseActionType.PRESS | KeyMouseActionType.HOLD );
        RegisterKeyMouseBinding( teleport );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Teleport/PositionMenu.layout";
    }

    ref array< ref TeleportLocation > GetLocations()
    {
        return settings.Locations;
    }

    void TeleportCursor()
    {
        if ( !GetPermissionsManager().HasPermission( "Teleport.Cursor" ) )
            return;
        }

        if ( !COTIsActive ) {
            Message( GetPlayer(), "Community Online Tools is currently toggled off." );
            return;
        }

        vector currentPosition = "0 0 0";
        vector hitPos = GetCursorPos();

        if ( CurrentActiveCamera && CurrentActiveCamera.IsActive() )
        {
            currentPosition = CurrentActiveCamera.GetPosition();
        } else 
        {
            currentPosition = GetPlayer().GetPosition();
        }

        float distance = vector.Distance( currentPosition, hitPos );

        if ( distance < 1000 )
        {
            GetRPCManager().SendRPC( "COT_Teleport", "Cursor", new Param1< vector >( hitPos ), true, NULL, GetGame().GetPlayer() );
        }
        else
        {
            Message( GetPlayer(), "Distance for teleportation is too far!" );
        }
    }

    void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if( type == CallType.Server )
        {
            if ( !GetPermissionsManager().HasPermission( "Teleport.Predefined", sender ) )
                return;

            GetRPCManager().SendRPC( "COT_Teleport", "LoadData", new Param1< ref TeleportSettings >( settings ), true );
        }

        if( type == CallType.Client )
        {
            Param1< ref TeleportSettings > data;
            if ( !ctx.Read( data ) ) return;

            settings = data.param1;
        }
    }

    void Cursor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Teleport.Cursor", sender ) )
            return;

        Param1< vector > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            PlayerBase player = PlayerBase.Cast( target );

            if ( !player ) return;

            if ( player.IsInTransport() )
            {
                // player.GetTransport().SetOrigin( data.param1 );
               HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

                if ( vehCommand )
                {
                    Transport transport = vehCommand.GetTransport();

                    if ( transport == NULL ) return;

                    transport.SetPosition( data.param1 );
                }
            } else
            {
                player.SetPosition( data.param1 );
            }

            // COTLog( sender, "Teleported to cursor" );
        }
    }
    
    void Predefined( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Teleport.Predefined", sender ) )
            return;

        Param2< string, ref array< string > > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

            vector position = "0 0 0";

            string name = data.param1;

            ref TeleportLocation location = NULL;

            for ( int i = 0; i < GetLocations().Count(); i++ )
            {
                if ( GetLocations()[i].Name == name )
                {
                    location = GetLocations()[i];
                    break;
                }
            }

            if ( location == NULL )
            {
                return;
            }

            position = SnapToGround( location.Position );

            for ( int j = 0; j < players.Count(); j++ )
            {
                PlayerBase player = players[j].PlayerObject;

                if ( player.IsInTransport() )
                {
                    HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

                    if ( vehCommand )
                    {
                        Transport transport = vehCommand.GetTransport();

                        if ( transport == NULL ) return;

                        transport.SetOrigin( position );
                        transport.SetPosition( position );
                    }
                } else 
                {
                    player.SetPosition( position );
                }
                
                COTLog( sender, "Teleported " + players[j].GetSteam64ID() + " to " + location.Name );
            }
        }
    }
}