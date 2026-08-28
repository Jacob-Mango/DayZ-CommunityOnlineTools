enum JMKillFeedModuleRPC
{
	INVALID = 10720,
	RequestKills,
	SendKills,
	ClearKills,
	SpawnDummy,   // client -> serveur : spawn d'un dummy godmode au stuff de la victime (par kill)
	CleanScene,   // client -> serveur : retire le dummy d'UN kill
	CleanAll,     // client -> serveur : retire TOUS les dummies de l'admin
	SetSelfOri,   // client -> serveur : oriente le joueur appelant (apres un TP)
	RequestBan,   // client -> serveur : bannit un joueur (steamId, nom, jours)
	RequestUnban, // client -> serveur : retire un ban (steamId)
	RequestBanList, // client -> serveur : demande la liste des bans
	SendBanList,  // serveur -> client : envoie la liste des bans
	COUNT
}
