//#ifdef NAMALSK_SURVIVAL
class JMEventSpawnerForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ActionsWrapper;
	private JMEventSpawnerModule m_Module;	
	private ref array<ref UIActionButton> m_EventButtons = {};
	
	static const ref array<string> EVENT_TYPES = {
		"Snowfall",
		"Blizzard",
		"ExtremeCold",
		"Aurora",
		"EVRStorm"
	};
	
	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

		m_sclr_MainActions.UpdateScroller();
	}
	
	protected override bool SetModule(ref JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}
	
	override void OnShow()
	{
		foreach (string event_name: EVENT_TYPES) {
			Widget wrapper = UIActionManager.CreatePanel(m_ActionsWrapper, 0x00000000, 30);
			UIActionBase name = UIActionManager.CreateText(wrapper, event_name);
			name.SetWidth(0.8);
			
			UIActionButton button = UIActionManager.CreateButton(wrapper, "Start", this, "BeginEvent");
			button.SetData(new JMEventSpawnerButtonData(event_name));
			button.SetWidth(0.2);
			button.SetPosition(0.8);
	
			m_EventButtons.Insert(button);
		}
		
		m_sclr_MainActions.UpdateScroller();
	}
	
	void BeginEvent(UIEvent eid, ref UIActionBase action)
	{
		JMEventSpawnerButtonData data = action.GetData();
		if (!data) return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.ClassName);
		rpc.Send(null, JMEventSpawnerRPC.StartEvent, true, null);
	}
}
//#endif