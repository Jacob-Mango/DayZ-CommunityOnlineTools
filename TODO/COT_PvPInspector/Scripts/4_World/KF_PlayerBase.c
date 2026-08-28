// Capture serveur des kills PvP.
// On fige un instantane a l'impact (EEHitBy) : c'est le moment reel du tir / de
// l'explosion, donc la visee/visibilite y sont correctes et l'explosif existe
// encore (il est supprime juste apres). On finalise a la mort (EEKilled).
modded class PlayerBase
{
	static const int KF_SNAPSHOT_WINDOW = 120000; // 2 min (couvre les saignements)
	static const bool KF_DEBUG = true;            // logs serveur dans le .RPT (a passer a false ensuite)

	protected ref KF_KillRecord m_KF_Snapshot;
	protected float m_KF_SnapshotTime;

	// Pose forcee des dummies de scene (-1 = aucune). Synchronisee pour que le CLIENT
	// applique aussi le verrou (sinon la mise en joue / raised ne s'affiche pas).
	protected int m_KF_PoseStanceSync = -1;

	static void KF_Log(string msg)
	{
		if (KF_DEBUG)
			PrintToRPT("[KillFeed] " + msg);
	}

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableInt("m_KF_PoseStanceSync", -1, 5);
	}

	// Pose le VERROU de stance UNE fois. ForceStance est un verrou (relache par
	// ForceStance(-1)) : appele une seule fois apres stabilisation, il maintient la
	// posture/mise en joue sans qu'on ait a le re-forcer (le re-forcer relancerait
	// l'animation a chaque appel -> soubresauts).
	void KF_LockStance()
	{
		if (m_KF_PoseStanceSync < 0)
			return;
		HumanCommandMove hcm = GetCommand_Move();
		if (!hcm)
			hcm = StartCommand_Move();
		if (hcm)
			hcm.ForceStance(m_KF_PoseStanceSync);
	}

	// Serveur : enregistre la pose, pose le verrou et synchronise au client.
	void KF_SetPose(int stance)
	{
		m_KF_PoseStanceSync = stance;
		KF_LockStance();
		SetSynchDirty();
	}

	// Client : pose le verrou des reception de la variable synchronisee (une fois).
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		KF_LockStance();
	}

	// --- Resolution d'un tireur "direct" (arme a feu / melee), ordre vanilla ---
	// Le vanilla (PluginAdminLog.PlayerKilled) prend source = l'ARME et remonte
	// au porteur via GetHierarchyParent(). On reproduit exactement ca.
	static PlayerBase KF_ResolveShooterPlayer(Object src)
	{
		if (!src)
			return null;

		EntityAI e;
		if (Class.CastTo(e, src))
		{
			PlayerBase parent;
			if (Class.CastTo(parent, e.GetHierarchyParent()))
				return parent;
		}

		PlayerBase direct;
		if (Class.CastTo(direct, src))
			return direct;

		if (e)
		{
			PlayerBase root;
			Man m = e.GetHierarchyRootPlayer();
			if (m && Class.CastTo(root, m))
				return root;
		}

		return null;
	}

	static void KF_SetKillerIdentity(KF_KillRecord rec, PlayerBase p)
	{
		if (p && p.GetIdentity())
		{
			rec.KillerName = p.GetIdentity().GetName();
			rec.KillerGUID = p.GetIdentity().GetId();
			rec.KillerSteamId = p.GetIdentity().GetPlainId();
		}
		else
		{
			rec.KillerName = "Inconnu";
			rec.KillerGUID = "";
			rec.KillerSteamId = "";
		}
	}

	static string KF_WeaponName(PlayerBase shooter)
	{
		if (!shooter)
			return "";
		EntityAI inHands = shooter.GetHumanInventory().GetEntityInHands();
		if (inHands)
			return inHands.GetDisplayName();
		return "Mains nues";
	}

	static string KF_Pad2(int v)
	{
		if (v < 10)
			return "0" + v.ToString();
		return v.ToString();
	}

	static string KF_NowStr()
	{
		int y, mo, d, h, mi, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, mi, s);
		return y.ToString() + "-" + KF_Pad2(mo) + "-" + KF_Pad2(d) + " " + KF_Pad2(h) + ":" + KF_Pad2(mi) + ":" + KF_Pad2(s);
	}

	// Construit les champs "tueur" + cause a partir de la source du degat.
	// Renvoie false si ce n'est pas un kill attribuable a un joueur.
	static int KF_StanceOf(Man p)
	{
		if (!p)
			return 0;
		PlayerBase pb = PlayerBase.Cast(p);
		if (!pb)
			return 0;
		HumanMovementState hms = new HumanMovementState();
		pb.GetMovementState(hms);
		return hms.m_iStanceIdx;
	}

	static bool KF_BuildKiller(Object source, PlayerBase victim, KF_KillRecord rec)
	{
		if (!source || !victim)
			return false;

		vector vPos = victim.GetPosition();

		// 1) Tir direct (arme a feu / melee) : la source resout vers un joueur.
		PlayerBase shooter = KF_ResolveShooterPlayer(source);
		if (shooter && shooter != victim)
		{
			vector sPos = shooter.GetPosition();
			rec.KillerPos = sPos;
			rec.KillerOri = shooter.GetOrientation();
			rec.KillerStance = KF_StanceOf(shooter);
			rec.Distance  = vector.Distance(sPos, vPos);
			rec.Weapon    = KF_WeaponName(shooter);

			EntityAI inHands = shooter.GetHumanInventory().GetEntityInHands();
			if (inHands)
				rec.KillerHands = inHands.GetType();
			if (inHands && inHands.IsWeapon())
				rec.Cause = "Arme a feu";
			else if (inHands)
				rec.Cause = "Melee";
			else
				rec.Cause = "Mains nues";

			KF_SetKillerIdentity(rec, shooter);

			rec.CombatInfo = true;
			rec.LoS        = KF_Visibility.HasLineOfSight(shooter, victim);
			rec.AimAngle   = KF_Visibility.AimAngleDeg(shooter.GetDirection(), sPos, vPos);
			rec.AimLegit   = (rec.AimAngle <= KF_Visibility.AIM_LEGIT_DEG);
			return true;
		}

		// 2) Explosif / piege : on lit le tag proprietaire pose au lancer/armement.
		ItemBase expl;
		if (Class.CastTo(expl, source) && (expl.IsExplosive() || expl.IsInherited(TrapBase)))
		{
			if (!expl.KF_HasOwner())
				return false; // explosion non attribuee a un joueur -> ignore

			rec.KillerName = expl.KF_OwnerName();
			rec.KillerGUID = expl.KF_OwnerGUID();
			rec.KillerSteamId = expl.KF_OwnerSteamId();
			rec.KillerPos  = expl.KF_OwnerPos();
			rec.KillerOri  = expl.KF_OwnerOri();
			rec.KillerStance = expl.KF_OwnerStance();
			rec.KillerHands = expl.KF_OwnerHands();
			rec.Weapon     = expl.GetDisplayName();
			rec.Distance   = vector.Distance(expl.KF_OwnerPos(), vPos);

			if (expl.IsInherited(Grenade_Base))
				rec.Cause = "Grenade";
			else if (expl.IsInherited(TrapBase))
				rec.Cause = "Piege";
			else
				rec.Cause = "Explosif";

			rec.CombatInfo = false;
			rec.LoS = true;
			rec.AimLegit = true;
			rec.AimAngle = 0;
			return true;
		}

		// 3) Vehicule : le conducteur est responsable.
		if (source.IsTransport())
		{
			Transport veh;
			if (Class.CastTo(veh, source))
			{
				Human driverH = veh.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER);
				PlayerBase driver;
				if (driverH && Class.CastTo(driver, driverH) && driver != victim)
				{
					vector dPos = veh.GetPosition();
					rec.KillerPos = dPos;
					rec.KillerOri = veh.GetOrientation();
					rec.KillerStance = KF_StanceOf(driver);
					EntityAI dHand = driver.GetHumanInventory().GetEntityInHands();
					if (dHand)
						rec.KillerHands = dHand.GetType();
					rec.Distance  = vector.Distance(dPos, vPos);
					rec.Weapon    = veh.GetDisplayName();
					rec.Cause     = "Vehicule";

					KF_SetKillerIdentity(rec, driver);

					rec.CombatInfo = false;
					rec.LoS = true;
					rec.AimLegit = true;
					rec.AimAngle = 0;
					return true;
				}
			}
		}

		return false;
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!GetGame() || !GetGame().IsServer())
			return;

		string srcName = "null";
		if (source)
			srcName = source.GetType();
		KF_Log("EEHitBy victime=" + GetType() + " source=" + srcName);

		KF_KillRecord rec = new KF_KillRecord();
		if (!KF_BuildKiller(source, this, rec))
		{
			KF_Log("EEHitBy -> non attribue (pas de tueur joueur)");
			return;
		}

		rec.Ammo = ammo;

		m_KF_Snapshot = rec;
		m_KF_SnapshotTime = GetGame().GetTime();
		KF_Log("EEHitBy -> snapshot tueur=" + rec.KillerName + " cause=" + rec.Cause);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame() || !GetGame().IsServer())
			return;

		string kName = "null";
		if (killer)
			kName = killer.GetType();
		KF_Log("EEKilled victime=" + GetType() + " killer=" + kName);

		KF_KillRecord rec;

		// 1) On privilegie l'instantane d'impact (visee/visibilite correctes,
		//    explosif encore vivant).
		if (m_KF_Snapshot && (GetGame().GetTime() - m_KF_SnapshotTime) <= KF_SNAPSHOT_WINDOW)
		{
			rec = m_KF_Snapshot;
			KF_Log("EEKilled -> utilise le snapshot");
		}
		else
		{
			// 2) Repli : on tente de resoudre depuis le killer de la mort.
			rec = new KF_KillRecord();
			if (!KF_BuildKiller(killer, this, rec))
			{
				KF_Log("EEKilled -> non enregistre (mort non-PvP ou tueur introuvable)");
				return; // mort non-PvP (zombie, chute, faim...) : on n'enregistre pas
			}
			KF_Log("EEKilled -> reconstruit depuis killer");
		}

		// Infos victime (figees a la mort).
		rec.VictimPos = GetPosition();
		rec.VictimOri = GetOrientation();
		// Posture de la victime a l'instant de la mort (debout/accroupi/allonge).
		HumanMovementState kfHms = new HumanMovementState();
		GetMovementState(kfHms);
		rec.VictimStance = kfHms.m_iStanceIdx;
		EntityAI vHand = GetHumanInventory().GetEntityInHands();
		if (vHand)
			rec.VictimHands = vHand.GetType();
		if (GetIdentity())
		{
			rec.VictimName = GetIdentity().GetName();
			rec.VictimGUID = GetIdentity().GetId();
			rec.VictimSteamId = GetIdentity().GetPlainId();
		}
		else
		{
			rec.VictimName = "Inconnu";
			rec.VictimGUID = "";
			rec.VictimSteamId = "";
		}

		rec.TimeStr = KF_NowStr();

		KF_Store.Add(rec);
		KF_Log("ENREGISTRE: " + rec.KillerName + " >> " + rec.VictimName + " (" + rec.Cause + ") total=" + KF_Store.s_Kills.Count());

		m_KF_Snapshot = null;
	}
}
