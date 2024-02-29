class JMNamalskEventManagerModule: JMRenderableModuleBase
{
	private Class m_EventManager;

	autoptr array<string> Events = new array<string>();
	int MaxEventCount;
	
	void JMNamalskEventManagerModule()
	{
		GetPermissionsManager().RegisterPermission( "Namalsk" );
		GetPermissionsManager().RegisterPermission( "Namalsk.View" );

		//! Just use a hardcoded list and be done with it
		TStringArray evts = {"Aurora", "Blizzard", "ExtremeCold", "Snowfall", "EVRStorm", "EVRStormDeadly", "HeavyFog"};
		foreach (string evt: evts)
		{
			if (!evt.ToType())
				continue;

			Events.Insert(evt);

			GetPermissionsManager().RegisterPermission("Namalsk." + evt + ".Start");
			GetPermissionsManager().RegisterPermission("Namalsk." + evt + ".Cancel");
		}
	}

	override void EnableUpdate()
	{
	}

	override bool HasButton()
	{
		return true; // module isn't loaded if namalsk isn't loaded anyways
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Namalsk.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/eventspawner_form.layout";
	}
	
	override string GetTitle()
	{
		return "Namalsk";
	}
		
	override string GetIconName()
	{
		return "N";
	}
	
	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (GetGame().IsServer())
		{
			RetrievePossibleEvents();
		}
		else if (GetPermissionsManager().HasPermission("Namalsk"))
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

		if (GetPermissionsManager().HasPermission("Namalsk"))
		{
			RequestEvents();
		}
	}
	
	override int GetRPCMin()
	{
		return JMNamalskEventManagerRPC.INVALID;
	}
	
	override int GetRPCMax()
	{
		return JMNamalskEventManagerRPC.COUNT;
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

				if (!GetPermissionsManager().HasPermission("Namalsk." + evt + ".Start", sender, instance)) return;
				
				StartEvent(evt);

				return;
			}

			case JMNamalskEventManagerRPC.CancelEvent:
			{
				if (!ctx.Read(evt)) return;

				if (!GetPermissionsManager().HasPermission("Namalsk." + evt + ".Cancel", sender, instance)) return;
				
				CancelEvent(evt);

				return;
			}
			
			case JMNamalskEventManagerRPC.LoadEvents:
			{
				auto trace1 = CF_Trace_0(this, "OnRPC - LoadEvents");

				if (!GetGame().IsClient()) return;

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

				if (!GetPermissionsManager().HasPermission("Namalsk", sender, instance)) return;

				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(Events);
				rpc.Write(MaxEventCount);
				rpc.Send(NULL, JMNamalskEventManagerRPC.LoadEvents, true, NULL);

				break;
			}
		}
	}

	bool IsEventActive(string evt)
	{
		if (!m_EventManager) return false;

		bool active = false;
		g_Script.CallFunction(m_EventManager, "IsEventActive", active, evt.ToType());
		return active;
	}

	void RetrievePossibleEvents()
	{
		auto trace = CF_Trace_0(this, "RetrievePossibleEvents");

		g_Script.CallFunction(GetGame().GetMission(), "GetNamEventManager", m_EventManager, null);
		Print("m_EventManager " + m_EventManager);
		if (!m_EventManager) return;
		
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

	void StartEvent(string evt)
	{
		auto trace = CF_Trace_0(this, "StartEvent");

		if (!m_EventManager) return;

		Param2<typename, bool> parms = new Param2<typename, bool>(evt.ToType(), true);
		g_Script.CallFunctionParams(m_EventManager, "StartEvent", null, parms);
	}

	void CancelEvent(string evt)
	{
		auto trace = CF_Trace_0(this, "CancelEvent");

		if (!m_EventManager) return;

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
};
