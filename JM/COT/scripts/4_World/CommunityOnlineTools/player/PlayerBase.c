
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

		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DeferredInit );
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

	void DeferredInit()
	{
		if ( !IsMissionHost() )
			return;

		if ( GetIdentity() )
		{
			m_AuthenticatedPlayer = GetPermissionsManager().GetPlayer( GetIdentity().GetId() );
			if ( m_AuthenticatedPlayer )
			{
				m_AuthenticatedPlayer.PlayerObject = this;
			}
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

	void SetLastPosition()
	{
		if ( GetGame().IsServer() )
		{
			vector trans[4];
			GetTransform( trans );
			m_JMLastPosition = trans[3];

			m_JMHasLastPosition = true;
		}
	}

	void SetWorldPosition( vector position )
	{
		Object parent;
		if ( Class.CastTo( parent, GetParent() ) )
		{
			/*
			vector tmPlayer[4];
			vector tmTarget[4];
			vector tmLocal[4];

			GetTransformWS( tmPlayer );
			tmPlayer[3] = position;

			parent.GetTransform( tmTarget );
			Math3D.MatrixInvMultiply4( tmTarget, tmPlayer, tmLocal );

			SetPosition( tmLocal[3] );
			SetDirection( tmLocal[2] );
			*/

			SetPosition( parent.WorldToModel( position ) );
		} else
		{
			SetPosition( position );
		}
	}

	ref JMPlayerInstance GetAuthenticatedPlayer()
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