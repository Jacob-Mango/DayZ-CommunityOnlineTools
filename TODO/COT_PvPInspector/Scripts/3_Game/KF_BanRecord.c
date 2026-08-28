// Vue d'un ban COT pour l'affichage cote client (fenetre "Liste des bans").
// Les bans sont stockes par COT (un fichier JSON par joueur, cle = GUID) avec
// seulement un Message et une duree d'expiration (timestamp UTC, <=0 = permanent).
class KF_BanRecord
{
	string Identifier;   // cle du fichier de ban COT (GUID) -> sert au deban (COT utilise le GUID)
	string Message;      // message du ban (KillFeed y met le nom + l'admin)
	int    BanDuration;  // timestamp UTC d'expiration, <= 0 = permanent
	string SteamId;      // SteamID64 resolu (affichage + lien profil Steam), peut etre vide

	// Duree restante lisible, calculee avec le "maintenant" serveur fourni.
	// Renvoie des cles #STR pour Permanent/Expire (SetText les traduit automatiquement).
	string RemainingStr(int serverNow)
	{
		if (BanDuration <= 0)
			return "#STR_GKFM_PERMANENT";
		int remain = BanDuration - serverNow;
		if (remain <= 0)
			return "#STR_GKFM_EXPIRED";
		return KF_Time.HumanDuration(remain);
	}

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(Identifier);
		ctx.Write(Message);
		ctx.Write(BanDuration);
		ctx.Write(SteamId);
	}

	bool Read(ParamsReadContext ctx)
	{
		if (!ctx.Read(Identifier)) return false;
		if (!ctx.Read(Message)) return false;
		if (!ctx.Read(BanDuration)) return false;
		if (!ctx.Read(SteamId)) return false;
		return true;
	}
}
