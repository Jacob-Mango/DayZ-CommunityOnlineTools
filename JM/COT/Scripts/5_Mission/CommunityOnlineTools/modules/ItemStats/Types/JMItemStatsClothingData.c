class JMItemStatsClothingData: JMItemStatsBaseData
{
	float m_VisibilityModifier;
	float m_HeatIsolation;
	ref JMItemStatsArmorData m_ArmorData;
	ref JMItemStatsProtectionData m_ProtectionData;

	void JMItemStatsClothingData(string path, string name)
	{
		m_VisibilityModifier = g_Game.ConfigGetFloat(path + " visibilityModifier");
		m_HeatIsolation = g_Game.ConfigGetFloat(path + " heatIsolation");
		m_ArmorData = new JMItemStatsArmorData(path + " DamageSystem GlobalArmor");
		m_ProtectionData = new JMItemStatsProtectionData(path + " Protection");
	}

	override string GetExportHeaderData()
	{
		string result = super.GetExportHeaderData();
		if (result != "")
			result += ", ";

		result += "VisibilityModifier";
		result += ", ";
		result += "HeatIsolation";
		result += ", ";
		result += m_ArmorData.GetExportHeaderData();
		result += ", ";
		result += m_ProtectionData.GetExportHeaderData();

		return result;
	}

	override string GetExportData()
	{
		string result = super.GetExportData();
		if (result != "")
			result += ", ";

		result += m_VisibilityModifier.ToString();
		result += ", ";
		result += m_HeatIsolation.ToString();
		result += ", ";
		result += m_ArmorData.GetExportData();
		result += ", ";
		result += m_ProtectionData.GetExportData();

		return result;
	}
};