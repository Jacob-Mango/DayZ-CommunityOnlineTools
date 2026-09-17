class CfgPatches
{
	class JM_COT_AutoTest
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
	class JM_COT_AutoTest
	{
		dir = "JM";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Community Online Tools - AutoTest";
		author = "COT Mod Team";
		authorID = "0";
		version = "0.0.0";
		extra = 0;
		type = "mod";

		dependencies[] =
		{
			"Game",
			"World",
			"Mission"
		};

		// Master compile gate. Commented out by default - uncomment to enable
		// the autotest boot pass. No diag-build/marker-file/user.cfg gating on
		// top of this: the define alone decides.
		defines[] =
		{
			"JM_COT_AUTOTEST"
		};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] =
				{
					"JM/COT/AutoTest/3_Game"
				};
			};

			class missionScriptModule
			{
				value = "";
				files[] =
				{
					"JM/COT/AutoTest/5_Mission"
				};
			};
		};
	};
};
