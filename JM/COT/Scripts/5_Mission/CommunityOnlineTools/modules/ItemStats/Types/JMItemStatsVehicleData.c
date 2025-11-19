class JMItemStatsVehicleData: JMItemStatsBaseData
{
    int fuelCapacity;
    int fuelConsumption;
}

/*
class JMItemStatsCargoData
{
    ref TIntArray m_ItemsCargoSize;

	void JMItemStatsCargoData(string path)
	{
		m_ItemsCargoSize = g_Game.ConfigGetInt(path + " itemsCargoSize");
	}

	string GetExportHeaderData(string name)
	{
		string result;

		result += name +" itemsCargoSize";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Burst.ToString();

		return result;
	}
}


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
}

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
}
*/
