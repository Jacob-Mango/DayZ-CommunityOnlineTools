#include "COS\\Modules\\Position\\gui\\PositionMenu.c"

class PositionModule: EditorModule
{
    void PositionModule()
    {
        GetRPCManager().AddRPC( "COS_Teleport", "SetPosition", this, SingeplayerExecutionType.Server );
    }

    string GetLayoutRoot()
    {
        return "COS\\Modules\\Position\\gui\\layouts\\PositionMenu.layout";
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
}

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new PositionModule );
}