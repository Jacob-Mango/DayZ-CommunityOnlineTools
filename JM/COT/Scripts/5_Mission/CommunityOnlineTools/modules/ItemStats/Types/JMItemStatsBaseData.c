class JMItemStatsBaseData
{
	string m_Classname;
	string m_DisplayName;

	vector m_ItemSize;
	vector m_ItemsCargoSize;

	ref TStringArray m_Attachments;

	int m_Health;

	void JMItemStatsBaseData(string path, string name)
	{
		m_Classname = name;
		g_Game.ConfigGetText(path + " displayName", m_DisplayName);
		m_DisplayName = Widget.TranslateString( m_DisplayName );

		m_Health = g_Game.ConfigGetInt(path + " DamageSystem GlobalHealth Health hitpoints");
		
		m_ItemSize = g_Game.ConfigGetVector(path + " itemSize");
		m_ItemsCargoSize = g_Game.ConfigGetVector(path + " itemsCargoSize");
		
		m_Attachments = new TStringArray;
		g_Game.ConfigGetTextArray(path + " attachments", m_Attachments);
	}

	string GetExportHeaderData()
	{
		string result;

		result += "Classname";
		result += ", ";
		result += "DisplayName";
		result += ", ";
		result += "ItemSize";
		result += ", ";
		result += "ItemsCargoSize";
		result += ", ";
		result += "Attachments";
		result += ", ";
		result += "Health";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Classname;
		result += ", ";
		result += m_DisplayName;
		result += ", ";
		result += m_ItemSize.ToString(false);
		result += ", ";
		result += m_ItemsCargoSize.ToString(false);
		result += ", ";
		string att;
		foreach(string attItem: m_Attachments)
		{
			att += attItem + " ";
		}
		result += att;
		result += ", ";
		result += m_Health.ToString();

		return result;
	}
}