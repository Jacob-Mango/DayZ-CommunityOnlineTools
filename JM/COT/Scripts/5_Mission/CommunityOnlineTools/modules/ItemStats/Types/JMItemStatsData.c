class JMItemStatsArmorData
{
	ref JMItemStatsHealthData m_Projectile;
	ref JMItemStatsHealthData m_Melee;
	ref JMItemStatsHealthData m_Infected;
	ref JMItemStatsHealthData m_FragGrenade;

	void JMItemStatsArmorData(string path)
	{
		m_Projectile = new JMItemStatsHealthData(path + " Projectile");
		m_Melee = new JMItemStatsHealthData(path + " Melee");
		m_Infected = new JMItemStatsHealthData(path + " Infected");
		m_FragGrenade = new JMItemStatsHealthData(path + " FragGrenade");
	}

	string GetExportHeaderData()
	{
		string result;

		result += m_Projectile.GetExportHeaderData("Projectile");
		result += ", ";
		result += m_Melee.GetExportHeaderData("Melee");
		result += ", ";
		result += m_Infected.GetExportHeaderData("Infected");
		result += ", ";
		result += m_FragGrenade.GetExportHeaderData("FragGrenade");

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Projectile.GetExportData();
		result += ", ";
		result += m_Melee.GetExportData();
		result += ", ";
		result += m_Infected.GetExportData();
		result += ", ";
		result += m_FragGrenade.GetExportData();

		return result;
	}
};

// NBC
class JMItemStatsProtectionData
{
	float m_Biological;
	float m_Chemical;

	void JMItemStatsProtectionData(string path)
	{
		m_Biological = g_Game.ConfigGetFloat(path + " biological");
		m_Chemical = g_Game.ConfigGetFloat(path + " chemical");
	}

	string GetExportHeaderData()
	{
		string result;

		result += "Protection Biological";
		result += ", ";
		result += "Protection Chemical";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Biological.ToString();
		result += ", ";
		result += m_Chemical.ToString();

		return result;
	}
};

class JMItemStatsHealthData
{
	float m_Health;
	float m_Blood;
	float m_Shock;

	void JMItemStatsHealthData(string path)
	{
		m_Health = g_Game.ConfigGetFloat(path + " Health damage");
		m_Blood = g_Game.ConfigGetFloat(path + " Blood damage");
		m_Shock = g_Game.ConfigGetFloat(path + " Shock damage");
	}

	string GetExportHeaderData(string name)
	{
		string result;

		result += "GlobalArmor " + name + " Health";
		result += ", ";
		result += "GlobalArmor " + name + " Blood";
		result += ", ";
		result += "GlobalArmor " + name + " Shock";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Health.ToString();
		result += ", ";
		result += m_Blood.ToString();
		result += ", ";
		result += m_Shock.ToString();

		return result;
	}
};

class JMItemStatsNutritionData
{
    float m_FullnessIndex;
    float m_Energy;
    float m_Water;
    float m_NutritionalIndex;
    float m_Toxicity;

	void JMItemStatsNutritionData(string path)
	{
		m_FullnessIndex = g_Game.ConfigGetFloat(path + " fullnessIndex");
		m_Energy = g_Game.ConfigGetFloat(path + " energy");
		m_Water = g_Game.ConfigGetFloat(path + " water");
		m_NutritionalIndex = g_Game.ConfigGetFloat(path + " nutritionalIndex");
		m_Toxicity = g_Game.ConfigGetFloat(path + " toxicity");
	}

	string GetExportHeaderData()
	{
		string result;

		result += "Nutrition FullnessIndex";
		result += ", ";
		result += "Nutrition Energy";
		result += ", ";
		result += "Nutrition Water";
		result += ", ";
		result += "Nutrition NutritionalIndex";
		result += ", ";
		result += "Nutrition Toxicity";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_FullnessIndex.ToString();
		result += ", ";
		result += m_Energy.ToString();
		result += ", ";
		result += m_Water.ToString();
		result += ", ";
		result += m_NutritionalIndex.ToString();
		result += ", ";
		result += m_Toxicity.ToString();

		return result;
	}
};