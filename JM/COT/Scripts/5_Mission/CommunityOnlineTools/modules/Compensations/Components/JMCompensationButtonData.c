class JMCompensationButtonData: UIActionData
{
	string m_SteamID;
	string m_Timestamp;

	void JMCompensationButtonData( string steamID, string timestamp )
	{
		m_SteamID = steamID;
		m_Timestamp = timestamp;
	}
}
