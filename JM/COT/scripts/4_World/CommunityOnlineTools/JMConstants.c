class JMConstants
{
	static const string OFFLINE_GUID = "OFFLINE";
	static const string OFFLINE_STEAM = "OFFLINE";
	static const string OFFLINE_NAME = "OFFLINE";
	
	static const string PERM_ROOT = "ROOT";

	static const string DIR_COT = "$profile:CommunityOnlineTools\\"
    static const string DIR_PF = "$profile:PermissionsFramework\\";

    static const string DIR_LOGS = DIR_COT + "Logs\\";

	static const string DIR_PERMISSIONS = DIR_PF + "Permissions\\";
	static const string DIR_ROLES = DIR_PF + "Roles\\";
	static const string DIR_PLAYERS = DIR_PF + "Players\\";

	static const string FT_ROLE = "txt";
	static const string FT_PERMISSION = "txt";
	static const string FT_PLAYER = "json";
	static const string FT_WINDOWS_DEFAULT = "txt";
	static const string FT_LOG = "log";

	static const string EXT_ROLE = "." + FT_ROLE;
	static const string EXT_PERMISSION = "." + FT_PERMISSION;
	static const string EXT_PLAYER = "." + FT_PLAYER;
	static const string EXT_WINDOWS_DEFAULT = "." + FT_WINDOWS_DEFAULT;
	static const string EXT_LOG = "." + FT_LOG;

	static const string FILE_TELEPORT = DIR_COT + "Teleports.json";
	static const string FILE_WEATHER = DIR_COT + "Weather.json";

	static const string DIR_VEHICLES = DIR_COT + "Vehicles\\";
	static const string FT_VEHICLE = "json";
	static const string EXT_VEHICLE = "." + FT_VEHICLE;

	static const string DIR_ITEMS = DIR_COT + "Items\\";
	static const string FT_ITEM = "json";
	static const string EXT_ITEM = "." + FT_ITEM;
}