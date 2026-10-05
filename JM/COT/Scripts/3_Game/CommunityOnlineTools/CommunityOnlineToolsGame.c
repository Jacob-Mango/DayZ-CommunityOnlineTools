class CommunityOnlineToolsGame
{
	protected bool m_IsActive;
	protected bool m_IsOpen;
	protected string m_FileLogName;
	protected ref map<string, bool> m_ActiveGUIDs = new map<string, bool>;

	bool IsActive()
	{
		return m_IsActive;
	}

	bool IsActive(Man player)
	{
		return IsActive(player.GetIdentity());
	}

	bool IsActive(PlayerIdentity identity)
	{
		return IsActive(identity.GetId());
	}

	bool IsActive(string guid)
	{
	#ifdef SERVER
		return m_ActiveGUIDs[guid];
	#else
		return m_IsActive;
	#endif
	}

	bool IsOpen()
	{
		return m_IsOpen;
	}

	bool ActiveCount()
	{
		return m_ActiveGUIDs.Count();
	}

	map<string, bool> GetActive()
	{
		return m_ActiveGUIDs;
	}

	TStringArray GetActiveGUIDs()
	{
		TStringArray activeGUIDs = {};

		foreach (string guid, bool active: m_ActiveGUIDs)
			activeGUIDs.Insert(guid);

		return activeGUIDs;
	}

	void CreateNewLog()
	{
		if ( !FileExist( JMConstants.DIR_LOGS ) )
			MakeDirectory( JMConstants.DIR_LOGS );

		m_FileLogName = JMConstants.DIR_LOGS + "cot-" + JMDate.Now().ToString( "YYYY-MM-DD-hh-mm-ss" ) + JMConstants.EXT_LOG;
		int fileLog = OpenFile( m_FileLogName, FileMode.WRITE );

		if ( fileLog != 0 )
			CloseFile( fileLog );
	}

	void CloseLog()
	{
		m_FileLogName = "";
	}

	void LogServer( string text )
	{
		if ( g_Game.IsServer() )
		{
			g_Game.AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now().ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( PlayerIdentity logIdentPlyer, string text )
	{
		if ( g_Game.IsMultiplayer() && logIdentPlyer )
		{
			text = "" + logIdentPlyer.GetPlainId() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( g_Game.IsServer() )
		{
			g_Game.AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + JMDate.Now().ToString( "YYYY-MM-DD hh:mm:ss" ) + "] " + text );
			CloseFile( fileLog );
		}
	}
}


//! @note deliberately not ref! When g_cotBase (instantiated in 5_Misson) get de-ref'd, this must become NULL
static CommunityOnlineToolsGame g_cotGame;

static CommunityOnlineToolsGame GetCOTGame()
{
	return g_cotGame;
}
