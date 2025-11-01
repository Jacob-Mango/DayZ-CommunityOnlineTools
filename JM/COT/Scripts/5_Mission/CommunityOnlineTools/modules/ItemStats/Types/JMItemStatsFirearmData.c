class JMItemStatsFirearmData: JMItemStatsBaseData
{
    int m_ChamberSize;
    float m_WeaponLength;
    float m_ObstructionDistance;
    float m_InitSpeedMultiplier;
    float m_BarrelArmor;
    vector m_RecoilModifier;
    vector m_SwayModifier;
    float m_ShoulderDistance;

    ref TStringArray m_Magazines;
    ref TStringArray m_ChamberableFrom;
    ref TStringArray m_Modes;

    ref JMItemStatsFireModeData m_FireSemi;
    ref JMItemStatsFireModeData m_FireBurst;
    ref JMItemStatsFireModeData m_FireAuto;
    ref JMItemStatsNoiseShootData m_NoiseShoot;
    ref JMItemStatsOpticsInfoData m_OpticsInfo;

	void JMItemStatsFirearmData(string path, string name)
	{
        m_ChamberSize = g_Game.ConfigGetInt(path + " chamberSize");
        m_WeaponLength = g_Game.ConfigGetFloat(path + " WeaponLength");
        m_ObstructionDistance = g_Game.ConfigGetFloat(path + " obstructionDistance");
        m_InitSpeedMultiplier = g_Game.ConfigGetFloat(path + " initSpeedMultiplier");
        m_BarrelArmor = g_Game.ConfigGetFloat(path + " barrelArmor");
        m_RecoilModifier = g_Game.ConfigGetVector(path + " recoilModifier");
        m_SwayModifier = g_Game.ConfigGetVector(path + " swayModifier");
        m_ShoulderDistance = g_Game.ConfigGetFloat(path + " ShoulderDistance");
        
		m_Magazines = new TStringArray;
		g_Game.ConfigGetTextArray(path + " magazines", m_Magazines);

		m_ChamberableFrom = new TStringArray;
		g_Game.ConfigGetTextArray(path + " chamberableFrom", m_ChamberableFrom);

		m_Modes = new TStringArray;
		g_Game.ConfigGetTextArray(path + " modes", m_Modes);

        m_FireSemi = new JMItemStatsFireModeData(path + " SemiAuto");
        m_FireBurst = new JMItemStatsFireModeData(path + " Burst");
        m_FireAuto = new JMItemStatsFireModeData(path + " FullAuto");
        m_NoiseShoot = new JMItemStatsNoiseShootData(path + " NoiseShoot");
		m_OpticsInfo = new JMItemStatsOpticsInfoData(path + " OpticsInfo");
	}

	override string GetExportHeaderData()
	{
		string result = super.GetExportHeaderData();
		if (result != "")
			result += ", ";

		result += "m_ChamberSize";
		result += ", ";
		result += "m_WeaponLength";
		result += ", ";
		result += "m_ObstructionDistance";
		result += ", ";
		result += "m_InitSpeedMultiplier";
		result += ", ";
		result += "m_BarrelArmor";
		result += ", ";
		result += "m_RecoilModifier";
		result += ", ";
		result += "m_SwayModifier";
		result += ", ";
		result += "m_ShoulderDistance";
		result += ", ";
		result += "m_Magazines";
		result += ", ";
		result += "m_ChamberableFrom";
		result += ", ";
		result += "m_Modes";
		result += ", ";
		result += m_FireSemi.GetExportHeaderData("Semi");
		result += ", ";
		result += m_FireBurst.GetExportHeaderData("Burst");
		result += ", ";
		result += m_FireAuto.GetExportHeaderData("FullAuto");
		result += ", ";
		result += m_NoiseShoot.GetExportHeaderData();
		result += ", ";
		result += m_OpticsInfo.GetExportHeaderData();

		return result;
	}

	override string GetExportData()
	{
		string result = super.GetExportData();
		if (result != "")
			result += ", ";

		result += m_ChamberSize.ToString();
		result += ", ";
		result += m_WeaponLength.ToString();
		result += ", ";
		result += m_ObstructionDistance.ToString();
		result += ", ";
		result += m_InitSpeedMultiplier.ToString();
		result += ", ";
		result += m_BarrelArmor.ToString();
		result += ", ";
		result += m_RecoilModifier.ToString(false);
		result += ", ";
		result += m_SwayModifier.ToString(false);
		result += ", ";
		result += m_ShoulderDistance.ToString();
		result += ", ";
		string mag;
		foreach(string selmag: m_Magazines)
		{
			mag += selmag + " ";
		}
		result += mag;
		result += ", ";
		string chamber;
		foreach(string chamgerfrom: m_ChamberableFrom)
		{
			chamber += chamgerfrom + " ";
		}
		result += chamber;
		result += ", ";
		string mode;
		foreach(string selmode: m_Modes)
		{
			mode += selmode + " ";
		}
		result += mode;
		result += ", ";
		result += m_FireSemi.GetExportData();
		result += ", ";
		result += m_FireBurst.GetExportData();
		result += ", ";
		result += m_FireAuto.GetExportData();
		result += ", ";
		result += m_NoiseShoot.GetExportData();
		result += ", ";
		result += m_OpticsInfo.GetExportData();

		return result;
	}
};

class JMItemStatsFireModeData
{
    float m_ReloadTime;
    float m_Dispersion;
    int m_Burst;

	void JMItemStatsFireModeData(string path)
	{
		m_ReloadTime = g_Game.ConfigGetFloat(path + " reloadTime");
		m_Dispersion = g_Game.ConfigGetFloat(path + " dispersion");
		m_Burst = g_Game.ConfigGetInt(path + " burst");
	}

	string GetExportHeaderData(string name)
	{
		string result;

		result += name +" ReloadTime";
        result += ", ";
		result += name +" Dispersion";
        result += ", ";
		result += name +" Burst";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_ReloadTime.ToString();
        result += ", ";
		result += m_Dispersion.ToString();
        result += ", ";
		result += m_Burst.ToString();

		return result;
	}
};

class JMItemStatsOpticsInfoData
{
    int m_DistanceZoomMin;
    int m_DistanceZoomMax;
    ref TIntArray m_DiscreteDistance;

	void JMItemStatsOpticsInfoData(string path)
	{
		m_DistanceZoomMin = g_Game.ConfigGetInt(path + " distanceZoomMin");
		m_DistanceZoomMax = g_Game.ConfigGetInt(path + " distanceZoomMax");
        
		m_DiscreteDistance = new TIntArray;
		g_Game.ConfigGetIntArray(path + " discreteDistance", m_DiscreteDistance);
	}

	string GetExportHeaderData()
	{
		string result;

		result += "OpticsInfo DistanceZoomMin";
        result += ", ";
		result += "OpticsInfo DistanceZoomMax";
        result += ", ";
		result += "OpticsInfo DiscreteDistance";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_DistanceZoomMin.ToString();
        result += ", ";
		result += m_DistanceZoomMax.ToString();
        result += ", ";
		string mode;
		foreach(int dist: m_DiscreteDistance)
		{
			mode += dist.ToString() + " ";
		}
		result += mode;

		return result;
	}
};

class JMItemStatsNoiseShootData
{
    int m_Strength;

	void JMItemStatsNoiseShootData(string path)
	{
		m_Strength = g_Game.ConfigGetInt(path + " strength");
	}

	string GetExportHeaderData()
	{
		string result;

		result += "NoiseShoot Strength";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Strength.ToString();

		return result;
	}
};