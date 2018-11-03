#include "COS\\Modules\\Teleport\\TeleportModule.c"

#include "COS\\Modules\\Teleport\\gui\\PositionMenu.c"

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new TeleportModule );
}