class CfgMods
{
	class COS
	{
		dir = "COS";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "";
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
				files[] = {"COS/Scripts/3_Game"};
			};

			class worldScriptModule
			{
				value = "";
				files[] = {"COS/Scripts/4_World"};
			};

			class missionScriptModule
			{
				value = "";
				files[] = {"COS/Scripts/5_Mission","COS/Modules/Camera","COS/Modules/Game","COS/Modules/Object","COS/Modules/Teleport","COS/Modules/Weather"};
			};
		}
	};
};
