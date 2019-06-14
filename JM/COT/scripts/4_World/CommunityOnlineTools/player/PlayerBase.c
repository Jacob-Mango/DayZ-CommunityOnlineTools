
modded class PlayerBase
{
	ref AuthPlayer authenticatedPlayer;

	bool m_HasGodMode;
	bool m_IsInvisible;

	override void Init()
	{
		super.Init();

		RegisterNetSyncVariableBool( "m_IsInvisible" );
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		SetInvisible( m_IsInvisible );
	}

	void SetGodMode( bool mode )
	{
		if ( GetGame().IsServer() )
		{
			m_HasGodMode = mode;

			SetAllowDamage( m_HasGodMode );
		}
	}

	void SetInvisibility(bool mode)
	{
		if ( GetGame().IsServer() )
		{
			m_IsInvisible = mode

			SetSynchDirty();
		}
	}
}