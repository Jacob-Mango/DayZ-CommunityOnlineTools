// Un enregistrement de kill. Serialisable a la fois pour le JSON (JsonFileLoader)
// et pour la synchro RPC serveur -> client (Write/Read).
class KF_KillRecord
{
	string KillerName;
	string KillerGUID;
	vector KillerPos;

	string VictimName;
	string VictimGUID;
	vector VictimPos;

	string Weapon;
	string Ammo;
	float  Distance;

	string Cause;       // "Arme a feu", "Melee", "Grenade", "Explosif", "Piege", "Vehicule"
	bool   CombatInfo;  // true => infos visibilite/cadrage pertinentes (tir direct)

	bool   LoS;         // true = tir a vue (cible visible) / false = tir aveugle
	bool   AimLegit;    // true = cadrage legitime / false = cadrage suspect
	float  AimAngle;    // angle (deg) entre la direction du tireur et la direction vers la cible

	int    Id;          // identifiant croissant (tri : plus grand = plus recent)
	string TimeStr;     // horodatage lisible "AAAA-MM-JJ HH:MM:SS"

	string KillerSteamId; // GetPlainId() du tueur (cle loadout LM), si dispo
	string VictimSteamId; // GetPlainId() de la victime (cle loadout LM), si dispo

	vector KillerOri;     // orientation (yaw/pitch/roll) du tueur a l'instant du tir
	int    KillerStance;  // posture du tueur au tir : 0-2 base, 3-5 = arme levee (vise)
	string KillerHands;   // classname de l'objet/arme en mains du tueur au tir
	vector VictimOri;     // orientation de la victime a la mort
	int    VictimStance;  // posture a la mort : 0-2 base, 3-5 = arme levee (visait)
	string VictimHands;   // classname de l'objet/arme en mains de la victime a la mort

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(KillerName);
		ctx.Write(KillerGUID);
		ctx.Write(KillerPos);
		ctx.Write(VictimName);
		ctx.Write(VictimGUID);
		ctx.Write(VictimPos);
		ctx.Write(Weapon);
		ctx.Write(Ammo);
		ctx.Write(Distance);
		ctx.Write(Cause);
		ctx.Write(CombatInfo);
		ctx.Write(LoS);
		ctx.Write(AimLegit);
		ctx.Write(AimAngle);
		ctx.Write(Id);
		ctx.Write(TimeStr);
		ctx.Write(KillerSteamId);
		ctx.Write(VictimSteamId);
		ctx.Write(KillerOri);
		ctx.Write(KillerStance);
		ctx.Write(KillerHands);
		ctx.Write(VictimOri);
		ctx.Write(VictimStance);
		ctx.Write(VictimHands);
	}

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(KillerName)) return false;
		if (!ctx.Read(KillerGUID)) return false;
		if (!ctx.Read(KillerPos)) return false;
		if (!ctx.Read(VictimName)) return false;
		if (!ctx.Read(VictimGUID)) return false;
		if (!ctx.Read(VictimPos)) return false;
		if (!ctx.Read(Weapon)) return false;
		if (!ctx.Read(Ammo)) return false;
		if (!ctx.Read(Distance)) return false;
		if (!ctx.Read(Cause)) return false;
		if (!ctx.Read(CombatInfo)) return false;
		if (!ctx.Read(LoS)) return false;
		if (!ctx.Read(AimLegit)) return false;
		if (!ctx.Read(AimAngle)) return false;
		if (!ctx.Read(Id)) return false;
		if (!ctx.Read(TimeStr)) return false;
		if (!ctx.Read(KillerSteamId)) return false;
		if (!ctx.Read(VictimSteamId)) return false;
		if (!ctx.Read(KillerOri)) return false;
		if (!ctx.Read(KillerStance)) return false;
		if (!ctx.Read(KillerHands)) return false;
		if (!ctx.Read(VictimOri)) return false;
		if (!ctx.Read(VictimStance)) return false;
		if (!ctx.Read(VictimHands)) return false;
		return true;
	}
}

// Conteneur racine pour le fichier JSON.
class KF_SaveData
{
	int NextId = 1;
	ref array<ref KF_KillRecord> Kills;

	void KF_SaveData()
	{
		Kills = new array<ref KF_KillRecord>;
	}
}
