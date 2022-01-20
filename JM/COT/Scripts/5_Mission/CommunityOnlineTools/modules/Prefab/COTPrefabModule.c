[CF_RegisterModule(COTPrefabModule)]
class COTPrefabModule : COTModule
{
	autoptr array<ref COTPrefabFile> PrefabFiles = new array<ref COTPrefabFile>();


	override string GetLayoutFile()
	{
		return "";
	}

	override string GetTitle()
	{
		return "Prefab Module";
	}


	//! DEFAULTS

	void InitDefaults()
	{
	}

	void InitDefaultVehicles()
	{
		auto pCivilianSedan = COTPrefabFile.Create("CivilianSedan", "Civilian Sedan");

		pCivilianSedan.BeginAttachment("");
	}

};
