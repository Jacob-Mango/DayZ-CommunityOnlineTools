// Calculs serveur : ligne de vue (tir a vue / tir aveugle) et angle de visee (cadrage).
class KF_Visibility
{
	// Seuil d'angle (deg) en dessous duquel le cadrage est juge "legitime".
	// Le tireur regardait grosso modo vers la cible. Au-dela = "suspect".
	static const float AIM_LEGIT_DEG = 35.0;

	// Position des yeux (os "head"), avec repli si l'os est introuvable.
	static vector EyePos(PlayerBase p)
	{
		vector pos = p.GetPosition();
		pos[1] = pos[1] + 1.6;
		int bone = p.GetBoneIndexByName("head");
		if (bone != -1)
			return p.GetBonePositionWS(bone);
		return pos;
	}

	// Point vise sur la cible (tete), avec repli.
	static vector HeadPos(PlayerBase p)
	{
		vector pos = p.GetPosition();
		pos[1] = pos[1] + 1.6;
		int bone = p.GetBoneIndexByName("head");
		if (bone != -1)
			return p.GetBonePositionWS(bone);
		return pos;
	}

	// Point vise alternatif (poitrine).
	static vector ChestPos(PlayerBase p)
	{
		vector pos = p.GetPosition();
		pos[1] = pos[1] + 1.3;
		int bone = p.GetBoneIndexByName("Spine2");
		if (bone != -1)
			return p.GetBonePositionWS(bone);
		return pos;
	}

	// Un raycast unique entre 'from' et la position de 'target'.
	// On utilise DayZPhysics.RaycastRV (raycast MONDE : touche la geometrie statique
	// des batiments/murs de la carte, ce que RaycastRVProxy ne faisait pas) en
	// geometrie de TIR. On compare la distance du 1er contact a celle de la cible :
	// si un solide est nettement devant la cible -> tir aveugle.
	static bool RayClear(vector from, vector to, PlayerBase shooter, PlayerBase target)
	{
		vector hitPos;
		vector hitNormal;
		int hitComp;

		// with = NULL (tout objet), ignore = shooter. hitPos = contact le plus proche.
		bool hit = DayZPhysics.RaycastRV(from, to, hitPos, hitNormal, hitComp, NULL, NULL, shooter, false, false, ObjIntersectFire, 0.0);

		// Rien sur le trajet : champ libre (les buissons n'ont pas de geometrie de tir,
		// donc ils ne bloquent pas -> on voit "a travers", comportement voulu).
		if (!hit)
			return true;

		float dContact = vector.Distance(from, hitPos);
		float dTarget  = vector.Distance(from, to);

		// Le 1er contact est a la cible ou au-dela (la tolerance absorbe l'epaisseur
		// de la hitbox de la cible) -> rien de solide devant -> a vue.
		if (dContact >= dTarget - 0.5)
			return true;

		// Un obstacle solide (mur, batiment, arbre, vehicule...) est devant la cible.
		return false;
	}

	// Le tireur avait-il la cible en vue ? On teste tete puis poitrine.
	static bool HasLineOfSight(PlayerBase shooter, PlayerBase victim)
	{
		if (!shooter || !victim)
			return false;

		vector eye = EyePos(shooter);

		if (RayClear(eye, HeadPos(victim), shooter, victim))
			return true;
		if (RayClear(eye, ChestPos(victim), shooter, victim))
			return true;

		return false;
	}

	// Angle horizontal (deg) entre la direction du regard/corps du tireur
	// et la direction vers la cible. Le yaw (cap) est fiable cote serveur,
	// le pitch ne l'est pas -> on ignore la composante verticale.
	static float AimAngleDeg(vector shooterDir, vector shooterPos, vector victimPos)
	{
		vector fwd = shooterDir;
		fwd[1] = 0;
		fwd.Normalize();

		vector toV = victimPos - shooterPos;
		toV[1] = 0;
		toV.Normalize();

		float dot = fwd[0] * toV[0] + fwd[2] * toV[2];
		if (dot > 1.0)
			dot = 1.0;
		if (dot < -1.0)
			dot = -1.0;

		return Math.Acos(dot) * Math.RAD2DEG;
	}
}
