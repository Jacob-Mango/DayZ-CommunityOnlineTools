class GameModule: EditorModule
{
    protected ref GameSettings settings;

    void GameModule()
    {
        GetRPCManager().AddRPC( "COT_Game", "LoadData", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Game", "SpawnVehicle", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Game", "SetOldAiming", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Game", "ThrowApple", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Game.ChangeAimingMode" );
        GetPermissionsManager().RegisterPermission( "Game.ThrowApple" );
    }

    override void OnMissionLoaded()
    {
        super.OnMissionLoaded();

        if ( GetGame().IsClient() )
            GetRPCManager().SendRPC( "COT_Game", "LoadData", new Param, true );
    }

    override void ReloadSettings()
    {
        super.ReloadSettings();

        settings = GameSettings.Load();

        for ( int i = 0; i < settings.Vehicles.Count(); i++ )
        {
            GetPermissionsManager().RegisterPermission( "Game.Spawn.Vehicle." + settings.Vehicles.GetKey( i ) );
        }
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();

        if ( GetGame().IsServer() )
            settings.Save();
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/Game/GameMenu.layout";
    }

    autoptr map< string, ref array< string > > GetVehicles()
    {
        return settings.Vehicles;
    }

    override void OnUpdate( float timeslice )
    {
        if ( GetGame().IsClient() )
        {
            if ( m_OldAiming )
            {
                GetPlayer().OverrideShootFromCamera( false );
            }
        }
    }

    private void FillCar( Car car, CarFluid fluid )
    {
        float cap = car.GetFluidCapacity( fluid );
        car.Fill( fluid, cap );
    }
    
    void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if( type == CallType.Server )
        {
            GetRPCManager().SendRPC( "COT_Game", "LoadData", new Param1< string >( JsonFileLoader<GameSettings>.JsonMakeData( settings ) ), true );
        }

        if( type == CallType.Client )
        {
            Param1< string > data;
            if ( !ctx.Read( data ) ) return;

            JsonFileLoader<GameSettings>.JsonLoadData( data.param1, settings );
        }
    }

    void ThrowApple( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Game.ThrowApple", sender ) )
            return;
            
        if( type == CallType.Server )
        {
            vector direction = target.GetDirection();
            vector position = target.GetPosition();

            Human human = Human.Cast( target );

            if ( human )
            {
                int index = human.GetBoneIndexByName("Camera3rd_Helper");

                float pQuat[];
                human.GetBoneRotationWS( index, pQuat )
                vector rotation = Math3D.QuatToAngles( pQuat );

                position = human.GetBonePositionWS( index );
                direction = rotation.AnglesToVector();
            }

            position = position + ( direction * 2 ) + "0 1.5 0";
            
            Object physicsObj = GetGame().CreateObject( "Apple", position );

            if ( physicsObj == NULL ) return;

            dBodyDestroy( physicsObj );

            autoptr PhysicsGeomDef geoms[] = { PhysicsGeomDef("", dGeomCreateSphere( 0.1 ), "material/default", 0xffffffff )};
            dBodyCreateDynamicEx( physicsObj, "0 0 0", 1.0, geoms );
        }
    }

    void SpawnVehicle( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        ref Param2< string, vector > data;
        if ( !ctx.Read( data ) ) return;

        ref array< string > attachments = new ref array< string>;
        if ( !GetVehicles().Find( data.param1, attachments ) )
        {
            return;
        }

        if ( !GetPermissionsManager().HasPermission( "Game.Spawn.Vehicle." + data.param1, sender ) )
        
        if( type == CallType.Server )
        {
            Car oCar = Car.Cast( GetGame().CreateObject( data.param1, data.param2 ) );

            for (int j = 0; j < attachments.Count(); j++)
            {
                oCar.GetInventory().CreateInInventory( attachments[j] );
            }

            FillCar( oCar, CarFluid.FUEL );
            FillCar( oCar, CarFluid.OIL );
            FillCar( oCar, CarFluid.BRAKE );
            FillCar( oCar, CarFluid.COOLANT );

            COTLog( sender, "Spawned vehicle " + data.param1 + " at " + data.param2.ToString() );
        }
    }

    void SetOldAiming( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Game.ChangeAimingMode", sender ) )
            return;

        Param1< bool > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            m_OldAiming = data.param1;

            GetRPCManager().SendRPC( "COT_Game", "SetOldAiming", data, true );

            COTLog( sender, "Set the server aiming mode to " + data.param1 );
        }

        if( type == CallType.Client )
        {
            m_OldAiming = data.param1;
            
            GetPlayer().OverrideShootFromCamera( !m_OldAiming );
        }
    }
}