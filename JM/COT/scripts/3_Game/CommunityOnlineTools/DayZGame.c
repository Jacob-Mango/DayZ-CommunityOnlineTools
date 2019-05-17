CGame CreateGame_COT()
{
	g_Game = new DayZGame;

    // RegInp( "UACOTModuleFocusGame", "UACOTModuleFocusGame", "core" );
    // RegInp( "UACOTModuleFocusUI", "UACOTModuleFocusUI", "core" );
    // RegInp( "UACOTModuleToggleCOT", "UACOTModuleToggleCOT", "core" );
    // RegInp( "UACOTModuleToggleMenu", "UACOTModuleToggleMenu", "core" );
    // RegInp( "UACOTModuleCloseCOT", "UACOTModuleCloseCOT", "core" );
    // RegInp( "UATeleportModuleTeleportCursor", "UATeleportModuleTeleportCursor", "core" );
    // RegInp( "UACameraToolToggleCamera", "UACameraToolToggleCamera", "core" );

	return g_Game;
}

UAInput RegInp( string sInputName, string sLoc, string sGroupName )
{
    GetUApi().DeRegisterInput( sInputName );
    return GetUApi().RegisterInput( sInputName, sLoc, sGroupName );
}