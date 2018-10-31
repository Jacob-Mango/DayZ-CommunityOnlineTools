class EditorModule : Module
{
    bool m_OldAiming;

    ref array< PlayerBase > m_GodModePlayers;

    void EditorModule()
    {
        m_OldAiming = false;
        m_GodModePlayers = new ref array< PlayerBase >;

        GetRPCManager().AddRPC( "COS", "SetPosition", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COS", "SpawnObjectPosition", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COS", "SpawnObjectInventory", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COS", "SpawnVehicle", this, SingeplayerExecutionType.Server );

        GetRPCManager().AddRPC( "COS", "SetOldAiming", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS", "SetGodMode", this, SingeplayerExecutionType.Client );

        GetRPCManager().AddRPC( "COS", "Weather_SetStorm", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS", "Weather_SetFog", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS", "Weather_SetOvercast", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS", "Weather_SetWindFunctionParams", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COS", "Weather_SetDate", this, SingeplayerExecutionType.Client );

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( this.UpdateGodMode, 100, true );
    }

    void ~EditorModule()
    {
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( this.UpdateGodMode );
    }

	override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType(), "ShowCOMEditor", "[Y]", "Opens the COM Editor." );
    
		toggleCOMEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );

		RegisterKeyMouseBinding( toggleCOMEditor );
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
            player.GetStatStomachSolid().Set(300);     
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
        }
	}

    void ShowCOMEditor()
    {
        GetGame().GetUIManager().ShowScriptedMenu( new EditorMenu() , NULL );
    }

    void SetPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param1< vector > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{	
            target.SetPosition( data.param1 );
		}
    }
    
    void SpawnObjectPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< string, vector, string > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{
            bool ai = false;

            if ( GetGame().IsKindOf( data.param1, "DZ_LightAI" ) ) 
            {
                ai = true;
            }

            EntityAI entity = GetGame().CreateObject( data.param1, data.param2, false, ai );

            entity.SetHealth( entity.GetMaxHealth() );

            if ( entity.IsInherited( ItemBase ) )
            {
                ItemBase oItem = ( ItemBase ) entity;
                SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );

                int quantity = 0;

                string text = data.param3;

                text.ToUpper();

                if (text == "MAX")
                {
                    quantity = oItem.GetQuantityMax();
                } else
                {
                    quantity = text.ToInt();
                }

                oItem.SetQuantity(quantity);
            }

            entity.PlaceOnSurface();
		}
    }

    void SpawnObjectInventory( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param2< string, string > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{
            EntityAI entity = EntityAI.Cast( target ).GetInventory().CreateInInventory( data.param1 );

            entity.SetHealth( entity.GetMaxHealth() );

            if ( entity.IsInherited( ItemBase ) )
            {
                ItemBase oItem = ( ItemBase ) entity;
                SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );

                int quantity = 0;

                string text = data.param2;

                text.ToUpper();

                if (text == "MAX")
                {
                    quantity = oItem.GetQuantityMax();
                } else
                {
                    quantity = text.ToInt();
                }

                oItem.SetQuantity(quantity);
            }
		}
    }

    void SpawnVehicle( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< string, vector, TStringArray > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{
            Car oCar = Car.Cast( GetGame().CreateObject( data.param1, data.param2 ) );

            for (int j = 0; j < data.param3.Count(); j++) { oCar.GetInventory().CreateAttachment( data.param3.Get(j) ); }

            oCar.Fill( CarFluid.FUEL, 1000 );
            oCar.Fill( CarFluid.OIL, 1000 );
            oCar.Fill( CarFluid.BRAKE, 1000 );
            oCar.Fill( CarFluid.COOLANT, 1000 );
        }
    }

    void SetOldAiming( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param1< bool > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{
            m_OldAiming = data.param1;

            GetRPCManager().SendRPC( "COS", "SetOldAiming", new Param1< bool >( m_GodMode ), true );
        }

		if( type == CallType.Client )
		{
            m_OldAiming = data.param1;
        }
    }

    void SetGodMode( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param1< PlayerBase > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{
            PlayerBase player;

            if ( !PlayerBase.CastTo( player, target ) ) return;

            if ( m_GodModePlayers.Find( data.param1 ) > -1 )
            {
                m_GodModePlayers.RemoveItem( data.param1 );
                player.MessageStatus("You no longer have god mode.");
            } else
            {
                m_GodModePlayers.Insert( data.param1 );
                player.MessageStatus("You now have god mode.");
            }
        }
    }

    void Weather_SetDate( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param5< int, int, int, int, int > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server && GetGame().IsMultiplayer() )
		{
            GetRPCManager().SendRPC( "COS", "Weather_SetDate", new Param5< int, int, int, int, int >( data.param1, data.param2, data.param3, data.param4, data.param5 ), true );
        }

        GetGame().GetWorld().SetDate( data.param1, data.param2, data.param3, data.param4, data.param5 );
    }

    void Weather_SetWindFunctionParams( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< float, float, float > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server && GetGame().IsMultiplayer() )
		{
            GetRPCManager().SendRPC( "COS", "Weather_SetWindFunctionParams", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().SetWindFunctionParams( data.param1, data.param2, data.param3 );
    }

    void Weather_SetOvercast( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< float, float, float > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server && GetGame().IsMultiplayer() )
		{
            GetRPCManager().SendRPC( "COS", "Weather_SetOvercast", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().GetOvercast().Set( data.param1, data.param2, data.param3 );
    }

    void Weather_SetFog( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< float, float, float > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server && GetGame().IsMultiplayer() )
		{
            GetRPCManager().SendRPC( "COS", "Weather_SetFog", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().GetFog().Set( data.param1, data.param2, data.param3 );
    }

    void Weather_SetStorm( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param3< float, float, float > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server && GetGame().IsMultiplayer() )
		{
            GetRPCManager().SendRPC( "COS", "Weather_SetStorm", new Param3< float, float, float >( data.param1, data.param2, data.param3 ), true );
        }

        GetGame().GetWeather().SetStorm( data.param1, data.param2, data.param3 );
    }
}