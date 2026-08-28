class CfgPatches
{
	class Expansion_ConfigReload
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DayZExpansion_Core_Scripts"
		};
	};
};
class CfgMods
{
	class Expansion_ConfigReload
	{
		dir="Expansion_ConfigReload";
		name="Expansion Config Reload";
		author="Lyra";
		type="mod";
		inputs="Expansion_ConfigReload/scripts/Data/Inputs.xml";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"Expansion_ConfigReload/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"Expansion_ConfigReload/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"Expansion_ConfigReload/scripts/5_Mission"
				};
			};
		};
	};
};
