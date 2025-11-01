class JMItemStatsOpticData: JMItemStatsBaseData
{
    vector m_RecoilModifier;
    vector m_SwayModifier;

    ref JMItemStatsOpticsInfoData m_OpticsInfo;

	void JMItemStatsOpticData(string path, string name)
	{
        m_RecoilModifier = g_Game.ConfigGetVector(path + " recoilModifier");
        m_SwayModifier = g_Game.ConfigGetVector(path + " swayModifier");

		m_OpticsInfo = new JMItemStatsOpticsInfoData(path + " OpticsInfo");
	}

	override string GetExportHeaderData()
	{
		string result = super.GetExportHeaderData();
		if (result != "")
			result += ", ";

		result += "m_RecoilModifier";
		result += ", ";
		result += "m_SwayModifier";
		result += ", ";
		result += m_OpticsInfo.GetExportHeaderData();

		return result;
	}

	override string GetExportData()
	{
		string result = super.GetExportData();
		if (result != "")
			result += ", ";

		result += m_RecoilModifier.ToString(false);
		result += ", ";
		result += m_SwayModifier.ToString(false);
		result += ", ";
		result += m_OpticsInfo.GetExportData();

		return result;
	}
};