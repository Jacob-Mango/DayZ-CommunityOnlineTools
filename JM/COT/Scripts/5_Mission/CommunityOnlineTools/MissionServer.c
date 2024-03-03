modded class MissionServer
{
	JMPlayerModule m_JM_PlayerModule;
	float m_JM_UpdateTime;

	void MissionServer()
	{
		m_bLoaded = false;
		
		if ( !g_cotBase )
		{
			g_cotBase = new CommunityOnlineTools;
		}

		CF_Modules<JMPlayerModule>.Get(m_JM_PlayerModule);
	}

	void ~MissionServer()
	{
		delete g_cotBase;
	}

	override void OnInit()
	{
		super.OnInit();
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		g_cotBase.OnStart();
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		GetPermissionsManager().SetMissionLoaded();
	}

	override void OnMissionFinish()
	{
		g_cotBase.OnFinish();

		super.OnMissionFinish();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		g_cotBase.OnUpdate( timeslice );

		m_JM_UpdateTime += timeslice;

		if ( m_JM_UpdateTime > 1.0 )
		{
			m_JM_UpdateTime = 0.0;
			m_JM_PlayerModule.UpdateSpectatorPositions();
		}
	}

	override bool ShouldPlayerBeKilled(PlayerBase player)
	{
		if (!super.ShouldPlayerBeKilled(player))
			return false;

		if (player.COTIsBeingKicked())
			return false;

		return true;
	}

    override void OnEvent(EventType eventTypeId, Param params)
    {
        super.OnEvent(eventTypeId, params);

        switch(eventTypeId)
        {
            case ClientPrepareEventTypeID:
            {
                ClientPrepareEventParams clientPrepareParams;
                Class.CastTo(clientPrepareParams, params);

                PlayerIdentity identity = clientPrepareParams.param1;

				//! TODO: Make a file or something to store the bans and try to find the banned player with his Identity
				string banReason;
                //if (IsCOTBanned(identity.GetId(), banReason))
                {
                    //! is faulty, yeet them out
                }
                break;
            }
        }
    }
};

