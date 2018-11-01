#include "COS\\Modules\\Position\\PositionModule.c"

#include "COS\\Modules\\Position\\gui\\PositionMenu.c"

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new PositionModule );
}