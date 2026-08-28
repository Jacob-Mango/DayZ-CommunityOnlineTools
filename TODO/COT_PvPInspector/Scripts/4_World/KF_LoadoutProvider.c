// Pont OPTIONNEL vers COT_LoadoutManager, sans dependance forcee entre les mods.
// Si COT_LoadoutManager est charge, on appelle son COTLoadoutStore.RestoreOnto(Nearest)
// par reflexion (aucune reference de type a la compilation) ; sinon on ne fait rien et
// le dummy reste nu. Ainsi PvPInspector compile et tourne seul, et LM aussi.
//
// IMPORTANT : on cree une instance NEUVE de COTLoadoutStore a chaque appel. Ne PAS
// mettre cette instance en cache : la reutiliser entre appels provoque un access
// violation dans CallFunctionParams (instance reflexive a usage unique).
class KF_LoadoutProvider
{
	private static ref KF_LoadoutProvider s_Instance;

	static KF_LoadoutProvider Get()
	{
		if (!s_Instance)
			s_Instance = new KF_LoadoutProvider();
		return s_Instance;
	}

	// COT_LoadoutManager est-il charge ? (presence de son CfgPatches)
	bool IsAvailable()
	{
		return g_Game.ConfigIsExisting("CfgPatches COT_LoadoutManager");
	}

	// Applique sur 'target' (un dummy de reproduction) le loadout sauvegarde a la
	// mort du joueur identifie par steamID (GetPlainId). index = -1 -> derniere mort.
	bool ApplyDeathLoadout(EntityAI target, string steamID)
	{
		if (!target || steamID == "")
			return false;

		if (!IsAvailable())
			return false;

		string cls = "COTLoadoutStore";
		typename t = cls.ToType();
		if (!t)
			return false;

		Class inst = Class.Cast(t.Spawn());
		if (!inst)
			return false;

		Param3<EntityAI, string, int> p = new Param3<EntityAI, string, int>(target, steamID, -1);
		g_Game.GameScript.CallFunctionParams(inst, "RestoreOnto", NULL, p);
		return true;
	}

	// Variante datee : applique le loadout de mort dont l'horodatage est le plus proche
	// de 'killDateTime' (format "YYYY-MM-DD hh:mm:ss"), a +/- toleranceSec secondes.
	bool ApplyDeathLoadoutNearest(EntityAI target, string steamID, string killDateTime, int toleranceSec)
	{
		if (!target || steamID == "")
			return false;

		if (!IsAvailable())
			return false;

		string cls = "COTLoadoutStore";
		typename t = cls.ToType();
		if (!t)
			return false;

		Class inst = Class.Cast(t.Spawn());
		if (!inst)
			return false;

		Param4<EntityAI, string, string, int> p = new Param4<EntityAI, string, string, int>(target, steamID, killDateTime, toleranceSec);
		g_Game.GameScript.CallFunctionParams(inst, "RestoreOntoNearest", NULL, p);
		return true;
	}
}
