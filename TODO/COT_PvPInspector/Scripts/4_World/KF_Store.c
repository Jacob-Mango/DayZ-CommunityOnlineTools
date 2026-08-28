// Stockage serveur des kills : liste en memoire + persistance JSON dans le profile.
// Cote 4_World pour etre accessible depuis le modded PlayerBase (capture).
// Le module 5_Mission lit s_Kills pour l'affichage.
class KF_Store
{
	static ref array<ref KF_KillRecord> s_Kills = new array<ref KF_KillRecord>;
	static int  s_NextId = 1;
	static bool s_Loaded = false;

	static const int    MAX_KILLS = 500;
	static const string DIR_COT   = "$profile:CommunityOnlineTools";
	static const string DIR_KF    = "$profile:CommunityOnlineTools\\KillFeed";
	static const string FILE_KF   = "$profile:CommunityOnlineTools\\KillFeed\\killfeed.json";

	static void Load()
	{
		if (s_Loaded)
			return;
		s_Loaded = true;

		if (FileExist(FILE_KF))
		{
			KF_SaveData data = new KF_SaveData();
			JsonFileLoader<KF_SaveData>.JsonLoadFile(FILE_KF, data);
			if (data && data.Kills)
			{
				s_Kills = data.Kills;
				s_NextId = data.NextId;
				if (s_NextId < 1)
					s_NextId = 1;
			}
		}
	}

	static void Save()
	{
		if (!FileExist(DIR_COT))
			MakeDirectory(DIR_COT);
		if (!FileExist(DIR_KF))
			MakeDirectory(DIR_KF);

		KF_SaveData data = new KF_SaveData();
		data.NextId = s_NextId;
		data.Kills = s_Kills;
		JsonFileLoader<KF_SaveData>.JsonSaveFile(FILE_KF, data);
	}

	static void Add(KF_KillRecord rec)
	{
		Load();

		rec.Id = s_NextId;
		s_NextId++;

		// Plus recent en tete de liste.
		s_Kills.InsertAt(rec, 0);

		while (s_Kills.Count() > MAX_KILLS)
			s_Kills.Remove(s_Kills.Count() - 1);

		Save();
	}

	static void Clear()
	{
		Load();
		s_Kills.Clear();
		Save();
	}
}
