// =============================================================================
//  JMAntiCheatConfig.c
//
//  Tunable thresholds for the anti-cheat module. Loaded once on mission start
//  from $profile:CommunityOnlineTools/AntiCheat/config.json. Edit the file to
//  tighten or loosen detection per server.
//
//  Defaults are conservative - false positives at the lower end, real cheats
//  well above. Adjust to taste.
//
//  A note on weights: they are meant to be read as "how sure are we". A speed
//  reading is 5 because a lagging client produces them; damage immunity is 25
//  because nothing legitimate produces it. FlagThresholdHigh is 15, so one
//  certain detection raises a flag on its own and the uncertain ones have to
//  accumulate.
// =============================================================================

class JMAntiCheatConfig
{
	// Master switch
	bool Enabled = true;

	// Poll cadence - how often (seconds) the server-side scan runs
	float PollInterval = 1.0;

	// Speed / teleport detector - max metres/second before raising a flag
	float MaxSpeed = 12.0;            // 12 m/s ~ sprint + slide; raises above this
	int   SpeedFlagWeight = 5;

	// No-clip / fly detector - gap in seconds between raycast hits that triggers
	float NoClipGroundGap = 2.0;
	int   NoClipFlagWeight = 8;

	// Damage anomaly - health delta in single server tick (lasts 0.05s)
	float MaxHealthDeltaPerTick = 60.0;
	int   DamageFlagWeight = 4;

	// Inventory cheats - items added per second per player
	int MaxItemsAddedPerSecond = 6;
	int InventoryFlagWeight = 10;

	// Modifier flag - if a "debug only" modifier is active outside an admin slot
	int DebugModifierFlagWeight = 20;

	// -- Teleport ------------------------------------------------------------
	//  Distinct from speed: a cheat that blinks 400m and stops looks nothing
	//  like one that runs at 40 m/s, and only the second is a speed reading.
	//  Anything past this in a single poll is a jump, not travel.
	float TeleportDistance = 150.0;
	int   TeleportFlagWeight = 12;

	//  Ceiling while the player is in a vehicle. Without this, every car on the
	//  server trips MaxSpeed every poll and the flag list is useless.
	float MaxVehicleSpeed = 60.0;

	//  Grace after a spawn / respawn / admin teleport, during which position
	//  history is discarded rather than measured. The previous sample belongs
	//  to a different place entirely and would read as a teleport.
	float PositionGraceS = 5.0;

	// -- Damage immunity (godmode) -------------------------------------------
	//  Real damage landing on a player whose health never moves. COT's own
	//  godmode is exempt - that is an admin decision, not a cheat.
	int GodModeHitsBeforeFlag = 5;
	int GodModeFlagWeight = 25;

	// -- Infinite ammo -------------------------------------------------------
	//  Shots fired with the cartridge count never decreasing.
	int AmmoShotsWithoutDecrement = 8;
	int AmmoFlagWeight = 20;

	// -- Kill statistics -----------------------------------------------------
	//  Aimbot heuristics. All of these need a sample size before they mean
	//  anything - a player with two kills, both headshots, is not evidence.
	int   HeadshotMinKills = 12;
	float HeadshotRatioMax = 0.75;
	int   HeadshotFlagWeight = 15;

	//  Melee and fists reach about two metres. A melee kill from further away
	//  is a reach hack, and there is no legitimate reading of it.
	float MaxMeleeDistance = 6.0;
	//  Absolute sanity ceiling for any kill, whatever the weapon.
	float MaxKillDistance = 1200.0;
	int   DistanceFlagWeight = 18;

	//  Kills inside a short window - snap aimbots clear a squad in seconds.
	float RapidKillWindowS = 6.0;
	int   RapidKillCount = 4;
	int   RapidKillFlagWeight = 15;

	//  Silent aim: the shot connected with somebody the shooter was not facing.
	//  Half-angle in degrees, generous on purpose - this is meant to catch a
	//  hit landing behind the shooter, not to police tracking accuracy.
	float SilentAimConeDeg = 60.0;
	//  Below this range the geometry is too noisy to judge.
	float SilentAimMinDistance = 8.0;
	int   SilentAimFlagWeight = 15;

	// -- Unauthorized RPC ----------------------------------------------------
	//  A legitimate client never calls an RPC it has no permission for: its own
	//  UI does not offer the control. Repeated denials are a cheat menu probing
	//  the mod. Counted in a window rather than flagged one at a time, so a
	//  single denial from a permission changing mid-session is not evidence.
	int   RpcDeniedBurstCount = 3;
	float RpcDeniedWindowS = 30.0;
	int   RpcDeniedFlagWeight = 22;

	//  Staff are held to the same rules as everyone else, with one honest
	//  exception: a COT client shows an admin every control their ROLE grants,
	//  and clicking one whose permission was revoked mid-session is a denial
	//  that means nothing. Players with COT access get a higher burst before
	//  it counts - a different number, not a free pass.
	int   RpcDeniedBurstCountStaff = 10;

	// -- Inventory -----------------------------------------------------------
	//  Item count is sampled on its own slower cadence: walking the inventory
	//  of every player once a second is real server cost for a signal that does
	//  not move that fast.
	float InventorySampleIntervalS = 5.0;

	// -- Noise control -------------------------------------------------------
	//  Minimum seconds between two flags of the SAME detector for the SAME
	//  player. Without it a speedhacker scores every poll and the total says
	//  "how long were they online" rather than "how bad is this".
	float DetectorCooldownS = 20.0;

	// -- Decay ---------------------------------------------------------------
	//  Points bled off the score per hour of real time since the last event.
	//  Without this the score is cumulative for the lifetime of the server and
	//  eventually flags every regular, at which point it measures hours played.
	//
	//  4/hour means one uncertain 5-point speed reading is gone in about 75
	//  minutes, while a 25-point immunity flag still stands the next evening.
	float FlagDecayPerHour = 4.0;

	//  A flag that has decayed to zero and seen nothing since is dropped after
	//  this many days, so the file does not grow forever.
	int FlagForgetAfterDays = 14;

	// -- Persistence ---------------------------------------------------------
	//  Flags survive a restart. A cheater who reconnects after a crash should
	//  not start from a clean sheet, and an admin coming on shift should see
	//  what happened overnight.
	bool  PersistFlags = true;
	//  Seconds between writes. Writes are skipped entirely when nothing has
	//  changed, so this is a ceiling on how much a hard crash can lose.
	float FlagSaveIntervalS = 60.0;

	// Score thresholds
	int FlagThresholdCritical = 30;  // fires webhook with DANGER colour
	int FlagThresholdHigh     = 15;  // fires webhook with WARNING colour

	// Auto-actions
	bool AutoKickOnCritical = false;
	int  AutoActionCooldownS = 600; // don't auto-act on the same player within 10 min

	static const string DIR  = "$profile:CommunityOnlineTools\\AntiCheat\\";
	static const string FILE = "$profile:CommunityOnlineTools\\AntiCheat\\config.json";

	static JMAntiCheatConfig Load()
	{
		JMAntiCheatConfig cfg = new JMAntiCheatConfig();

		if ( !FileExist( FILE ) )
			return cfg;

		JMAntiCheatConfig onDisk = new JMAntiCheatConfig();
		JsonFileLoader<JMAntiCheatConfig>.JsonLoadFile( FILE, onDisk );
		return onDisk;
	}
}
