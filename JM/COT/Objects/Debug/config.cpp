class CfgPatches
{
	class JM_COT_Objects_Debug
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Characters_Zombies"
		};
	};
};

class CfgVehicles
{

	class ZombieMaleBase;
	class COT_Zombie: ZombieMaleBase
	{
		scope=2;
		model="\dz\gear\cooking\barrel_holes.p3d";
	};

	class Inventory_Base;
    class COT_Objects_Debug_Base: Inventory_Base
    {
        scope = 1;
        hiddenSelections[] = { "texture" };
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(0,1,0,1,co)" };
    };
	
    class COT_Objects_Debug_Cube_Base: COT_Objects_Debug_Base
    {
        model = "JM\COT\Objects\Debug\COT_Objects_Debug_Cube_Base.p3d";
    };
    class COT_Objects_Debug_Cube_Green: COT_Objects_Debug_Cube_Base
    {
        scope = 2;
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(0,1,0,0.5,co)" };
    };
    class COT_Objects_Debug_Cube_Red: COT_Objects_Debug_Cube_Base
    {
        scope = 2;
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(1,0,0,0.5,co)" };
    };
    class COT_Objects_Debug_Cube_Orange: COT_Objects_Debug_Cube_Base
    {
        scope = 2;
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(1,0.5,0,0.5,co)" };
    };
    class COT_Objects_Debug_Cube_Blue: COT_Objects_Debug_Cube_Base
    {
        scope = 2;
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(0,0,1,0.5,co)" };
    };
    class COT_Objects_Debug_Cube_Purple: COT_Objects_Debug_Cube_Base
    {
        scope = 2;
        hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(0.5,0,1,0.5,co)" };
    };
};