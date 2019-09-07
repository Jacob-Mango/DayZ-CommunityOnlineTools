
modded class PlayerBase
{
	private JMPlayerInstance m_AuthenticatedPlayer;

	private bool m_HasGodMode;

	private bool m_IsInvisible;
	private bool m_IsInvisibleRemoteSynch;

	private vector m_LastPosition;
	private bool m_HasLastPosition;

	override void Init()
	{
		super.Init();

		RegisterNetSyncVariableBool( "m_IsInvisibleRemoteSynch" );

		m_HasLastPosition = false;
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if ( m_IsInvisibleRemoteSynch != m_IsInvisible )
		{
			m_IsInvisibleRemoteSynch = m_IsInvisible;
			if ( m_IsInvisible )
			{
				ClearFlags( EntityFlags.VISIBLE, true );
			} else
			{
				SetFlags( EntityFlags.VISIBLE, true );
			}

			//SetInvisible( m_IsInvisible );
		}
	}

	bool HasLastPosition()
	{
		return m_HasLastPosition;
	}

	vector GetLastPosition()
	{
		return m_LastPosition;
	}

	void SetLastPosition( vector pos )
	{
		if ( GetGame().IsServer() )
		{
			m_LastPosition = pos;

			m_HasLastPosition = true;
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
		return m_HasGodMode;
	}

	bool IsInvisible()
	{
		return m_IsInvisible;
	}

	void SetGodMode( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_HasGodMode = mode;

			SetAllowDamage( !m_HasGodMode );
		}
	}

	void SetInvisibility(bool mode)
	{
		if ( GetGame().IsServer() )
		{
			m_IsInvisible = mode;
			m_IsInvisibleRemoteSynch = mode;

			SetSynchDirty();
		}
	}
}