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
		PlayerIdentity identity;
		PlayerBase player;

        switch(eventTypeId)
        {
            case ClientNewEventTypeID:
            {
				ClientNewEventParams newParams;
				
				Class.CastTo(newParams, params);
				Class.CastTo(identity, newParams.param1);

                if (IsCOTBanned(identity))
                {
					OnClientDisconnectedEvent(identity, NULL, 0, true);
					return;
                }
                break;
            }
        }

        super.OnEvent(eventTypeId, params);
    }

	protected bool IsCOTBanned(PlayerIdentity identity)
	{
		string guid = identity.GetId();
		string steamid = identity.GetPlainId();
		JMPlayerBan banData = JMPlayerBan.Load(guid, steamid);
		if ( banData )
		{
			int banDuration = banData.BanDuration;
			if (banDuration > 0)
			{
				CF_Date nowUTC = CF_Date.Now(true);
				int currentTime = nowUTC.GetTimestamp();
				if ( currentTime > banDuration )
				{
					Print("[COT] Player "+ guid +" Ban Duration Expired - Letting player Join. Reason(BAN): "+ banData.Message);
					JMPlayerBan.DeleteBanFile(guid, steamid);
					return false;
				}
			}

			Print("[COT] Player "+ guid +" was kicked. Reason(BAN): "+ banData.Message);
			return true;
		}

		return false;
	}
};
