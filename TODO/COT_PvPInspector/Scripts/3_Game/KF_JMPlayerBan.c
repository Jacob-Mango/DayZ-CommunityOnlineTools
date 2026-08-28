// Ajoute le SteamID64 au fichier de ban natif de COT.
// COT indexe toujours le ban par GUID (nom de fichier) ; on ajoute juste le SteamID
// a l'interieur pour que la liste des bans du KillFeed puisse l'afficher et ouvrir
// le profil Steam, sans changer la cle de ban.
//
// Stocke en STRING : un SteamID64 fait ~17 chiffres et depasse l'int 32 bits d'Enforce
// Script (et la precision d'un float). Le JSON ecrira donc "SteamID": "765...".
// Fonctionnellement identique pour l'affichage et le lien steamcommunity.
modded class JMPlayerBan
{
	string SteamID;
}
