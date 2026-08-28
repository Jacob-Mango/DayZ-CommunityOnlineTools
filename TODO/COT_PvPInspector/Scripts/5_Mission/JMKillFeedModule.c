class JMKillFeedModule: JMRenderableModuleBase
{
	private ref array<ref KF_KillRecord> m_Kills;

	// Cache client de la liste des bans (rempli via SendBanList ou directement sur l'hote).
	private ref array<ref KF_BanRecord> m_Bans;

	// "Maintenant" serveur (epoch 2020) au moment de l'envoi de la liste, pour calculer
	// la duree restante cote client sans souci de fuseau horaire.
	private int m_BansServerNow;

	// Dummies de reproduction de scene : admin -> (killId -> entite).
	// Un dummy par kill et par admin ; plusieurs kills => plusieurs dummies simultanes.
	private ref map<string, ref map<int, EntityAI>> m_Dummies;

	// Id du kill dont la fiche detail est ouverte (-1 = vue liste). Persiste tant que
	// le module vit, donc la fiche se rouvre apres une fermeture/reouverture de COT.
	private int m_ViewedKillId = -1;

	// Tolerance (s) pour apparier le snapshot de mort au kill (latence serveur).
	private const int KF_DEATH_MATCH_TOLERANCE = 30;

	void JMKillFeedModule()
	{
		GetPermissionsManager().RegisterPermission("KillFeed.View");
		GetPermissionsManager().RegisterPermission("KillFeed.Teleport");
		GetPermissionsManager().RegisterPermission("KillFeed.Clear");
		GetPermissionsManager().RegisterPermission("KillFeed.Spawn");
		GetPermissionsManager().RegisterPermission("KillFeed.Ban");

		m_Kills = new array<ref KF_KillRecord>;
		m_Bans = new array<ref KF_BanRecord>;
		m_Dummies = new map<string, ref map<int, EntityAI>>;
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("KillFeed.View");
	}

	override string GetLayoutRoot()
	{
		return "COT_PvPInspector/GUI/layouts/killfeed/KillFeed_Menu.layout";
	}

	override string GetTitle()
	{
		return "#STR_GKF_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "COT_PvPInspector\\GUI\\textures\\skull.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override int GetRPCMin()
	{
		return JMKillFeedModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMKillFeedModuleRPC.COUNT;
	}

	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
		case JMKillFeedModuleRPC.RequestKills:
			RPC_RequestKills(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.SendKills:
			RPC_SendKills(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.ClearKills:
			RPC_ClearKills(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.SpawnDummy:
			RPC_SpawnDummy(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.CleanScene:
			RPC_CleanScene(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.CleanAll:
			RPC_CleanAll(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.SetSelfOri:
			RPC_SetSelfOri(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.RequestBan:
			RPC_RequestBan(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.RequestUnban:
			RPC_RequestUnban(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.RequestBanList:
			RPC_RequestBanList(ctx, sender, target);
			break;
		case JMKillFeedModuleRPC.SendBanList:
			RPC_SendBanList(ctx, sender, target);
			break;
		}
	}

	// ---- Demande de la liste ----
	void RequestKills()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMKillFeedModuleRPC.RequestKills, true);
		}
		else
		{
			KF_Store.Load();
			m_Kills.Clear();
			foreach (KF_KillRecord r: KF_Store.s_Kills)
				m_Kills.Insert(r);

			JMKillFeedMenu form;
			if (Class.CastTo(form, GetForm()))
				form.LoadKills();
		}
	}

	private void RPC_RequestKills(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.View", senderRPC))
			return;

		KF_Store.Load();

		auto rpc = new ScriptRPC();
		rpc.Write(KF_Store.s_Kills.Count());
		foreach (KF_KillRecord r: KF_Store.s_Kills)
			r.Write(rpc);

		rpc.Send(NULL, JMKillFeedModuleRPC.SendKills, true, senderRPC);
	}

	private void RPC_SendKills(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionClient())
			return;

		int count;
		if (!ctx.Read(count))
		{
			Error("KillFeed: lecture du compteur impossible");
			return;
		}

		m_Kills.Clear();

		while (count)
		{
			KF_KillRecord r = new KF_KillRecord();
			if (!r.Read(ctx))
			{
				Error("KillFeed: lecture d'un enregistrement impossible");
				return;
			}
			m_Kills.Insert(r);
			count--;
		}

		JMKillFeedMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadKills();
	}

	// ---- Vider le journal ----
	void ClearKills()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMKillFeedModuleRPC.ClearKills, true);
		}
		else
		{
			KF_Store.Clear();
			RequestKills();
		}
	}

	private void RPC_ClearKills(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.Clear", senderRPC))
			return;

		KF_Store.Clear();

		RPC_RequestKills(ctx, senderRPC, target);
	}

	// ---- Spawn / masquage des dummies de scene ----
	void RequestSpawnDummy(string steamID, vector pos, vector ori, int stance, string handsClass, string killTime, int killId, int role)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(steamID);
			rpc.Write(pos);
			rpc.Write(ori);
			rpc.Write(stance);
			rpc.Write(handsClass);
			rpc.Write(killTime);
			rpc.Write(killId);
			rpc.Write(role);
			rpc.Send(NULL, JMKillFeedModuleRPC.SpawnDummy, true);
		}
		else
		{
			DoSpawnDummy(steamID, pos, ori, stance, handsClass, killTime, killId, role, GetLocalPlainId());
		}
	}

	void RequestCleanScene(int killId, int role)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(killId);
			rpc.Write(role);
			rpc.Send(NULL, JMKillFeedModuleRPC.CleanScene, true);
		}
		else
		{
			DoCleanScene(killId, role, GetLocalPlainId());
		}
	}

	void RequestCleanAll()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMKillFeedModuleRPC.CleanAll, true);
		}
		else
		{
			DoCleanAll(GetLocalPlainId());
		}
	}

	// Oriente le joueur appelant (l'orientation n'est pas geree par le module Teleport COT).
	void RequestSetSelfOri(vector ori)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(ori);
			rpc.Send(NULL, JMKillFeedModuleRPC.SetSelfOri, true);
		}
		else
		{
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb)
				pb.SetOrientation(ori);
		}
	}

	private void RPC_SpawnDummy(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.Spawn", senderRPC))
			return;

		string steamID;
		vector pos;
		vector ori;
		int stance;
		string handsClass;
		string killTime;
		int killId;
		int role;
		if (!ctx.Read(steamID))
			return;
		if (!ctx.Read(pos))
			return;
		if (!ctx.Read(ori))
			return;
		if (!ctx.Read(stance))
			return;
		if (!ctx.Read(handsClass))
			return;
		if (!ctx.Read(killTime))
			return;
		if (!ctx.Read(killId))
			return;
		if (!ctx.Read(role))
			return;

		DoSpawnDummy(steamID, pos, ori, stance, handsClass, killTime, killId, role, AdminOf(senderRPC));
	}

	private void RPC_CleanScene(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.Spawn", senderRPC))
			return;

		int killId;
		int role;
		if (!ctx.Read(killId))
			return;
		if (!ctx.Read(role))
			return;

		DoCleanScene(killId, role, AdminOf(senderRPC));
	}

	private void RPC_CleanAll(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.Spawn", senderRPC))
			return;

		DoCleanAll(AdminOf(senderRPC));
	}

	private void RPC_SetSelfOri(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;

		if (!GetPermissionsManager().HasPermission("KillFeed.Teleport", senderRPC))
			return;

		vector ori;
		if (!ctx.Read(ori))
			return;

		PlayerBase pb = GetPlayerObjectByIdentity(senderRPC);
		if (pb)
			pb.SetOrientation(ori);
	}

	// ---- Logique serveur (dummies) ----
	// Cherche sur le dummy un item d'un type donne (arme posee par le LoadoutManager).
	private EntityAI KF_FindItemOfType(PlayerBase pb, string type)
	{
		if (type == "")
			return NULL;
		array<EntityAI> items = new array<EntityAI>();
		pb.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI it : items)
		{
			if (it && it.GetType() == type)
				return it;
		}
		return NULL;
	}

	// Releve les classnames des accessoires montes sur une arme (optique, chargeur,
	// silencieux...) pour pouvoir les recreer sur l'arme remise en main.
	private void KF_CollectAttachments(EntityAI item, array<string> outTypes)
	{
		if (!item)
			return;
		GameInventory inv = item.GetInventory();
		if (!inv)
			return;
		int n = inv.AttachmentCount();
		for (int i = 0; i < n; i++)
		{
			EntityAI att = inv.GetAttachmentFromIndex(i);
			if (att)
				outTypes.Insert(att.GetType());
		}
	}

	// Restaure la pose exacte du dummy APRES que le LoadoutManager l'ait equipe.
	// Methode du mod HeroesAndBandits (Guard) : on NE deplace PAS l'arme (le take-to-
	// hands serveur ne marche pas sur un dummy et casse la posture). A la place on
	// recree l'arme exacte directement en main avec ses accessoires, et on supprime
	// celle posee par LM pour eviter le doublon. Puis posture synchronisee (raised).
	void KF_ApplyDummyPose(PlayerBase pb, int stanceIdx, string handsClass)
	{
		if (!pb || !pb.IsAlive())
			return;

		if (handsClass != "")
		{
			EntityAI inHands = pb.GetHumanInventory().GetEntityInHands();
			bool haveRight = (inHands && inHands.GetType() == handsClass);
			if (!haveRight)
			{
				// 1) Retrouver l'arme exacte posee par LM (sur le corps) pour ses accessoires.
				array<string> atts = new array<string>();
				EntityAI lmWpn = KF_FindItemOfType(pb, handsClass);

				string found = "none";
				if (lmWpn)
				{
					found = lmWpn.GetType();
					KF_CollectAttachments(lmWpn, atts);
					GetGame().ObjectDelete(lmWpn);   // supprime l'arme de LM -> plus de doublon
				}
				if (inHands)
					GetGame().ObjectDelete(inHands); // vide la main (mauvais objet)

				// 2) Recreer l'arme exacte directement en main (fiable, n'altere pas la stance).
				EntityAI newWpn = pb.GetHumanInventory().CreateInHands(handsClass);
				int restored = 0;
				if (newWpn)
				{
					foreach (string at : atts)
					{
						newWpn.GetInventory().CreateAttachment(at);
						restored++;
					}
				}
				Print("[PvPInspector] Equip hands='" + handsClass + "' LM=" + found + " accessoires=" + restored.ToString());
			}
		}
	}

	private void DoSpawnDummy(string steamID, vector pos, vector ori, int stance, string handsClass, string killTime, int killId, int role, string admin)
	{
		map<int, EntityAI> am = GetAdminMap(admin, true);
		int key = killId * 2 + role; // role 0 = victime, 1 = tireur (dummies distincts)

		// Re-afficher le meme element remplace son dummy precedent.
		if (am.Contains(key))
		{
			EntityAI old = am.Get(key);
			if (old)
				GetGame().ObjectDelete(old);
			am.Remove(key);
		}

		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);

		PlayerBase pb;
		// Spawn façon Guard HeroesAndBandits : CreateObject avec init_ai=false -> AUCUN
		// cerveau IA, donc le moteur ne rabaisse pas l'arme et le verrou ForceStance tient.
		// (CreatePlayer attache une IA qui faisait redescendre la mise en joue.)
		Object dObj = GetGame().CreateObject("SurvivorM_Mirek", pos, false, false, true);
		if (!Class.CastTo(pb, dObj))
			return;
		pb.SetPosition(pos);

		pb.SetAllowDamage(false); // godmode
		pb.SetOrientation(ori);   // orientation de la victime a la mort

		// Habillage au loadout de mort le plus proche du kill (si COT_LoadoutManager charge).
		KF_LoadoutProvider.Get().ApplyDeathLoadoutNearest(pb, steamID, killTime, KF_DEATH_MATCH_TOLERANCE);

		// 1) Equipement de l'arme exacte en main (apres que LM ait pose le loadout).
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(KF_ApplyDummyPose, 800, false, pb, stance, handsClass);

		// 2) Posture SANS mise en joue : on retire le raised (indices 3/4/5 -> 0/1/2), donc
		//    debout/accroupi/couche selon le kill, mais arme baissee (la visee ne tenait pas
		//    sur un dummy, on l'abandonne). Applique 2x par securite (serveur + sync client).
		int sBase = stance;
		if (sBase < 0)
			sBase = DayZPlayerConstants.STANCEIDX_ERECT;
		else if (sBase >= 3)
			sBase = sBase - 3;
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(pb.KF_SetPose, 1200, false, sBase);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(pb.KF_SetPose, 2000, false, sBase);

		am.Set(key, pb);
	}

	private void DoCleanScene(int killId, int role, string admin)
	{
		map<int, EntityAI> am = GetAdminMap(admin, false);
		if (!am)
			return;

		int key = killId * 2 + role;
		if (am.Contains(key))
		{
			EntityAI e = am.Get(key);
			if (e)
				GetGame().ObjectDelete(e);
			am.Remove(key);
		}
	}

	private void DoCleanAll(string admin)
	{
		map<int, EntityAI> am = GetAdminMap(admin, false);
		if (!am)
			return;

		for (int i = 0; i < am.Count(); i++)
		{
			EntityAI e = am.GetElement(i);
			if (e)
				GetGame().ObjectDelete(e);
		}
		am.Clear();
	}

	private map<int, EntityAI> GetAdminMap(string admin, bool create)
	{
		if (m_Dummies.Contains(admin))
			return m_Dummies.Get(admin);
		if (!create)
			return NULL;
		map<int, EntityAI> m = new map<int, EntityAI>;
		m_Dummies.Set(admin, m);
		return m;
	}

	private string AdminOf(PlayerIdentity senderRPC)
	{
		if (senderRPC)
			return senderRPC.GetPlainId();
		return "HOST";
	}

	private string GetLocalPlainId()
	{
		if (GetGame().GetPlayer() && GetGame().GetPlayer().GetIdentity())
			return GetGame().GetPlayer().GetIdentity().GetPlainId();
		return "HOST";
	}

	array<ref KF_KillRecord> GetKills()
	{
		return m_Kills;
	}

	void SetViewedKillId(int id)
	{
		m_ViewedKillId = id;
	}

	int GetViewedKillId()
	{
		return m_ViewedKillId;
	}

	// ===================== BANS (via le systeme COT) =====================
	// On ne gere plus de stockage propre : KillFeed ecrit/lit/supprime directement
	// les fichiers de ban de COT (un JSON par joueur, cle = GUID). COT s'occupe seul
	// du kick a la connexion (avant spawn) ; ici on ne kicke que les joueurs en ligne.

	static const int KF_BAN_MAX_DAYS = 3650; // garde-fou (10 ans)

	array<ref KF_BanRecord> GetBans()
	{
		return m_Bans;
	}

	int GetBansServerNow()
	{
		return m_BansServerNow;
	}

	// ---- Bannir ----
	void RequestBan(string guid, string name, int days, string steamId)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(guid);
			rpc.Write(name);
			rpc.Write(days);
			rpc.Write(steamId);
			rpc.Send(NULL, JMKillFeedModuleRPC.RequestBan, true);
		}
		else
		{
			DoBan(guid, name, days, GetLocalAdminName(), steamId);
			KF_CollectBans(m_Bans, m_BansServerNow);
			RefreshFormBans();
		}
	}

	private void RPC_RequestBan(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;
		if (!GetPermissionsManager().HasPermission("KillFeed.Ban", senderRPC))
			return;

		string guid;
		string name;
		int days;
		if (!ctx.Read(guid))
			return;
		if (!ctx.Read(name))
			return;
		if (!ctx.Read(days))
			return;
		string steamId;
		if (!ctx.Read(steamId))
			return;

		string adminName = "Console";
		if (senderRPC)
			adminName = senderRPC.GetName();

		DoBan(guid, name, days, adminName, steamId);
		SendBanListTo(senderRPC);
	}

	// Supprime le fichier de ban en ciblant son nom EXACT sur le disque.
	// IMPORTANT : l'Identifier vient de FindFilesInLocation (= nom de fichier reel),
	// donc on supprime DIR_BANS + identifier + ".json" tel quel. On NE repasse PAS par
	// FileReadyStripName, qui retire les '=' '+' '/' presents dans les GUID DayZ (base64)
	// et provoquait l'echec (fichier '...=.json' cherche en '....json'). Repli COT ensuite.
	private bool KF_DeleteBan(string identifier)
	{
		if (identifier == "")
			return false;

		bool removed = false;

		string exact = JMConstants.DIR_BANS + identifier + JMConstants.EXT_BAN;
		if (DeleteFile(exact))
			removed = true;

		if (!removed)
		{
			// Repli : methode native COT (utile si le fichier a ete ecrit "strippe").
			if (JMPlayerBan.DeleteBanFile(identifier, identifier))
				removed = true;
		}

		Print("[KillFeed] Unban identifier='" + identifier + "' path='" + exact + "' supprime=" + removed);
		return removed;
	}

	// ---- Debannir ----
	void RequestUnban(string identifier)
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Write(identifier);
			rpc.Send(NULL, JMKillFeedModuleRPC.RequestUnban, true);
		}
		else
		{
			KF_DeleteBan(identifier);
			KF_CollectBans(m_Bans, m_BansServerNow);
			RefreshFormBans();
		}
	}

	private void RPC_RequestUnban(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;
		if (!GetPermissionsManager().HasPermission("KillFeed.Ban", senderRPC))
		{
			Print("[KillFeed] Unban refuse : permission KillFeed.Ban manquante.");
			return;
		}

		string identifier;
		if (!ctx.Read(identifier))
		{
			Print("[KillFeed] Unban : lecture de l'identifiant echouee.");
			return;
		}

		// COT indexe les bans par GUID (nom de fichier). Suppression par nom exact.
		KF_DeleteBan(identifier);

		SendBanListTo(senderRPC);
	}

	// ---- Liste des bans ----
	void RequestBanList()
	{
		if (g_Game.IsClient())
		{
			auto rpc = new ScriptRPC();
			rpc.Send(NULL, JMKillFeedModuleRPC.RequestBanList, true);
		}
		else
		{
			KF_CollectBans(m_Bans, m_BansServerNow);
			RefreshFormBans();
		}
	}

	private void RPC_RequestBanList(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionHost())
			return;
		if (!GetPermissionsManager().HasPermission("KillFeed.Ban", senderRPC))
			return;

		SendBanListTo(senderRPC);
	}

	private void RPC_SendBanList(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (!IsMissionClient())
			return;

		int serverNow;
		if (!ctx.Read(serverNow))
			return;
		m_BansServerNow = serverNow;

		int count;
		if (!ctx.Read(count))
			return;

		m_Bans.Clear();
		while (count)
		{
			KF_BanRecord r = new KF_BanRecord();
			if (!r.Read(ctx))
				return;
			m_Bans.Insert(r);
			count--;
		}

		RefreshFormBans();
	}

	// ---- Logique serveur ----
	private void DoBan(string guid, string name, int days, string adminName, string steamId)
	{
		if (guid == "")
			return;

		string message = name;
		if (adminName != "")
			message = message + " | ban by " + adminName;

		int d = days;
		if (d < 0)
			d = 0;
		if (d > KF_BAN_MAX_DAYS)
			d = KF_BAN_MAX_DAYS;

		JMPlayerBan banData = new JMPlayerBan();
		banData.Message = message;
		banData.SteamID = steamId; // SteamID64 ecrit dans le fichier de ban (cle reste le GUID)
		if (d <= 0)
			banData.BanDuration = -1; // permanent
		else
			banData.BanDuration = CF_Date.Now(true).GetTimestamp() + d * 86400;
		JMPlayerBan.Save(banData, guid);

		KF_KickIfOnline(guid, message);
	}

	// Kick propre (sans drop de loot) d'un joueur banni encore connecte, facon COT.
	private void KF_KickIfOnline(string guid, string message)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man m: players)
		{
			PlayerBase pb = PlayerBase.Cast(m);
			if (!pb || !pb.GetIdentity())
				continue;
			if (pb.GetIdentity().GetId() == guid)
			{
				pb.MessageImportant("#STR_GKFM_BANNED_MSG");
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KF_DoCleanKick, 1000, false, pb, pb.GetIdentity());
				return;
			}
		}
	}

	void KF_DoCleanKick(PlayerBase pb, PlayerIdentity id)
	{
		if (!g_Game || !pb || !id)
			return;
		MissionServer ms = MissionServer.Cast(g_Game.GetMission());
		if (!ms)
			return;
		pb.COTSetIsBeingKicked(true);
		g_Game.SendLogoutTime(pb, 0);
		ms.PlayerDisconnected(pb, id, id.GetId());
	}

	// Enumere les fichiers de ban COT et construit la liste a afficher.
	private void KF_CollectBans(out array<ref KF_BanRecord> outList, out int serverNow)
	{
		outList = new array<ref KF_BanRecord>;
		serverNow = CF_Date.Now(true).GetTimestamp();

		array<string> files = FindFilesInLocation(JMConstants.DIR_BANS);
		foreach (string fn: files)
		{
			if (fn == "")
				continue;

			string ident = fn;
			ident.Replace(JMConstants.EXT_BAN, "");

			JMPlayerBan bd = new JMPlayerBan();
			JsonFileLoader<JMPlayerBan>.JsonLoadFile(JMConstants.DIR_BANS + fn, bd);

			// On masque les bans expires (COT les supprimera a la prochaine connexion).
			if (bd.BanDuration > 0 && serverNow > bd.BanDuration)
				continue;

			KF_BanRecord r = new KF_BanRecord();
			r.Identifier = ident;
			r.Message = bd.Message;
			r.BanDuration = bd.BanDuration;
			r.SteamId = bd.SteamID; // SteamID stocke dans le fichier de ban (COT modde)
			if (r.SteamId == "")
				r.SteamId = KF_SteamIdForGuid(ident); // repli : resolu depuis les kills
			outList.Insert(r);
		}
	}

	// Retrouve le SteamID64 d'un GUID a partir des kills enregistres (le KillFeed y
	// stocke deja le couple GUID<->SteamId pour tueur et victime). Renvoie "" si inconnu.
	private string KF_SteamIdForGuid(string guid)
	{
		if (guid == "")
			return "";

		KF_Store.Load();
		foreach (KF_KillRecord k: KF_Store.s_Kills)
		{
			if (!k)
				continue;
			if (k.KillerGUID == guid && k.KillerSteamId != "")
				return k.KillerSteamId;
			if (k.VictimGUID == guid && k.VictimSteamId != "")
				return k.VictimSteamId;
		}
		return "";
	}

	private void SendBanListTo(PlayerIdentity who)
	{
		array<ref KF_BanRecord> all;
		int now;
		KF_CollectBans(all, now);

		auto rpc = new ScriptRPC();
		rpc.Write(now);
		rpc.Write(all.Count());
		foreach (KF_BanRecord r: all)
			r.Write(rpc);
		rpc.Send(NULL, JMKillFeedModuleRPC.SendBanList, true, who);
	}

	private void RefreshFormBans()
	{
		JMKillFeedMenu form;
		if (Class.CastTo(form, GetForm()))
			form.LoadBans();
	}

	private string GetLocalAdminName()
	{
		if (GetGame().GetPlayer() && GetGame().GetPlayer().GetIdentity())
			return GetGame().GetPlayer().GetIdentity().GetName();
		return "Console";
	}
}
