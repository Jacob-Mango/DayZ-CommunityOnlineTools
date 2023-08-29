class JMNamalskEventManagerForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;

	private Widget m_ActionsWrapper;
	private JMNamalskEventManagerModule m_Module;
	
	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller(layoutRoot.FindAnyWidget("panel"));
		m_ActionsWrapper = m_sclr_MainActions.GetContentWidget();

		m_sclr_MainActions.UpdateScroller();
	}
	
	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}
	
	private void AddSimpleButton(Widget wrapper, string event_name, string button_text, string function, float position = 0.6)
	{
		UIActionButton button = UIActionManager.CreateButton(wrapper, button_text, this, function);
		button.SetData(new JMNamalskEventManagerButtonData(event_name));
		button.SetWidth(0.2);
		button.SetPosition(position);
	}

	private void AddSimpleText(Widget wrapper, string field_text)
	{
		UIActionBase field = UIActionManager.CreateText(wrapper, field_text);
		field.SetWidth(0.4);
		field.SetPosition(0.6);
	}
	
	private void AddEvent(string event_name)
	{
		Widget wrapper = UIActionManager.CreatePanel(m_ActionsWrapper, 0x00000000, 30);
		UIActionBase name = UIActionManager.CreateText(wrapper, event_name);
		name.SetWidth(0.8);

		bool canStart = GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Start");
		bool canCancel = GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Cancel");

		if (canStart)
			AddSimpleButton(wrapper, event_name, "Start", "StartEvent");

		if (canCancel)
			AddSimpleButton(wrapper, event_name, "Cancel", "CancelEvent", 0.8);

		if (!canStart && !canCancel)
			AddSimpleText(wrapper, "No Permission");
	}
	
	override void OnShow()
	{
		foreach (string event_name: m_Module.Events)
		{
			AddEvent(event_name);
		}
		
		m_sclr_MainActions.UpdateScroller();
	}
	
	void StartEvent(UIEvent eid, UIActionBase action)
	{
		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData())) return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.ClassName);
		rpc.Send(null, JMNamalskEventManagerRPC.StartEvent, true, null);
	}
	
	void CancelEvent(UIEvent eid, UIActionBase action)
	{
		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData())) return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.ClassName);
		rpc.Send(null, JMNamalskEventManagerRPC.CancelEvent, true, null);
	}
};
