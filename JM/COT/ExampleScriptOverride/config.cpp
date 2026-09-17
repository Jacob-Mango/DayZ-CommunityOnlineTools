class CfgPatches
{
	class JM_COT_ExampleScriptOverride
	{
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"JM_COT_Scripts"
		};
	};
};

class CfgMods
{
	class JM_COT_ExampleScriptOverride
	{
		dir = "JM";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Community Online Tools - Example Script Override";
		author = "COT Mod Team";
		authorID = "0";
		version = "1.0.0";
		extra = 0;
		type = "mod";

		dependencies[] =
		{
			"Game",
			"World",
			"Mission"
		};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] =
				{
					"JM/COT/ExampleScriptOverride/3_Game"
				};
			};

			class worldScriptModule
			{
				value = "";
				files[] =
				{
					"JM/COT/ExampleScriptOverride/4_World"
				};
			};

			class missionScriptModule
			{
				value = "";
				files[] =
				{
					"JM/COT/ExampleScriptOverride/5_Mission"
				};
			};
		};
	};
};
