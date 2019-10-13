
modded class PlayerBase
{
	private JMPlayerInstance m_AuthenticatedPlayer;

	private bool m_JMHasGodMode;

	private bool m_JMIsInvisible;
	private bool m_JMIsInvisibleRemoteSynch;

	private vector m_JMLastPosition;
	private bool m_JMHasLastPosition;

	override void Init()
	{
		super.Init();

		RegisterNetSyncVariableBool( "m_JMIsInvisibleRemoteSynch" );

		m_JMHasLastPosition = false;
		m_JMLastPosition = "0 0 0";

		if ( GetGame() && IsMissionOffline() )
		{
			if ( GetGame().GetPlayer() == NULL && g_Game.GetGameState() != DayZGameState.MAIN_MENU )
			{
				JMPlayerInstance instance;
				GetPermissionsManager().OnClientConnected( NULL, instance );
			}
		}
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if ( m_JMIsInvisibleRemoteSynch != m_JMIsInvisible )
		{
			m_JMIsInvisibleRemoteSynch = m_JMIsInvisible;
			if ( m_JMIsInvisible )
			{
				ClearFlags( EntityFlags.VISIBLE, true );
			} else
			{
				SetFlags( EntityFlags.VISIBLE, true );
			}

			//SetInvisible( m_JMIsInvisible );
		}
	}

	bool HasLastPosition()
	{
		return m_JMHasLastPosition;
	}

	vector GetLastPosition()
	{
		return m_JMLastPosition;
	}

	void SetLastPosition( vector pos )
	{
		if ( GetGame().IsServer() )
		{
			m_JMLastPosition = pos;

			m_JMHasLastPosition = true;
		}
	}

	void SetAuthenticatedPlayer( ref JMPlayerInstance player )
	{
		m_AuthenticatedPlayer = player;
	}

	JMPlayerInstance GetAuthenticatedPlayer()
	{
		return m_AuthenticatedPlayer;
	}

	bool HasGodMode()
	{
		return m_JMHasGodMode;
	}

	bool IsInvisible()
	{
		return m_JMIsInvisible;
	}

	void SetGodMode( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasGodMode = mode;

			SetAllowDamage( !m_JMHasGodMode );
		}
	}

	void SetInvisibility(bool mode)
	{
		if ( GetGame().IsServer() )
		{
			m_JMIsInvisible = mode;
			m_JMIsInvisibleRemoteSynch = mode;

			SetSynchDirty();
		}
	}
}