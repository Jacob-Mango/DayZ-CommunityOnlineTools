/*
	COT_VehicleManager
	Addon pour Community-Online-Tools :
	ajoute un bouton "Nettoyer vehicules sur spawn" dans le menu Vehicules de COT.

	Le serveur lit $mission:cfgeventspawns.xml et supprime tout vehicule/bateau
	situe a moins de CVM_RADIUS metres (2D) d'un point de spawn vehicule vanilla.
	S'adapte automatiquement a la map de la mission (Chernarus / Livonia / Sakhal...).
*/

class CfgPatches
{
	class COT_VehicleManager
	{
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"JM_COT_Scripts"
		};
	};
};

class CfgMods
{
	class COT_VehicleManager
	{
		author = "Jim";
		type = "mod";
		dependencies[] =
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class missionScriptModule
			{
				value = "";
				files[] =
				{
					"COT_VehicleManager/scripts/5_Mission"
				};
			};
		};
	};
};
