class TeleportModule: EditorModule
{
    void TeleportModule()
    {
        GetRPCManager().AddRPC( "COS_Teleport", "SetPosition", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Teleport.SetPosition" );
    }

    override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding teleport	    = new KeyMouseBinding( GetModuleType() , "TeleportCursor", "[H]"    , "Teleport to cursor position." );

		teleport.AddKeyBind( KeyCode.KC_H,    KeyMouseBinding.KB_EVENT_PRESS   );
		
		RegisterKeyMouseBinding( teleport );
	}

    override string GetLayoutRoot()
    {
        return "COS/Modules/Teleport/gui/layouts/PositionMenu.layout";
    }

    void TeleportCursor()
    {
        Print( "TeleportModule::TeleportCursor()" );

        vector hitPos = GetCursorPos();

        float distance = vector.Distance( GetGame().GetPlayer().GetPosition(), hitPos );

        if ( distance < 5000 )
        {
            GetRPCManager().SendRPC( "COS_Teleport", "SetPosition", new Param1< vector >( hitPos ), true, NULL, GetGame().GetPlayer() );
        }
        else
        {
            GetPlayer().MessageStatus( "Distance for teleportation is too far!" );
        }
    }
    
    void SetPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( sender, "Teleport.SetPosition" ) )
            return;

        Param1< vector > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {    
            target.SetPosition( data.param1 );

            PlayerBase player = target;

            if ( player )
            {
                player.MessageImportant( "Teleported!" );
            }
        }
    }
}