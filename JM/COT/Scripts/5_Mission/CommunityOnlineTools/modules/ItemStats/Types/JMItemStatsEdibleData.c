class JMItemStatsEdibleData: JMItemStatsBaseData
{
    int m_QuantityMax;
    int m_LiquidTypeInit;
	ref JMItemStatsNutritionData m_NutritionData;

	void JMItemStatsEdibleData(string path, string name)
	{
		m_QuantityMax = g_Game.ConfigGetInt(path + " varQuantityMax");
		m_LiquidTypeInit = g_Game.ConfigGetInt(path + " varLiquidTypeInit");

		m_NutritionData = new JMItemStatsNutritionData(path + " Nutrition");
	}

	override string GetExportHeaderData()
	{
		string result = super.GetExportHeaderData();
		if (result != "")
			result += ", ";

		result += "QuantityMax";
		result += ", ";
		result += "LiquidTypeInit";
		result += ", ";
		result += m_NutritionData.GetExportHeaderData();

		return result;
	}

	override string GetExportData()
	{
		string result = super.GetExportData();
		if (result != "")
			result += ", ";

		result += m_QuantityMax.ToString();
		result += ", ";
		result += m_LiquidTypeInit.ToString();
		result += ", ";
		result += m_NutritionData.GetExportData();

		return result;
	}
}