class CfgPatches
{
    class COT_Scripts
    {
        units[]={};
        weapons[]={};
        requiredVersion=0.1;
        requiredAddons[]=
        {
            "RPC_Scripts", "COT_GUI", "DZ_Data"
        };
    };
};

class CfgMods
{
    class CommunityOnlineTools
    {
        dir = "COT";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "Community Online Tools";
        credits = "Jacob_Mango, DannyDog, Arkensor";
        author = "Jacob_Mango, DannyDog, Arkensor";
        authorID = "0"; 
        version = "1.0"; 
        extra = 0;
        type = "mod";
        
        dependencies[] = {"Game", "World", "Mission"};
        
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"COT/Scripts/3_Game"};
            };

            class worldScriptModule
            {
                value = "";
                files[] = {"COT/Scripts/4_World"};
            };

            class missionScriptModule
            {
                value = "";
                files[] = {"COT/Scripts/5_Mission","COT/Modules/Camera","COT/Modules/Game","COT/Modules/Object","COT/Modules/Teleport","COT/Modules/Weather"};
            };
        }
    };
};

class CfgVehicles
{
    class Inventory_Base;
    class Car;

    class CarScript: Car
    {
        scope = 2;
        class Reflectors
        {
            class Left
            {
                color[] = { 0.9, 1.0, 0.9, 1.0 };
                brightness = 5.0;
                radius = 50;
                angle = 100;
                angleInnerRatio = 0.8;
                dayLight = 1;
                flareAngleBias = 80.0;
                position = "light_left";
                direction = "light_left_dir";
                hitpoint = "dmgZone_lights";
                selection = "dmgZone_lights";
            };
            class Right: Left
            {
                position = "light_right";
                direction = "light_right_dir";
                hitpoint = "dmgZone_lights";
                selection = "dmgZone_light";
            };
        };
    };
    class Flashlight: Inventory_Base
    {
        scope = 2;
        class Reflectors
        {
            class Beam
            {
                color[] = { 0.9, 1.0, 0.9, 1.0 };
                brightness = 5.0;
                radius = 50;
                angle = 190;
                angleInnerRatio = 0.8;
                dayLight = 1;
                flareAngleBias = 80.0;
                position = "beamStart";
                direction = "beamEnd";
                hitpoint = "bulb";
                selection = "bulb";
            };
        };
    };
};