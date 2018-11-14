class CfgMods
{
	class CommunityOnlineTools
	{
		dir = "@CommunityOnlineTools";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Community Online Tools";
		credits = "Jacob_Mango";
		author = "Jacob";
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
