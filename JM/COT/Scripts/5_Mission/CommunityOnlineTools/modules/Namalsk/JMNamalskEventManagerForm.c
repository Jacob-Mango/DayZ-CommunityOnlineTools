class JMNamalskEventManagerForm: JMFormBase
{
	protected UIActionScroller m_sclr_MainActions;
	protected Widget m_ContentWrapper;
	protected Widget m_ActionsWrapper;
	//! protected, not private: sub-mods reach for the module through the form.
	protected JMNamalskEventManagerModule m_Module;

	override void OnCreate()
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

	protected void Rebuild()
	{
		if (m_ActionsWrapper)
			m_ActionsWrapper.Unlink();

		m_ActionsWrapper = UIActionManager.CreateGridSpacer(m_ContentWrapper, 1, 1);

		if (!m_Module || !m_Module.Events || m_Module.Events.Count() < 1)
		{
			UIActionManager.CreateText(m_ActionsWrapper, "#STR_COT_NAMALSK_NO_EVENTS_AVAILABLE");
			m_sclr_MainActions.UpdateScroller();
			return;
		}

		foreach (string event_name: m_Module.Events)
			AddEvent(event_name);

		m_sclr_MainActions.UpdateScroller();
	}

	protected void AddEvent(string event_name)
	{
		bool canStart  = JMPermissions.Has(JMConstants.PERM_NAMALSK + "." + event_name + ".Start");
		bool canCancel = JMPermissions.Has(JMConstants.PERM_NAMALSK + "." + event_name + ".Cancel");

		Widget row = UIActionManager.CreateWrapSpacer(m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

		UIActionText nameField = UIActionManager.CreateText(row, "", event_name);
		nameField.SetWidth(0.60);
		nameField.SetTextVAlign(UIActionVAlign.CENTER);

		if (!canStart && !canCancel)
		{
			UIActionText permField = UIActionManager.CreateText(row, "", "#STR_COT_NAMALSK_NO_PERMISSION");
			permField.SetWidth(0.40);
			permField.SetTextVAlign(UIActionVAlign.CENTER);
			return;
		}

		if (canCancel)
		{
			UIActionConfirmInline cancelBtn = UIActionManager.CreateDeleteConfirmIcon( row, this, "OnClick_CancelEvent" );
			cancelBtn.SetData(new JMNamalskEventManagerButtonData(event_name));
			cancelBtn.SetTooltip("#STR_COT_NAMALSK_CANCEL_THIS_EVENT");
		}

		if (canStart)
		{
			UIActionButton startBtn = UIActionManager.CreateButton(row, "#STR_COT_NAMALSK_START_EVENT", this, "OnClick_StartEvent");
			startBtn.SetWidth(0.40);
			startBtn.SetColor(JMTheme.SUCCESS_FILL);
			startBtn.SetData(new JMNamalskEventManagerButtonData(event_name));
			startBtn.SetTooltip("#STR_COT_NAMALSK_START_THIS_EVENT");
		}
	}

	void OnClick_StartEvent(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module)
			return;

		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData()))
			return;

		m_Module.RequestStartEvent(data.ClassName);
	}

	void OnClick_CancelEvent(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE || !m_Module)
			return;

		JMNamalskEventManagerButtonData data;
		if (!Class.CastTo(data, action.GetData()))
			return;

		m_Module.RequestCancelEvent(data.ClassName);
	}
}
