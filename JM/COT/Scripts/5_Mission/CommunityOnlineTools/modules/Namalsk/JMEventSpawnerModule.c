
enum JMEventSpawnerRPC
{
	INVALID = 10400,
	StartEvent,
	ShitPants,
	ShitPantsServer,
	COUNT
}

class JMEventSpawnerModule: JMRenderableModuleBase
{
	
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
	
	override int GetRPCMin()
	{
		return JMEventSpawnerRPC.INVALID;
	}
	
	override int GetRPCMax()
	{
		return JMEventSpawnerRPC.COUNT;
	}
	
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch (rpc_type) {
			
			case JMEventSpawnerRPC.StartEvent: {
			
				string event_param;
				if (!ctx.Read(event_param)) {
					break;
				}
				
				Class event_manager;
				EnScript.GetClassVar(GetGame().GetMission(), "m_EventManagerServer", 0, event_manager);
				if (event_manager) {
					g_Script.CallFunction(event_manager, "StartEventServer", null, event_param.ToType());
				}
				
				break;
			}
			
			
			case JMEventSpawnerRPC.ShitPants: {
				
				if (GetGame().IsClient()) {
					thread ShitPants();
				}
				
				break;
			}
			
			case JMEventSpawnerRPC.ShitPantsServer: {
				
				GetGame().RPCSingleParam(null, JMEventSpawnerRPC.ShitPants, null, true);
				break;
			}
		}
	}
	
	void ShitPants()
	{
		EVRStorm storm = new EVRStorm();
		storm.CreateBlowoutClient(1);
		LerpColorization(0, 1, 1, 1, 1000, 0.5, 0.15, 0.15);
		Sleep(10000);
		delete storm;
	}
}
