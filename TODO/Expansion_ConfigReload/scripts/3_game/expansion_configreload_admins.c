class ExpansionConfigReloadAdminList
{
	string m_Version = "1";
	ref TStringArray Admins;

	void ExpansionConfigReloadAdminList()
	{
		Admins = new TStringArray;
	}
}

class ExpansionConfigReloadAdmin
{
	protected static const string CONFIG_FOLDER = "$profile:ExpansionConfigReload\\";
	protected static const string CONFIG_FILE = CONFIG_FOLDER + "Admins.json";

	protected static ref TStringArray s_Admins;
	protected static bool s_Loaded;

	static void EnsureInitialized()
	{
		Load();
	}

	static void Load()
	{
		s_Loaded = true;
		s_Admins = new TStringArray;

		EnsureFolders();

		if (!FileExist(CONFIG_FILE))
		{
			CreateDefaultFile();
			return;
		}

		ExpansionConfigReloadAdminList data = new ExpansionConfigReloadAdminList();
		bool loaded = ExpansionJsonFileParser<ExpansionConfigReloadAdminList>.Load(CONFIG_FILE, data);

		if (!loaded)
		{
			JsonFileLoader<ExpansionConfigReloadAdminList>.JsonLoadFile(CONFIG_FILE, data);
		}

		if (!data || !data.Admins)
		{
			Print("[Expansion_ConfigReload] Failed to load admin list: " + CONFIG_FILE);
			Print("[Expansion_ConfigReload] Expected format: {\"m_Version\":\"1\",\"Admins\":[\"76561198xxxxxxxx\"]}");
			return;
		}

		s_Admins = data.Admins;
		Print("[Expansion_ConfigReload] Loaded " + s_Admins.Count() + " admin(s) from " + CONFIG_FILE);

		if (s_Admins.Count() == 0)
		{
			Print("[Expansion_ConfigReload] WARNING: Admins.json has no Steam64 IDs. Add your Steam64 to Admins, then restart the server.");
		}
	}

	protected static void EnsureFolders()
	{
		if (FileExist(CONFIG_FOLDER))
		{
			return;
		}

		if (!ExpansionStatic.MakeDirectoryRecursive(CONFIG_FOLDER))
		{
			MakeDirectory(CONFIG_FOLDER);
		}

		if (!FileExist(CONFIG_FOLDER))
		{
			Print("[Expansion_ConfigReload] ERROR: Could not create folder: " + CONFIG_FOLDER);
		}
	}

	protected static void CreateDefaultFile()
	{
		ExpansionConfigReloadAdminList defaults = new ExpansionConfigReloadAdminList();
		JsonFileLoader<ExpansionConfigReloadAdminList>.JsonSaveFile(CONFIG_FILE, defaults);

		if (!FileExist(CONFIG_FILE))
		{
			ExpansionJsonFileParser<ExpansionConfigReloadAdminList>.Save(CONFIG_FILE, defaults);
		}

		Print("[Expansion_ConfigReload] Created default admin list at: " + CONFIG_FILE);
		Print("[Expansion_ConfigReload] Add your Steam64 ID(s) to the Admins array, then restart the server.");
	}

	static void Reload()
	{
		s_Loaded = false;
		Load();
	}

	static bool IsAdmin(string steamId)
	{
		if (!steamId || steamId == string.Empty)
		{
			return false;
		}

		if (!s_Loaded)
		{
			Load();
		}

		if (!s_Admins)
		{
			return false;
		}

		return s_Admins.Find(steamId) > -1;
	}
}
