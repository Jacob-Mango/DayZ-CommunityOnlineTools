class CfgPatches
{
	class COT_BaseBuildingManager_Scripts
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"JM_CF_Scripts",
			"JM_COT_Scripts"
		};
	};
};

class CfgMods
{
	class COT_BaseBuildingManager
	{
		dir = "COT_BaseBuildingManager";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "COT Base Building Manager";
		credits = "";
		author = "Jim";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = { "Game", "World", "Mission" };

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = { "COT_BaseBuildingManager/Scripts/3_Game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "COT_BaseBuildingManager/Scripts/4_World" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "COT_BaseBuildingManager/Scripts/5_Mission" };
			};
		};
	};
};
