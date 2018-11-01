#include "COS\\Modules\\Object\\ObjectModule.c"

#include "COS\\Modules\\Object\\gui\\ObjectMenu.c"

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new ObjectModule );
}