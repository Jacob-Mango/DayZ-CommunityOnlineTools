/**
 * config.cpp
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2020 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class CfgPatches
{
	class JM_COT_Objects_Debug
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data"
		};
	};
};

class CfgVehicles
{
    class Hammer;
	class Inventory_Base;
	class HouseNoDestruct;

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

	class CarScript;
	class Truck_01_Base: CarScript
	{
        scope = 2;
	};

	class Truck_01_Chassis;
		class AnimationSources;
	class Truck_01_Command: Truck_01_Chassis
	{
		displayName = "M3S Command";
		model = "\dz\vehicles\wheeled\Truck_01\Truck_01_Command.p3d";
		attachments[] = {"TruckBattery","Reflector_1_1","Reflector_2_1","Truck_01_Door_1_1","Truck_01_Door_2_1","Truck_01_Hood","Truck_01_Wheel_1_1","Truck_01_Wheel_1_2","Truck_01_Wheel_1_3","Truck_01_Wheel_2_1","Truck_01_Wheel_2_2","Truck_01_Wheel_2_3","Truck_01_Wheel_Spare_1","Truck_01_Wheel_Spare_2"};
		
		class Cargo
		{
			itemsCargoSize[] = {10,60};
			allowOwnedCargoManipulation = 1;
			openable = 0;
		};
		class AnimationSources: AnimationSources
		{
			class DoorsTrunk
			{
				source = "user";
				initPhase = 0;
				animPeriod = 0.5;
			};
		};
	};
};