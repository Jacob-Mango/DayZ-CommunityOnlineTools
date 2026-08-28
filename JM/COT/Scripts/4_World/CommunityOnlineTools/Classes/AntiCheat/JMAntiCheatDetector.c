// =============================================================================
//  JMAntiCheatDetector.c
//
//  Pure detector functions - one per rule. They never call RPC / webhook
//  directly; they return a struct describing what they found and let the
//  module decide policy. Server-side only.
//
//  Everything here is a function of numbers the module already has. Nothing in
//  this file reads the world, allocates beyond the hit it reports, or knows
//  what a flag is - which is what makes the rules readable and testable in
//  isolation.
// =============================================================================

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
	//!
	//! Counted rather than flagged on the first occurrence: one hit that lands
	//! in the same tick as a bandage or a regen tick can genuinely net out to
	//! zero. A run of them cannot.
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
	//!
	//! A reload raises the count and resets the run, so this cannot be tripped
	//! by normal play: firing N times without the magazine ever going down is
	//! not something the game produces.
	static void DetectInfiniteAmmo( int shotsWithoutDecrement, int threshold, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( shotsWithoutDecrement >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Infinite ammo: " + shotsWithoutDecrement.ToString() + " shots with no cartridge decrement" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Unauthorized RPC
	// -------------------------------------------------------------------------

	//! A client asking the server for something its own UI never offers it.
	//!
	//! The burst count exists so a permission revoked mid-session - where an
	//! already-open form can legitimately fire one stale request - is not
	//! evidence. Past that this is the highest-confidence signal the mod has:
	//! nothing a legitimate client does produces a run of refusals.
	static void DetectDeniedRpcBurst( int deniedInWindow, int threshold, float windowS, string lastPermission, int weight, out array< ref JMAntiCheatHit > hits )
	{
		if ( threshold <= 0 )
			return;

		if ( deniedInWindow >= threshold )
		{
			hits.Insert( new JMAntiCheatHit( weight, "Unauthorized RPC: " + deniedInWindow.ToString() + " denied in " + windowS.ToString() + "s (last: " + lastPermission + ")" ) );
		}
	}

	// -------------------------------------------------------------------------
	//  Kill statistics
	//
	//  These read numbers JMAntiCheatKillStats already collects. Every one of
	//  them needs a sample size first: a player with two kills, both headshots,
	//  is a lucky player, not an aimbot.
	// -------------------------------------------------------------------------

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
	//!
	//! Explosives are exempt outright: the "distance" of a grenade hit is blast
	//! radius, and a mine kills whoever walks onto it from wherever its owner
	//! was standing when they placed it - possibly the far side of the map.
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

	//! Silent aim: a ranged hit that landed on somebody the shooter was not
	//! pointing at.
	//!
	//! The cone is deliberately wide, and flattened to the horizontal plane. It
	//! is not an accuracy check - a player whose body has not caught up with a
	//! fast flick would fail a tight cone constantly. It is here to catch a hit
	//! landing behind the shooter, which nothing legitimate produces.
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
