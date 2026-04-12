class JMConstants
{
	static const string OFFLINE_GUID 	= "OFFLINE";
	static const string OFFLINE_STEAM 	= "OFFLINE";
	static const string OFFLINE_NAME 	= "OFFLINE";
	
	static const string PERM_ROOT = "ROOT";

	static const string DIR_COT = "$profile:CommunityOnlineTools\\";
	static const string DIR_PF = "$profile:PermissionsFramework\\";

	static const string DIR_PERMISSIONS = DIR_PF + "Permissions\\";
	static const string DIR_ROLES 		= DIR_PF + "Roles\\";
	static const string DIR_PLAYERS 	= DIR_PF + "Players\\";
	static const string DIR_BANS 		= DIR_PF + "Bans\\";
	static const string DIR_NOTES 		= DIR_PF + "Notes\\";

	static const string EXT_PERMISSION 	= ".txt";
	static const string EXT_ROLE 		= ".txt";
	static const string EXT_PLAYER 		= ".json";
	static const string EXT_BAN 		= ".json";
	static const string EXT_NOTE 		= ".json";
	static const string EXT_CSV 		= ".csv";
	static const string EXT_WINDOWS_DEFAULT = ".txt";

	static const string FILE_WEBHOOK 	= DIR_COT + "Webhooks.json";
	static const string FILE_WEATHER 	= DIR_COT + "Weather.json";
	static const string FILE_TELEPORT 	= DIR_COT + "Teleports_";
	static const string DIR_LOGS 		= DIR_COT + "Logs\\";
	static const string DIR_LOADOUTS 	= DIR_COT + "Loadouts\\";
	static const string DIR_DELETIONS 	= DIR_COT + "Deletions\\";
	static const string DIR_EXPORTS 	= DIR_COT + "Exports\\";

	static const string EXT_LOG 		= ".log";
	static const string EXT_TELEPORT 	= ".json";
	static const string EXT_LOADOUT 	= ".json";

	// RPC security limits
	static const int    RPC_MAX_GUIDS         = 64;   // max targets per multi-target RPC
	static const float  RPC_RATE_LIMIT_S      = 0.1;  // minimum seconds between RPCs per sender

	// Player stat clamp ranges  [min, max]
	static const float STAT_HEALTH_MIN        = 0.0;
	static const float STAT_HEALTH_MAX        = 100.0;
	static const float STAT_BLOOD_MIN         = 0.0;
	static const float STAT_BLOOD_MAX         = 5000.0;
	static const float STAT_SHOCK_MIN         = 0.0;
	static const float STAT_SHOCK_MAX         = 100.0;
	static const float STAT_ENERGY_MIN        = 0.0;
	static const float STAT_ENERGY_MAX        = 20000.0;
	static const float STAT_WATER_MIN         = 0.0;
	static const float STAT_WATER_MAX         = 5000.0;
	static const float STAT_STAMINA_MIN       = 0.0;
	static const float STAT_STAMINA_MAX       = 100.0;
	static const float STAT_HEATBUFFER_MIN    = 0.0;
	static const float STAT_HEATBUFFER_MAX    = 1.0;

	// Spawner config file
	static const string FILE_SPAWNER_CONFIG   = DIR_COT + "SpawnerConfig.json";

	// Legacy
	static const string DIR_VEHICLES = DIR_COT + "Vehicles\\";
	static const string FT_VEHICLE = "json";
	static const string EXT_VEHICLE = "." + FT_VEHICLE;

	static const string DIR_ITEMS = DIR_COT + "Items\\";
	static const string FT_ITEM = "json";
	static const string EXT_ITEM = "." + FT_ITEM;
	// Legacy

	// Discord embed color palette (decimal representation of hex colors)
	static const int WEBHOOK_COLOR_DEFAULT    = 16766720;  // 0xFFCC00 - amber/gold  (default, generic info)
	static const int WEBHOOK_COLOR_INFO       = 3447003;   // 0x3498DB - blue        (join, neutral info)
	static const int WEBHOOK_COLOR_SUCCESS    = 3066993;   // 0x2ECC71 - green       (server startup, positive)
	static const int WEBHOOK_COLOR_WARNING    = 16098851;  // 0xF5A623 - orange      (admin actions, modifications)
	static const int WEBHOOK_COLOR_DANGER     = 15158332;  // 0xE74C3C - red         (death, ban, damage)
	static const int WEBHOOK_COLOR_CRITICAL   = 10038562;  // 0x992D22 - dark red    (mass delete, critical admin ops)
	static const int WEBHOOK_COLOR_MODERATION = 10181046;  // 0x9B59B6 - purple      (kick, permissions, roles)
	static const int WEBHOOK_COLOR_NEUTRAL    = 9807270;   // 0x95A5A6 - grey        (leave, shutdown)
	static const int WEBHOOK_COLOR_SPAWN      = 1752220;   // 0x1ABC9C - teal        (object/loadout spawn)
	static const int WEBHOOK_COLOR_TELEPORT   = 3553599;   // 0x3643FF - indigo      (teleport actions)
	static const int WEBHOOK_COLOR_ESP        = 8311585;   // 0x7ED321 - lime green  (ESP actions on objects)
	static const int WEBHOOK_COLOR_CAMERA     = 8900331;   // 0x87CEEB - sky blue    (camera actions)
}
