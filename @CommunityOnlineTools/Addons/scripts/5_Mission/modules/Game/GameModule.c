class GameModule: EditorModule
{
    void GameModule()
    {
        GetRPCManager().AddRPC( "COT_Game", "SpawnVehicle", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Game", "SetOldAiming", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Game", "ThrowApple", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Game.SpawnVehicle" );
        GetPermissionsManager().RegisterPermission( "Game.ChangeAimingMode" );
        GetPermissionsManager().RegisterPermission( "Game.ThrowApple" );
    }

    override string GetLayoutRoot()
    {
        return "COT/gui/layouts/Game/GameMenu.layout";
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
        if ( !GetPermissionsManager().HasPermission( "Game.SpawnVehicle", sender ) )
            return;

        ref Param3< string, vector, ref array< string> > data;
        if ( !ctx.Read( data ) ) return;

        ref array< string> attachments = new ref array< string>;
        attachments.Copy( data.param3 );
        
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