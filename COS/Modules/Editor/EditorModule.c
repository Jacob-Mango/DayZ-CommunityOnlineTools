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
    }

    void ~EditorModule()
    {

    }

	override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType() , "ShowCOMEditor" , "[Y]"    , "Opens the COM Editor."        );
    
		toggleCOMEditor.AddKeyBind( KeyCode.KC_Y,    KeyMouseBinding.KB_EVENT_RELEASE );

		RegisterKeyMouseBinding( toggleCOMEditor );
    }

    override void onUpdate( float timeslice )
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
            if ( m_GodModePlayers.Find( data.param1 ) > -1 )
            {
                m_GodModePlayers.RemoveItem( data.param1 );
            } else
            {
                m_GodModePlayers.Insert( data.param1 );
            }
        }
    }

}