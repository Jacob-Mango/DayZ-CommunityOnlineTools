
enum JMNamalskEventManagerRPC
{
	INVALID = 10400,
	LoadEvents,
	RequestEvents,
	StartEvent,
	CancelEvent,
	COUNT
};

[CF_RegisterModule(JMNamalskEventManagerModule)]
class JMNamalskEventManagerModule: JMRenderableModuleBase
{
	private Class m_EventManager;

	autoptr array<string> Events = new array<string>();
	bool AllowMultipleEvents;
	
	override void OnInit()
	{
		super.OnInit();

		EnableMissionLoaded();
		EnableRPC();

		GetPermissionsManager().RegisterPermission( "Namalsk" );
		GetPermissionsManager().RegisterPermission( "Namalsk.View" );
	}

	override bool HasButton()
	{
		return true;
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
	
	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		if (GetGame().IsServer())
		{
			RetrievePossibleEvents();
		}
		else if (GetPermissionsManager().HasPermission("Namalsk"))
		{
			RequestEvents();
		}
	}
	
	override void OnPermissionsChanged(Class sender, CF_EventArgs args)
	{
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

	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto rpcArgs = CF_EventRPCArgs.Cast(args);

		JMPlayerInstance instance;
		string evt;

		switch (rpcArgs.ID)
		{
			case JMNamalskEventManagerRPC.StartEvent:
			{
				if (!rpcArgs.Context.Read(evt)) return;

				if (!GetPermissionsManager().HasPermission("Namalsk." + evt + ".Start", rpcArgs.Sender, instance)) return;
				
				StartEvent(evt);

				return;
			}

			case JMNamalskEventManagerRPC.CancelEvent:
			{
				if (!rpcArgs.Context.Read(evt)) return;

				if (!GetPermissionsManager().HasPermission("Namalsk." + evt + ".Cancel", rpcArgs.Sender, instance)) return;
				
				CancelEvent(evt);

				return;
			}
			
			case JMNamalskEventManagerRPC.LoadEvents:
			{
				if (!GetGame().IsClient()) return;

				array<string> evts;
				if (!rpcArgs.Context.Read(evts)) return;
				Events.Copy(evts);

				if (!rpcArgs.Context.Read(AllowMultipleEvents)) return;

				OnSettingsChanged(this, new CF_EventArgs);
				
				break;
			}
			
			case JMNamalskEventManagerRPC.RequestEvents:
			{
				if (!GetPermissionsManager().HasPermission("Namalsk", rpcArgs.Sender, instance)) return;

				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(Events);
				rpc.Write(AllowMultipleEvents);
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
		g_Script.CallFunction(GetGame().GetMission(), "GetEventManager", m_EventManager, null);
		if (!m_EventManager) return;
		
		map<typename, float> possibleEvents;
		EnScript.GetClassVar(m_EventManager, "m_PossibleEventTypes", 0, possibleEvents);

		EnScript.GetClassVar(m_EventManager, "m_AllowMultipleEvents", 0, AllowMultipleEvents);

		Events.Clear();

		array<typename> types = possibleEvents.GetKeyArray();
		for (int i = 0; i < types.Count(); i++)
		{
			string evt = types[i].ToString();
			Events.Insert(evt);

			GetPermissionsManager().RegisterPermission("Namalsk." + evt + ".Start");
			GetPermissionsManager().RegisterPermission("Namalsk." + evt + ".Cancel");
		}
	}

	void StartEvent(string evt)
	{
		if (!m_EventManager) return;

		g_Script.CallFunction(m_EventManager, "StartEvent", null, evt.ToType());
	}

	void CancelEvent(string evt)
	{
		if (!m_EventManager) return;

		g_Script.CallFunction(m_EventManager, "CancelEvent", null, evt.ToType());
	}
	
	void RequestEvents()
	{
		if (IsMissionClient() && !IsMissionOffline())
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, JMNamalskEventManagerRPC.RequestEvents, true, NULL);
		}
		else
		{
			OnSettingsChanged(this, new CF_EventArgs);
		}
	}
};
