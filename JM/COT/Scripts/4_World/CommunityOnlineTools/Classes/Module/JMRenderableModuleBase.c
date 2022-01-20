class JMRenderableModuleBase extends COTModule
{
	JMFormBase m_Form;

	override JMFormBase GetForm()
	{
		return m_Form;
	}

	JMFormBase InitForm(Widget root)
	{
		Error("Not implemented.");
		return NULL;
	}

	override void Show()
	{
		if (!HasAccess())
		{
			return;
		}

		m_Window = new CF_Window();

		Widget widgets = m_Window.CreateWidgets(GetLayoutRoot());

		widgets.GetScript(m_Form);

		if (m_Form)
		{
			m_Form.Init(m_Window, this);
		}
	}
};
