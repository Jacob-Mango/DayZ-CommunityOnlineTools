class CfgPatches
{
	class COT_AnimalManager_Scripts
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
	class COT_AnimalManager
	{
		dir = "COT_AnimalManager";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "COT Animal Manager";
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
				files[] = { "COT_AnimalManager/Scripts/3_Game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "COT_AnimalManager/Scripts/4_World" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "COT_AnimalManager/Scripts/5_Mission" };
			};
		};
	};
};
