// Application du ban a la connexion via InvokeOnConnect.
//
// Pourquoi InvokeOnConnect et plus OnEvent(ClientNewEventTypeID) :
// les versions recentes de DayZ utilisent une machine a etats pour la connexion
// (WaitAuthPlayerLoginState, PlayerAssignedLoginState, ...). Dans ce flux,
// OnEvent(ClientNewEventTypeID) n'est pas emis, donc le controle de ban n'y passait
// jamais. InvokeOnConnect, lui, est bien appele une fois le personnage charge.
//
// Le joueur apparait donc brievement avant le kick, mais COTSetIsBeingKicked + un
// kick propre (SendLogoutTime 0 + PlayerDisconnected) evitent qu'il laisse un corps
// lootable. Des lignes [KillFeed] sont ecrites dans le .RPT pour verifier.
modded class MissionServer
{
	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!player || !identity)
			return;

		PrintToRPT("[KillFeed] InvokeOnConnect guid=" + identity.GetId() + " steam=" + identity.GetPlainId());

		if (!KF_IsBanned(identity))
			return;

		PrintToRPT("[KillFeed]  -> joueur banni : kick");
		player.MessageImportant("#STR_GKFM_BANNED_MSG");
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KF_KickConnected, 600, false, player, identity);
	}

	void KF_KickConnected(PlayerBase player, PlayerIdentity identity)
	{
		if (!g_Game || !player || !identity)
			return;

		player.COTSetIsBeingKicked(true);     // evite que COT tue/loot le perso au kick
		g_Game.SendLogoutTime(player, 0);     // deconnexion immediate
		PlayerDisconnected(player, identity, identity.GetId());
	}

	private bool KF_IsBanned(PlayerIdentity identity)
	{
		JMPlayerBan banData = JMPlayerBan.Load(identity.GetId(), identity.GetPlainId());
		if (!banData)
		{
			PrintToRPT("[KillFeed]  -> aucun fichier de ban (autorise)");
			return false;
		}

		if (banData.BanDuration > 0)
		{
			int now = CF_Date.Now(true).GetTimestamp();
			PrintToRPT("[KillFeed]  -> ban trouve expire=" + banData.BanDuration + " now=" + now);
			if (now > banData.BanDuration)
			{
				JMPlayerBan.DeleteBanFile(identity.GetId(), identity.GetPlainId());
				PrintToRPT("[KillFeed]  -> ban EXPIRE, fichier supprime (autorise)");
				return false;
			}
		}
		else
		{
			PrintToRPT("[KillFeed]  -> ban PERMANENT");
		}

		return true;
	}
}
