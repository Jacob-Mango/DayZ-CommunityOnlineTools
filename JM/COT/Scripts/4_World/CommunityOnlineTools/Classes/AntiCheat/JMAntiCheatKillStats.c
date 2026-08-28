// =============================================================================
//  JMAntiCheatKillStats.c
//
//  Per-player kill statistics for the anti-cheat module. Server-side only;
//  mirrored to admin clients on demand. The form renders hit/miss ratio,
//  body-part distribution, and engagement distance as a compact panel.
//
//  Data sources:
//    - hit/miss  : `EntityAI.GetDamageHistory()` (last N damage events)
//    - body-part : damage record's `DamageType` + `DamageZone` strings
//    - distance  : attacker position (captured at death time) vs victim
//                   last-known alive position
//
//  Window: last 50 kills. Older entries fall off the back to bound memory.
// =============================================================================

class JMAntiCheatKillStats
{
	string PlayerName;
	string Guid;

	int TotalKills;
	int TotalMisses;        // hits registered but the attack didn't connect (defender survived)
	int BodyHead;
	int BodyTorso;
	int BodyLeftArm;
	int BodyRightArm;
	int BodyLeftLeg;
	int BodyRightLeg;
	int BodyFeet;            // DayZ combines Foot+Hand under Feet in some paths

	float DistanceSum;
	int   DistanceCount;
	float DistanceMax;

	ref array< string > RecentKills;  // formatted "[dist] killed <name> via <bodypart>"

	void JMAntiCheatKillStats()
	{
		TotalKills  = 0;
		TotalMisses = 0;
		BodyHead = 0;
		BodyTorso = 0;
		BodyLeftArm = 0;
		BodyRightArm = 0;
		BodyLeftLeg = 0;
		BodyRightLeg = 0;
		BodyFeet = 0;

		DistanceSum = 0;
		DistanceCount = 0;
		DistanceMax = 0;

		RecentKills = new array< string >;
	}

	// Hit/miss ratio as a 0..1 float. -1 if no shots yet.
	float GetHitRatio()
	{
		int total = TotalKills + TotalMisses;
		if ( total == 0 )
			return -1;

		// Both operands are ints, so without the cast this is integer division
		// and the ratio can only ever come out as 0 or 1.
		return TotalKills / (float)total;
	}

	float GetAvgDistance()
	{
		if ( DistanceCount == 0 )
			return 0;
		return DistanceSum / DistanceCount;
	}

	void RecordHit( string victimName, string bodyPart, float distance, int nowMs )
	{
		TotalKills++;

		if ( bodyPart == "Head" )         BodyHead++;
		else if ( bodyPart == "Torso" )    BodyTorso++;
		else if ( bodyPart == "LeftArm" )  BodyLeftArm++;
		else if ( bodyPart == "RightArm" ) BodyRightArm++;
		else if ( bodyPart == "LeftLeg" )  BodyLeftLeg++;
		else if ( bodyPart == "RightLeg" ) BodyRightLeg++;
		else                                BodyFeet++;

		DistanceSum += distance;
		DistanceCount++;
		if ( distance > DistanceMax )
			DistanceMax = distance;

		string victimDisplay = victimName;
		if ( victimDisplay == "" )
			victimDisplay = "?";
		string entry = "[" + nowMs.ToString() + "] " + distance.ToString() + "m -> " + victimDisplay + " [" + bodyPart + "]";
		RecentKills.Insert( entry );

		while ( RecentKills.Count() > 50 )
			RecentKills.Remove( 0 );
	}

	void RecordMiss( float distance )
	{
		TotalMisses++;
		DistanceSum += distance;
		DistanceCount++;
		if ( distance > DistanceMax )
			DistanceMax = distance;
	}

	void Clear()
	{
		TotalKills  = 0;
		TotalMisses = 0;
		BodyHead = 0;
		BodyTorso = 0;
		BodyLeftArm = 0;
		BodyRightArm = 0;
		BodyLeftLeg = 0;
		BodyRightLeg = 0;
		BodyFeet = 0;
		DistanceSum = 0;
		DistanceCount = 0;
		DistanceMax = 0;
		RecentKills.Clear();
	}
}