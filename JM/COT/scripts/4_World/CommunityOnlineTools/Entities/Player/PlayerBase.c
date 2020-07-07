
modded class PlayerBase
{
	private JMPlayerInstance m_AuthenticatedPlayer;

	private bool m_JMHasGodMode;

	private bool m_JMIsInvisible;
	private bool m_JMIsInvisibleRemoteSynch;

	private bool m_JMIsFrozen;
	private bool m_JMIsFrozenRemoteSynch;

	private vector m_JMLastPosition;
	private bool m_JMHasLastPosition;

	private bool m_JMHasUnlimitedAmmo;

	override void Init()
	{
		/*
		Print( "TOUCH: " + EntityEvent.TOUCH );
		Print( "VISIBLE: " + EntityEvent.VISIBLE );
		Print( "NOTVISIBLE: " + EntityEvent.NOTVISIBLE );
		Print( "FRAME: " + EntityEvent.FRAME );
		Print( "POSTFRAME: " + EntityEvent.POSTFRAME );
		Print( "INIT: " + EntityEvent.INIT );
		Print( "JOINTBREAK: " + EntityEvent.JOINTBREAK );
		Print( "SIMULATE: " + EntityEvent.SIMULATE );
		Print( "POSTSIMULATE: " + EntityEvent.POSTSIMULATE );
		Print( "PHYSICSMOVE: " + EntityEvent.PHYSICSMOVE );
		Print( "CONTACT: " + EntityEvent.CONTACT );
		Print( "EXTRA: " + EntityEvent.EXTRA );
		Print( "ANIMEVENT: " + EntityEvent.ANIMEVENT );
		Print( "SOUNDEVENT: " + EntityEvent.SOUNDEVENT );
		Print( "PHYSICSSTEADY: " + EntityEvent.PHYSICSSTEADY );
		Print( "USER: " + EntityEvent.USER );
		Print( "ALL: " + EntityEvent.ALL );
		*/
		super.Init();

		RegisterNetSyncVariableBool( "m_JMIsInvisibleRemoteSynch" );
		RegisterNetSyncVariableBool( "m_JMIsFrozenRemoteSynch" );

		GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( GetAuthenticatedPlayer, 2000, false );

		m_JMHasLastPosition = false;
		m_JMLastPosition = "0 0 0";
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

		if ( m_JMIsFrozenRemoteSynch != m_JMIsFrozen )
		{
			m_JMIsFrozen = m_JMIsFrozenRemoteSynch;

			HumanInputController hic = GetInputController();
			if ( hic )
				hic.SetDisabled( m_JMIsFrozen );
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
			SetPosition( parent.WorldToModel( position ) );
		} else
		{
			SetPosition( position );
		}
	}

	JMPlayerInstance GetAuthenticatedPlayer()
	{
		if ( m_AuthenticatedPlayer )
			return m_AuthenticatedPlayer;

		if ( !GetIdentity() ) // Could be AI
			return NULL;

		if ( !m_AuthenticatedPlayer )
		{
			if ( IsMissionOffline() )
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetClientPlayer();
			} else
			{
				m_AuthenticatedPlayer = GetPermissionsManager().GetPlayer( GetIdentity().GetId() );
			}
		}

		if ( Assert_Null( m_AuthenticatedPlayer ) )
			return NULL;
		
		m_AuthenticatedPlayer.PlayerObject = this;
		return m_AuthenticatedPlayer;
	}
	
	string FormatSteamWebhook()
	{
		return GetAuthenticatedPlayer().FormatSteamWebhook();
	}

	bool HasGodMode()
	{
		return m_JMHasGodMode;
	}

	bool IsFrozen()
	{
		return m_JMIsFrozen;
	}

	bool IsInvisible()
	{
		return m_JMIsInvisible;
	}

	bool HasUnlimitedAmmo()
	{
		return m_JMHasUnlimitedAmmo;
	}

	void SetGodMode( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasGodMode = mode;

			SetAllowDamage( !m_JMHasGodMode );
		}
	}

	void SetFreeze( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMIsFrozen = mode;
			m_JMIsFrozenRemoteSynch = mode;

			SetSynchDirty();

			HumanInputController hic = GetInputController();
			if ( hic )
				hic.SetDisabled( m_JMIsFrozen );
		}
	}

	void SetInvisibility( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMIsInvisible = mode;
			m_JMIsInvisibleRemoteSynch = mode;

			SetSynchDirty();
		}
	}

	void SetUnlimitedAmmo( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_JMHasUnlimitedAmmo = mode;
		}
	}
}