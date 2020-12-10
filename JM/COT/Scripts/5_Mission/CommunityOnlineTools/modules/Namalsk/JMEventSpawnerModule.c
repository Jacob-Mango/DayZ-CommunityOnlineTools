//#ifdef NAMALSK_SURVIVAL
enum JMEventSpawnerRPC
{
	INVALID = 10400,
	StartEvent,
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
			
		}
	}
}
//#endif