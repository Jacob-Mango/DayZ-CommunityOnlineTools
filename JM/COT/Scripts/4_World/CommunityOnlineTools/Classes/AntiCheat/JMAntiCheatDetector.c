// JMAntiCheatDetector.c - pure detector functions, one per rule. Server-side only.
// They never call RPC / webhook; they return a struct describing the hit and
// the module decides policy. Functions of numbers the module already has: no
// world reads, no allocation beyond the reported hit, no knowledge of flags.

class JMAntiCheatHit
{
	int Weight;
	string Text;

	void JMAntiCheatHit( int w, string t )
	{
		Weight = w;
		Text = t;
	}
}

class JMAntiCheatDetector
{
	// Per-player position tracker (caller passes in the prior position+timestamp)
	static void DetectSpeed( vector prevPos, int prevMs, vector nowPos, int nowMs, float maxSpeed, int weight, out array< ref JMAntiCheatHit > hits )
	{
		float dt = ( nowMs - prevMs ) / 1000.0;
		if ( dt <= 0 )
			return;

		float dist = vector.Distance( prevPos, nowPos );
		float speed = dist / dt;

		if ( speed > maxSpeed )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Speed " + speed.ToString() + " m/s (max " + maxSpeed.ToString() + ")" ) );
		}
	}

	// No-clip - sample ground under player
	static void DetectNoClip( vector groundPos, vector playerPos, float tolerance, int weight, out array< ref JMAntiCheatHit > hits )
	{
		float gap = playerPos[1] - groundPos[1];
		if ( gap > tolerance )
		{
			hits.Insert( new JMAntiCheatHit( weight, "No-clip / fly (gap " + gap.ToString() + "m)" ) );
		}
	}

	// Health anomaly - too much HP change in one server tick
	static void DetectHealthDelta( float prevHp, float nowHp, float maxDelta, int weight, out array< ref JMAntiCheatHit > hits )
	{
		float d = nowHp - prevHp;
		if ( Math.AbsFloat( d ) > maxDelta )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Health delta " + d.ToString() + " in one tick (max " + maxDelta.ToString() + ")" ) );
		}
	}

	// Inventory cheats - items added per second above threshold
	static void DetectItemAddRate( int itemsAdded, float windowS, int maxPerSecond, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( windowS <= 0 )
			return;

		float rate = itemsAdded / windowS;
		if ( rate > maxPerSecond )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Item-add rate " + rate.ToString() + "/s (max " + maxPerSecond.ToString() + ")" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Movement
	// -------------------------------------------------------------------------

	//! A jump, as opposed to travel. Reported separately from speed because the
	//! two cheats look nothing alike and the evidence reads differently: 400m
	//! between two samples is a teleport whether it happened over one second or
	//! ten, while 40 m/s sustained is a speedhack whatever the distance.
	static void DetectTeleport( vector prevPos, vector nowPos, float maxJump, int weight, out array< ref JMAntiCheatHit > hits )
	{
		float dist = vector.Distance( prevPos, nowPos );

		if ( dist > maxJump )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Teleport " + dist.ToString() + "m in one poll (max " + maxJump.ToString() + "m)" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Damage immunity
	// -------------------------------------------------------------------------

	//! Real damage landing on a player whose health does not move.
	//! Counted rather than flagged on first occurrence: one hit in the same tick as
	//! a bandage or regen tick can net out to zero. A run of them cannot.
	static void DetectDamageImmunity( int consecutiveNoDrop, int threshold, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( consecutiveNoDrop >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Damage immunity: " + consecutiveNoDrop.ToString() + " damaging hits with no health loss" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Ammunition
	// -------------------------------------------------------------------------

	//! Shots fired with the cartridge count never falling.
	//! A reload raises the count and resets the run, so normal play cannot trip
	//! this: firing N times without the magazine going down is not producible.
	static void DetectInfiniteAmmo( int shotsWithoutDecrement, int threshold, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( shotsWithoutDecrement >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Infinite ammo: " + shotsWithoutDecrement.ToString() + " shots with no cartridge decrement" ) );
		}
	}

	//! Cartridge count rising outside a reload. DetectInfiniteAmmo only catches
	//! a count that never falls; this catches the opposite manipulation, a
	//! count that climbs on its own between shots.
	static void DetectAmmoIncrease( int prevCount, int nowCount, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( nowCount <= prevCount )
			return;

		hits.Insert( new JMAntiCheatHit( weight, "Ammo increase: " + prevCount.ToString() + " -> " + nowCount.ToString() + " outside a reload" ) );
	}

	// Weapon jam suppression: the server rolls Weapon_Base.GetChanceToJam() per shot;
	// a client cannot change that roll but can suppress the JAMMED state so its own
	// reported jam count never rises. Judged over a minimum sample count so a lucky
	// run of shots is not evidence.
	static void DetectJamSuppression( int shotsFired, int jamsObserved, float expectedJamChance, float toleranceRatio, int minSamples, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( shotsFired < minSamples || expectedJamChance <= 0 )
			return;

		float observedRate = jamsObserved / shotsFired;
		float floor = expectedJamChance * toleranceRatio;

		if ( observedRate < floor )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Jam suppression: " + jamsObserved.ToString() + "/" + shotsFired.ToString() + " jams, expected ~" + expectedJamChance.ToString() + " chance per shot" ) );
		}
	}

	// Spread / accuracy manipulation: configured dispersion is the std deviation of
	// shot angle; a no-spread hack removes it, so actual dispersion over many shots
	// comes out far tighter. Needs a minimum sample count (a tight burst is not a
	// cheat), same as headshot ratio.
	static void DetectSpreadAnomaly( float observedStdDevDeg, float configuredDispersionDeg, float toleranceRatio, int sampleCount, int minSamples, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( sampleCount < minSamples || configuredDispersionDeg <= 0 )
			return;

		float floor = configuredDispersionDeg * toleranceRatio;

		if ( observedStdDevDeg < floor )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Spread anomaly: " + observedStdDevDeg.ToString() + " deg over " + sampleCount.ToString() + " shots (configured " + configuredDispersionDeg.ToString() + ")" ) );
		}
	}

	// Projectile speed manipulation: compares a shot's observed muzzle-to-impact
	// speed against the ammo's configured initSpeed. A shot covering its distance
	// faster than the ammo config allows indicates a speed hack (or removed drop/drag).
	static void DetectProjectileSpeedAnomaly( float observedSpeed, float configuredInitSpeed, float toleranceRatio, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( configuredInitSpeed <= 0 )
			return;

		float ceiling = configuredInitSpeed * toleranceRatio;

		if ( observedSpeed > ceiling )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Projectile speed " + observedSpeed.ToString() + " m/s (configured initSpeed " + configuredInitSpeed.ToString() + ")" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Unauthorized RPC
	// -------------------------------------------------------------------------

	//! A client asking the server for something its own UI never offers it.
	//! The burst count lets a permission revoked mid-session (one stale request from
	//! an open form) pass. Past that it is the highest-confidence signal the mod has:
	//! a legitimate client never produces a run of refusals.
	static void DetectDeniedRpcBurst( int deniedInWindow, int threshold, float windowS, string lastPermission, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( deniedInWindow >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Unauthorized RPC: " + deniedInWindow.ToString() + " denied in " + windowS.ToString() + "s (last: " + lastPermission + ")" ) );
		}
	}

	// Kill statistics: these read numbers JMAntiCheatKillStats already collects.
	// Each needs a sample size first: two kills, both headshots, is a lucky player,
	// not an aimbot.

	static void DetectHeadshotRatio( int headshots, int totalKills, int minKills, float maxRatio, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( totalKills < minKills || totalKills <= 0 )
			return;

		float ratio = headshots / totalKills;

		if ( ratio > maxRatio )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Headshot ratio " + ratio.ToString() + " over " + totalKills.ToString() + " kills (max " + maxRatio.ToString() + ")" ) );
		}
	}

	//! Distance rules, judged per delivery class.
	//! Explosives are exempt: a grenade's 'distance' is blast radius, and a mine kills
	//! whoever walks onto it from wherever its owner stood when placing it.
	static void DetectKillDistance( float distance, bool isMelee, bool isExplosive, float maxMelee, float maxAny, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( isExplosive )
			return;

		if ( isMelee && distance > maxMelee )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Melee reach " + distance.ToString() + "m (max " + maxMelee.ToString() + "m)" ) );
			return;
		}

		if ( distance > maxAny )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Kill at " + distance.ToString() + "m (sanity max " + maxAny.ToString() + "m)" ) );
		}
	}

	static void DetectRapidKills( int killsInWindow, int threshold, float windowS, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( killsInWindow >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Rapid kills: " + killsInWindow.ToString() + " in " + windowS.ToString() + "s" ) );
		}
	}

	//! Silent aim: a ranged hit that landed on somebody the shooter was not pointing at.
	//! The cone is deliberately wide and flattened to the horizontal plane; it is not
	//! an accuracy check (a fast flick would fail a tight cone). It catches hits landing
	//! behind the shooter, which nothing legitimate produces.
	static void DetectSilentAim( vector attackerPos, vector attackerDir, vector victimPos, bool isRanged, float minDistance, float coneDeg, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( !isRanged )
			return;

		vector toVictim = victimPos - attackerPos;
		toVictim[1] = 0;

		float dist = toVictim.Length();
		if ( dist < minDistance )
			return;

		vector aim = attackerDir;
		aim[1] = 0;

		if ( aim.Length() <= 0 )
			return;

		toVictim.Normalize();
		aim.Normalize();

		float dot = vector.Dot( aim, toVictim );
		if ( dot > 1.0 )
			dot = 1.0;
		if ( dot < -1.0 )
			dot = -1.0;

		float angle = Math.Acos( dot ) * Math.RAD2DEG;

		if ( angle > coneDeg )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Silent aim: hit " + angle.ToString() + " deg off aim at " + dist.ToString() + "m (max " + coneDeg.ToString() + ")" ) );
		}
	}
}
