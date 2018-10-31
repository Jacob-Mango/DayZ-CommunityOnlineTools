#include "COS\\Modules\\Editor\\ObjectEditor.c"
#include "COS\\Modules\\Editor\\COSKeyBinds.c"

#include "COS\\Modules\\Editor\\gui\\EditorMenu.c"
#include "COS\\Modules\\Editor\\gui\\ObjectMenu.c"
#include "COS\\Modules\\Editor\\gui\\PositionMenu.c"
#include "COS\\Modules\\Editor\\gui\\WeatherMenu.c"
#include "COS\\Modules\\Editor\\gui\\GameMenu.c"
#include "COS\\Modules\\Editor\\gui\\PopupMenu.c"
#include "COS\\Modules\\Editor\\gui\\ObjectInfoMenu.c"
#include "COS\\Modules\\Editor\\gui\\OverrideVerticalSpacer.c"

void RegisterModules( ModuleManager moduleManager )
{
    moduleManager.RegisterModule( new ObjectEditor );

    moduleManager.RegisterModule( new COSKeyBinds );
}