class GameModule: EditorModule
{
    void GameModule()
    {
        GetRPCManager().AddRPC( "COT_Game", "SpawnVehicle", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Game", "SetOldAiming", this, SingeplayerExecutionType.Client );

        GetPermissionsManager().RegisterPermission( "Game.SpawnVehicle" );
        GetPermissionsManager().RegisterPermission( "Game.ChangeAimingMode" );
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
        Print( "Filling the vehicle by " + cap + " with " + fluid )
    }

    void SpawnVehicle( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Game.SpawnVehicle", sender ) )
            return;

        Param3< string, vector, ref array< string> > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            Car oCar = Car.Cast( GetGame().CreateObject( data.param1, data.param2 ) );

            ref array< string> attachments = new ref array< string>;
            attachments.Copy( data.param3 );
            
            for (int j = 0; j < attachments.Count(); j++) {
                oCar.GetInventory().CreateInInventory( attachments[j] );
            }

            FillCar( oCar, CarFluid.FUEL );
            FillCar( oCar, CarFluid.OIL );
            FillCar( oCar, CarFluid.BRAKE );
            FillCar( oCar, CarFluid.COOLANT );
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
        }

        if( type == CallType.Client )
        {
            m_OldAiming = data.param1;
            
            GetPlayer().OverrideShootFromCamera( !m_OldAiming );
        }
    }
}