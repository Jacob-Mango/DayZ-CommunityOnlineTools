class JMItemStatsMagazineData: JMItemStatsBaseData
{
    int m_Count;
    string m_Ammo;
    ref TStringArray m_AmmoItems;
    int m_TracersEvery;

	void JMItemStatsMagazineData(string path, string name)
	{
        m_Count = g_Game.ConfigGetInt(path + " count");
		g_Game.ConfigGetText(path + " ammo", m_Ammo);

		m_AmmoItems = new TStringArray;
		g_Game.ConfigGetTextArray(path + " ammoItems", m_AmmoItems);

        m_TracersEvery = g_Game.ConfigGetInt(path + " tracersEvery");
	}

	override string GetExportHeaderData()
	{
		string result = super.GetExportHeaderData();
		if (result != "")
			result += ", ";

		result += "m_Count";
		result += ", ";
		result += "m_Ammo";
		result += ", ";
		result += "m_AmmoItems";
		result += ", ";
		result += "m_TracersEvery";
		result += ", ";

		return result;
	}

	override string GetExportData()
	{
		string result = super.GetExportData();
		if (result != "")
			result += ", ";

		result += m_Count.ToString();
		result += ", ";
		result += m_Ammo;
		result += ", ";
		string mag;
		foreach(string selmag: m_AmmoItems)
		{
			mag += selmag + " ";
		}
		result += mag;
		result += ", ";
		result += m_TracersEvery.ToString();

		return result;
	}
};