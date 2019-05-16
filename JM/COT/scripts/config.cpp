class CfgPatches
{
	class JM_COT_Scripts
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"JM_CF_Scripts", "JM_COT_GUI", "DZ_Data"
		};
	};
};

class CfgMods
{
	class CommunityOnlineTools
	{
		dir = "JM";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Community Online Tools";
		credits = "Jacob_Mango, DannyDog, Arkensor";
		creditsJson = "JM/COT/Scripts/Data/Credits.json";
		author = "Jacob_Mango, DannyDog, Arkensor";
		authorID = "0";
		version = "JM/COT/scripts/Data/Version.hpp";
		extra = 0;
		type = "mod";
		
		dependencies[] = {"Game", "World", "Mission"};
		
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"JM/COT/Scripts/3_Game"};
			};

			class worldScriptModule
			{
				value = "";
				files[] = {"JM/COT/Scripts/4_World"};
			};

			class missionScriptModule
			{
				value = "";
				files[] = {"JM/COT/Scripts/5_Mission"};
			};
		}
	};
};