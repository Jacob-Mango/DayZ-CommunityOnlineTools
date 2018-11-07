class GameModule: EditorModule
{
    ref array< PlayerBase > m_GodModePlayers;

    void GameModule()
    {
        m_GodModePlayers = new ref array< PlayerBase >;

        GetRPCManager().AddRPC( "COS_Game", "SpawnVehicle", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COS_Game", "KillEntity", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COS_Game", "SetOldAiming", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS_Game", "SetGodMode", this, SingeplayerExecutionType.Client );

        GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.UpdateGodMode, 100, true );
    }

    override string GetLayoutRoot()
    {
        return "COS/Modules/Game/gui/layouts/GameMenu.layout";
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

    override void onUpdate( float timeslice )
    {
        if ( GetGame().IsClient() )
        {
            if ( m_OldAiming )
            {
                GetPlayer().OverrideShootFromCamera( false );
            }
        } else
        {
            /*
            ref array<Man> players = new ref array<Man>;

            GetGame().GetPlayers( players );

            for ( int i = 0; i < players.Count(); i++ )
            {
                if ( players[i].IsInherited( DayZPlayerImplement ) )
                {
                    DayZPlayerImplement.Cast( players[i] ).OverrideShootFromCamera( false );
                }
            }
            */
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
        if ( !GetPermissionsManager().HasPermission( sender, "Game.SpawnVehicle" ) )
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
        if ( !GetPermissionsManager().HasPermission( sender, "Game.ChangeAimingMode" ) )
            return;

        Param1< bool > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            m_OldAiming = data.param1;

            GetRPCManager().SendRPC( "COS_Game", "SetOldAiming", data, true );
        }

        if( type == CallType.Client )
        {
            m_OldAiming = data.param1;
            
            GetPlayer().OverrideShootFromCamera( !m_OldAiming );
        }
    }

    void SetGodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Game.EnableGodMode" ) )
            return;

        Param1< PlayerBase > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            if ( m_GodModePlayers.Find( data.param1 ) > -1 )
            {
                m_GodModePlayers.RemoveItem( data.param1 );
                GetGame().ChatMP( data.param1, "You no longer have god mode.", "colorAction" );
            } else
            {
                m_GodModePlayers.Insert( data.param1 );
                GetGame().ChatMP( data.param1, "You now have god mode.", "colorAction" );
            }
        }
    }

    void KillEntity( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Game.KillEntity" ) )
            return;

        Param1< EntityAI > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            PlayerBase player;

            if ( PlayerBase.CastTo( player, data.param1 ) ) 
            {
                m_GodModePlayers.RemoveItem( player );
                GetGame().ChatMP( player, "You no longer have god mode.", "colorAction" );
            }

            data.param1.SetHealth( "", "", 0 );
        }
    }
}