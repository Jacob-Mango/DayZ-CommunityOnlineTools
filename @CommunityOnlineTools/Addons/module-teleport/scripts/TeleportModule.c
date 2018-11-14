class TeleportModule: EditorModule
{
    void TeleportModule()
    {
        GetRPCManager().AddRPC( "COT_Teleport", "Cursor", this, SingeplayerExecutionType.Server );
        GetRPCManager().AddRPC( "COT_Teleport", "Predefined", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "Teleport.Cursor" );
        GetPermissionsManager().RegisterPermission( "Teleport.Predefined" );
    }

    override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding teleport = new KeyMouseBinding( GetModuleType() , "TeleportCursor", "[H]"    , "Teleport to cursor position." );

		teleport.AddKeyBind( KeyCode.KC_H, KeyMouseBinding.KB_EVENT_PRESS );
		
		RegisterKeyMouseBinding( teleport );
	}

    override string GetLayoutRoot()
    {
        return "COT/Modules/Teleport/layouts/PositionMenu.layout";
    }

    void TeleportCursor()
    {
        Print( "TeleportModule::TeleportCursor()" );

        vector hitPos = GetCursorPos();

        float distance = vector.Distance( GetGame().GetPlayer().GetPosition(), hitPos );

        if ( distance < 5000 )
        {
            GetRPCManager().SendRPC( "COT_Teleport", "Cursor", new Param1< vector >( hitPos ), true, NULL, GetGame().GetPlayer() );
        }
        else
        {
            GetPlayer().MessageStatus( "Distance for teleportation is too far!" );
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
            target.SetPosition( data.param1 );
        }
    }
    
    void Predefined( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "Teleport.Predefined", sender ) )
            return;

        Param2< vector, ref array< ref PlayerData > > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            ref array< ref AuthPlayer > players = DeserializePlayers( data.param2 );

            for ( int i = 0; i < players.Count(); i++ )
            {
                Man player = players[i].GetPlayerObject();

                player.SetPosition( data.param1 );
            }
        }
    }
}