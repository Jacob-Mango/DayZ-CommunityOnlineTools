/*
	Manages all keybinds for COM
*/
class COSKeyBinds extends Module 
{
	bool m_IsHudVisible = true;
	
	void COSKeyBinds()
	{
	}

	void ~COSKeyBinds()
	{
	}
	
	override void Init() 
	{
		super.Init();
	}
	
	override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding toggleCursor    = new KeyMouseBinding( GetModuleType() , "ToggleCursor"  , "[U]"    , "Toggles the cursor."   , true );
		KeyMouseBinding teleport	    = new KeyMouseBinding( GetModuleType() , "TeleportCursor", "[T]"    , "Teleport to cursor position." );
		KeyMouseBinding reload          = new KeyMouseBinding( GetModuleType() , "Reload"        , "[R]"    , "Instantly reloads mag."		 );
        KeyMouseBinding spawnZ          = new KeyMouseBinding( GetModuleType() , "SpawnZ"        , "[O]"    , "Spawns infected." );
        KeyMouseBinding hideHud          = new KeyMouseBinding( GetModuleType() , "HideHud"        , "[HOME]"    , "Hides ui completely." );

		toggleCursor   .AddKeyBind( KeyCode.KC_U,    KeyMouseBinding.KB_EVENT_PRESS   );
		teleport       .AddKeyBind( KeyCode.KC_T,    KeyMouseBinding.KB_EVENT_PRESS   );
		reload         .AddKeyBind( KeyCode.KC_R,    KeyMouseBinding.KB_EVENT_RELEASE );
        spawnZ         .AddKeyBind( KeyCode.KC_O,    KeyMouseBinding.KB_EVENT_RELEASE );
        hideHud        .AddKeyBind( KeyCode.KC_HOME, KeyMouseBinding.KB_EVENT_RELEASE );
		
		RegisterKeyMouseBinding( toggleCursor );
		RegisterKeyMouseBinding( teleport );
		RegisterKeyMouseBinding( reload );
        RegisterKeyMouseBinding( spawnZ );
        RegisterKeyMouseBinding( hideHud );
	}

    void ToggleCursor()
    {
        if ( GetGame().GetInput().HasGameFocus( INPUT_DEVICE_MOUSE ) )
        {
            GetGame().GetInput().ChangeGameFocus( 1, INPUT_DEVICE_MOUSE );
            GetGame().GetUIManager().ShowUICursor( true );
        }
        else
        {
            GetGame().GetUIManager().ShowUICursor( false );
            GetGame().GetInput().ResetGameFocus( INPUT_DEVICE_MOUSE );
        }
    }

    void TeleportCursor()
    {
        Print( "COMKeyBinds::TeleportCursor()" );

//		if ( CameraTool.Cast( m_Mission.GetModule( CameraTool ) ).IsUsingCamera() ) //Todo renable after module manager is done
//		{
//			GetPlayer().MessageStatus( "You can not teleport while you are inside the freecam!" );
//			return;
//		}
		
        vector hitPos = GetCursorPos();

        float distance = vector.Distance( GetPlayer().GetPosition(), hitPos );

        if ( distance < 5000 )
        {
            EntityAI oVehicle = GetPlayer().GetDrivingVehicle();

            if( oVehicle )
            {
                GetPlayer().MessageStatus( "Get out of the vehicle first!" );
            }
            else
            {
                GetRPCManager().SendRPC( "COS", "SetPosition", new Param1< vector >( hitPos ), true, NULL, GetGame().GetPlayer() );
            }
        }
        else
        {
            GetPlayer().MessageStatus( "Distance for teleportation is too far!" );
        }
    }

    void Reload()
    {
        EntityAI oWeapon = GetPlayer().GetHumanInventory().GetEntityInHands();

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

    void SpawnZ() 
    {
        GetGame().CreateObject( WorkingZombieClasses().GetRandomElement(), GetCursorPos(), false, true );
    }

    void HideHud() 
    {
        //Widget hudWidget = IngameHud.Cast(GetClientMission().GetHud()).GetHudPanelWidget();
        //hudWidget.Show(!hudWidget.IsVisible());
    }
}