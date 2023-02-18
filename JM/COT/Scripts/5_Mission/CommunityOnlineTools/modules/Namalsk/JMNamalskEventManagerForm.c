
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
	
	private void AddSimpleButton(string text, string button_text, string function)
	{
		Widget wrapper = UIActionManager.CreatePanel(m_ActionsWrapper, 0x00000000, 30);
		UIActionBase name = UIActionManager.CreateText(wrapper, text);
		name.SetWidth(0.8);
		
		UIActionButton button = UIActionManager.CreateButton(wrapper, button_text, this, function);
		button.SetData(new JMNamalskEventManagerButtonData(text));
		button.SetWidth(0.2);
		button.SetPosition(0.8);
	}

	private void AddSimpleText(string text, string field_text)
	{
		Widget wrapper = UIActionManager.CreatePanel(m_ActionsWrapper, 0x00000000, 30);
		UIActionBase name = UIActionManager.CreateText(wrapper, text);
		name.SetWidth(0.6);
		
		UIActionBase field = UIActionManager.CreateText(wrapper, field_text);
		field.SetWidth(0.4);
		field.SetPosition(0.6);
	}
	
	override void OnShow()
	{
		foreach (string event_name: m_Module.Events)
		{
			if (m_Module.IsEventActive(event_name))
			{
				if (GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Cancel"))
				{
					AddSimpleButton(event_name, "Cancel", "CancelEvent");
					continue;
				}

				AddSimpleText(event_name, "(C) No Permission");
				continue;
			} 

			if (!m_Module.IsEventActive(event_name))
			{
				if (GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Start"))
				{
					AddSimpleButton(event_name, "Start", "StartEvent");
					continue;
				}

				AddSimpleText(event_name, "(S) No Permission");
				continue;
			}

			if (GetPermissionsManager().HasPermission("Namalsk." + event_name))
			{
				AddSimpleText(event_name, "Invalid Permission");
				continue;
			}
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