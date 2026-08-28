class JMNamalskEventManagerForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ContentWrapper;
	private Widget m_ActionsWrapper;
	//! protected, not private: sub-mods reach for the module through the form.
	protected JMNamalskEventManagerModule m_Module;

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller(layoutRoot.FindAnyWidget("panel"));
		m_ContentWrapper = m_sclr_MainActions.GetContentWidget();
		m_sclr_MainActions.UpdateScroller();
	}

	override void OnResize(float w, float h)
	{
		if (m_sclr_MainActions)
			m_sclr_MainActions.UpdateScroller();
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnShow()
	{
		Rebuild();
	}

	private void Rebuild()
	{
		if (m_ActionsWrapper)
			delete m_ActionsWrapper;

		m_ActionsWrapper = UIActionManager.CreateGridSpacer(m_ContentWrapper, 1, 1);

		if (!m_Module || !m_Module.Events || m_Module.Events.Count() < 1)
		{
			UIActionManager.CreateText(m_ActionsWrapper, "No Namalsk events available");
			m_sclr_MainActions.UpdateScroller();
			return;
		}

		foreach (string event_name: m_Module.Events)
			AddEvent(event_name);

		m_sclr_MainActions.UpdateScroller();
	}

	private void AddEvent(string event_name)
	{
		bool canStart  = GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Start");
		bool canCancel = GetPermissionsManager().HasPermission("Namalsk." + event_name + ".Cancel");

		Widget row = UIActionManager.CreateWrapSpacer(m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

		UIActionText nameField = UIActionManager.CreateText(row, "", event_name);
		nameField.SetWidth(0.60);
		nameField.SetTextVAlign(UIActionVAlign.CENTER);

		if (!canStart && !canCancel)
		{
			UIActionText permField = UIActionManager.CreateText(row, "", "No Permission");
			permField.SetWidth(0.40);
			permField.SetTextVAlign(UIActionVAlign.CENTER);
			return;
		}

		if (canCancel)
		{
			UIActionConfirmInline cancelBtn = UIActionManager.CreateConfirmInline(row, "", this, "OnClick_CancelEvent");
			UIActionIconGrid.ApplyDeletePreset(cancelBtn);
			cancelBtn.SetButton("");
			cancelBtn.SetFixedSize(32, 32);
			cancelBtn.CenterIcon(32, 16);
			cancelBtn.SetConfirmLabel("O");
			cancelBtn.SetCancelLabel("X");
			cancelBtn.SetData(new JMNamalskEventManagerButtonData(event_name));
			cancelBtn.SetTooltip("Cancel this event");
		}

		if (canStart)
		{
			UIActionButton startBtn = UIActionManager.CreateButton(row, "Start", this, "OnClick_StartEvent");
			startBtn.SetWidth(0.40);
			startBtn.SetColor(JMTheme.SUCCESS_FILL);
			startBtn.SetData(new JMNamalskEventManagerButtonData(event_name));
			startBtn.SetTooltip("Start this event");
		}
	}

	void OnClick_StartEvent(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData()))
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.ClassName);
		rpc.Send(null, JMNamalskEventManagerRPC.StartEvent, true, null);
	}

	void OnClick_CancelEvent(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData()))
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data.ClassName);
		rpc.Send(null, JMNamalskEventManagerRPC.CancelEvent, true, null);
	}
}
