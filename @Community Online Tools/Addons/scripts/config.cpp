class CfgPatches
{
    class COT_Scripts
    {
        units[]={};
        weapons[]={};
        requiredVersion=0.1;
        requiredAddons[]=
        {
            "RPC_Scripts", "Permissions_Framework_Scripts", "COT_GUI", "DZ_Data"
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
                files[] = {"COT/Scripts/5_Mission"};
            };
        }
    };
};