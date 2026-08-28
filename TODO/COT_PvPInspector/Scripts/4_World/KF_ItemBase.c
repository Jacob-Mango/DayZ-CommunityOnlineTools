// Tag du "proprietaire" pose sur les explosifs et pieges au moment ou un joueur
// les lance / pose / arme. On lit ce tag a l'explosion pour attribuer le kill.
// Place sur ItemBase = ancetre commun de ExplosivesBase (grenades, claymore,
// plastic, improvised) et de TrapBase (bear trap, mine, tripwire).
modded class ItemBase
{
	string m_KF_OwnerGUID;
	string m_KF_OwnerName;
	string m_KF_OwnerSteamId;
	vector m_KF_OwnerPos;
	vector m_KF_OwnerOri;
	string m_KF_OwnerHands;
	bool   m_KF_HasOwner;
	int    m_KF_OwnerStance;

	// Ne tagge que ce qui peut tuer a retardement (explosif ou piege).
	bool KF_IsTrackable()
	{
		return IsExplosive() || IsInherited(TrapBase);
	}

	void KF_SetOwner(Man player)
	{
		if (!player)
			return;

		PlayerBase pb = PlayerBase.Cast(player);
		if (!pb)
			return;

		m_KF_OwnerPos = pb.GetPosition();
		m_KF_OwnerOri = pb.GetOrientation();
		EntityAI kfHand = pb.GetHumanInventory().GetEntityInHands();
		if (kfHand)
			m_KF_OwnerHands = kfHand.GetType();
		HumanMovementState kfHms = new HumanMovementState();
		pb.GetMovementState(kfHms);
		m_KF_OwnerStance = kfHms.m_iStanceIdx;
		m_KF_HasOwner = true;

		if (pb.GetIdentity())
		{
			m_KF_OwnerName = pb.GetIdentity().GetName();
			m_KF_OwnerGUID = pb.GetIdentity().GetId();
			m_KF_OwnerSteamId = pb.GetIdentity().GetPlainId();
		}
		else
		{
			m_KF_OwnerName = "Inconnu";
			m_KF_OwnerGUID = "";
			m_KF_OwnerSteamId = "";
		}
	}

	bool   KF_HasOwner()    { return m_KF_HasOwner; }
	string KF_OwnerName()   { return m_KF_OwnerName; }
	string KF_OwnerGUID()   { return m_KF_OwnerGUID; }
	string KF_OwnerSteamId(){ return m_KF_OwnerSteamId; }
	vector KF_OwnerPos()    { return m_KF_OwnerPos; }
	vector KF_OwnerOri()    { return m_KF_OwnerOri; }
	string KF_OwnerHands()  { return m_KF_OwnerHands; }
	int    KF_OwnerStance() { return m_KF_OwnerStance; }

	// Lancer d'une grenade, depot d'un explosif/piege au sol : sortie d'inventaire.
	override void OnInventoryExit(Man player)
	{
		super.OnInventoryExit(player);

		if (GetGame() && GetGame().IsServer() && KF_IsTrackable())
			KF_SetOwner(player);
	}

	// Pose terminee (claymore, plastic, mine...) : attribution la plus fiable.
	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		super.OnPlacementComplete(player, position, orientation);

		if (GetGame() && GetGame().IsServer() && KF_IsTrackable())
			KF_SetOwner(player);
	}
}
