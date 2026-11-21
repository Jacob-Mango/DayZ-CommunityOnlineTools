class JMNamalskEventManagerModule: JMRenderableModuleBase
{
	protected Class m_EventManager;
	autoptr array<string> Events = new array<string>();
	int MaxEventCount;

	void JMNamalskEventManagerModule()
	{
		//! Just use a hardcoded list and be done with it
		TStringArray evts = {"Aurora", "Blizzard", "ExtremeCold", "Snowfall", "EVRStorm", "EVRStormDeadly", "HeavyFog"};
		foreach (string evt: evts)
		{
			if (!evt.ToType())
				continue;

			Events.Insert(evt);
		}
	}

	bool IsEventActive(string evt)
	{
		if (!m_EventManager) return false;

		bool active = false;
		g_Script.CallFunction(m_EventManager, "IsEventActive", active, evt.ToType());
		return active;
	}

	//! Runs after the constructor, so Events is already filtered down to the event types this map has.
	override void DeclarePermissions()
	{
		super.DeclarePermissions();

		JMPermissions.Register( JMConstants.PERM_NAMALSK );
		JMPermissions.Register( JMConstants.PERM_NAMALSK_VIEW );

		foreach (string evt: Events)
		{
			JMPermissions.Register( JMConstants.PERM_NAMALSK + "." + evt + ".Start" );
			JMPermissions.Register( JMConstants.PERM_NAMALSK + "." + evt + ".Cancel" );
		}
	}

	override void EnableUpdate()
	{
	}

	override bool HasButton()
	{
		return true; // module isn't loaded if namalsk isn't loaded anyways
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "Namalsk";
		info.WebhookTitle = "Namalsk Events Module";
		info.Icon = "radiation";
		info.Layout = "JM/COT/GUI/layouts/eventspawner_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_EVENTS;
		info.ViewPermission = JMConstants.PERM_NAMALSK_VIEW;
		info.SetRPCRange( JMNamalskEventManagerRPC.INVALID, JMNamalskEventManagerRPC.COUNT );

		info.AddWebhookType( "StartEvent" );
		info.AddWebhookType( "CancelEvent" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
		{
			RetrievePossibleEvents();
		}
		else if (JMPermissions.Has(JMConstants.PERM_NAMALSK))
		{
			RequestEvents();
		}
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

		super.OnClientPermissionsUpdated();

		if (Events.Count() > 0) return;

		if (JMPermissions.Has(JMConstants.PERM_NAMALSK))
		{
			RequestEvents();
		}
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		JMPlayerInstance instance;

		string evt;

		switch (rpc_type)
		{
			case JMNamalskEventManagerRPC.StartEvent:
			{
				if (!ctx.Read(evt)) return;

				if (!JMPermissions.HasRPC("Namalsk." + evt + ".Start", sender, instance)) return;

				GetCommunityOnlineToolsBase().Log( sender, "Started Namalsk event: " + evt );
				SendWebhookColored( "StartEvent", instance, "Started Namalsk event: " + evt, JMConstants.WEBHOOK_COLOR_WARNING );

				StartEvent(evt);

				return;
			}

			case JMNamalskEventManagerRPC.CancelEvent:
			{
				if (!ctx.Read(evt)) return;

				if (!JMPermissions.HasRPC("Namalsk." + evt + ".Cancel", sender, instance)) return;

				GetCommunityOnlineToolsBase().Log( sender, "Cancelled Namalsk event: " + evt );
				SendWebhookColored( "CancelEvent", instance, "Cancelled Namalsk event: " + evt, JMConstants.WEBHOOK_COLOR_WARNING );

				CancelEvent(evt);

				return;
			}
			
			case JMNamalskEventManagerRPC.LoadEvents:
			{
				auto trace1 = CF_Trace_0(this, "OnRPC - LoadEvents");

				if (!g_Game.IsClient()) return;

				array<string> evts;
				if (!ctx.Read(evts)) return;
				Events.Copy(evts);

				if (!ctx.Read(MaxEventCount)) return;

				OnSettingsUpdated();
				
				break;
			}
			
			case JMNamalskEventManagerRPC.RequestEvents:
			{
				auto trace2 = CF_Trace_0(this, "OnRPC - RequestEvents");

				if (!sender) return;

				if (!JMPermissions.HasRPC("Namalsk", sender, instance)) return;

				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(Events);
				rpc.Write(MaxEventCount);
				rpc.Send(NULL, JMNamalskEventManagerRPC.LoadEvents, true, NULL);

				break;
			}
		}
	}

	void RetrievePossibleEvents()
	{
		auto trace = CF_Trace_0(this, "RetrievePossibleEvents");

		g_Script.CallFunction(g_Game.GetMission(), "GetNamEventManager", m_EventManager, null);
		Print("m_EventManager " + m_EventManager);
		if (!m_EventManager)
		{
			Error("[JMNamalskEventManagerModule] RetrievePossibleEvents failed: m_EventManager is null!");
			return;
		}
		
		//! Always returns empty, but we want all possible event types (also unregistered ones) anyway...
		//map<typename, float> possibleEvents;
		//EnScript.GetClassVar(m_EventManager, "m_PossibleEventTypes", 0, possibleEvents);
		//foreach (typename type, float duration: possibleEvents)
		//{
			//Print("Event type " + type + " duration " + duration);
		//}

		EnScript.GetClassVar(m_EventManager, "m_MaxEventCount", 0, MaxEventCount);
		Print("MaxEventCount " + MaxEventCount);
	}

	//! Host-direct/RPC split, same shape as JMBanModule.Ban()/Unban(): a client
	//! sends the RPC, which OnRPC re-validates and then calls StartEvent()
	//! below; the mission host calls straight through since it already IS the
	//! authority the RPC path would otherwise be asking permission of.
	void RequestStartEvent(string evt)
	{
		if (IsMissionHost())
		{
			PlayerIdentity hostIdent = NULL;
			GetCommunityOnlineToolsBase().Log(hostIdent, "Started Namalsk event: " + evt);
			SendWebhookColored("StartEvent", NULL, "Started Namalsk event: " + evt, JMConstants.WEBHOOK_COLOR_WARNING);
			StartEvent(evt);
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(evt);
			rpc.Send(NULL, JMNamalskEventManagerRPC.StartEvent, true, NULL);
		}
	}

	void RequestCancelEvent(string evt)
	{
		if (IsMissionHost())
		{
			PlayerIdentity hostIdent = NULL;
			GetCommunityOnlineToolsBase().Log(hostIdent, "Cancelled Namalsk event: " + evt);
			SendWebhookColored("CancelEvent", NULL, "Cancelled Namalsk event: " + evt, JMConstants.WEBHOOK_COLOR_WARNING);
			CancelEvent(evt);
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(evt);
			rpc.Send(NULL, JMNamalskEventManagerRPC.CancelEvent, true, NULL);
		}
	}

	protected void StartEvent(string evt)
	{
		auto trace = CF_Trace_0(this, "StartEvent");

		if (!m_EventManager)
		{
			Error("[JMNamalskEventManagerModule] StartEvent failed: m_EventManager is null!");
			return;
		}

		Param2<typename, bool> parms = new Param2<typename, bool>(evt.ToType(), true);
		g_Script.CallFunctionParams(m_EventManager, "StartEvent", null, parms);
	}

	protected void CancelEvent(string evt)
	{
		auto trace = CF_Trace_0(this, "CancelEvent");

		if (!m_EventManager)
		{
			Error("[JMNamalskEventManagerModule] CancelEvent failed: m_EventManager is null!");
			return;
		}

		g_Script.CallFunction(m_EventManager, "CancelEvent", null, evt.ToType());
	}

	void RequestEvents()
	{
		auto trace = CF_Trace_0(this, "RequestEvents");

		if (IsMissionClient() && !IsMissionOffline())
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, JMNamalskEventManagerRPC.RequestEvents, true, NULL);
		}
		else
		{
			OnSettingsUpdated();
		}
	}
}
