//! Base class for a tab's controller - the object that owns one tab's widgets and state.
//! JMFormBase drives OnCreate (first select), OnFocus, OnUpdate (only while focused), OnUnfocus,
//! OnResize, OnDestruct. Members are prefixed m_Tab* (subclasses keep their own typed `m_Form`).
//! A tab with no controller class overrides the JMFormBase.OnTab* hooks instead.
class JMFormTab
{
	protected JMFormBase m_TabForm;
	protected int m_TabId = -1;
	protected Widget m_TabPanel;
	protected bool m_TabFocused;
	protected bool m_TabEnabled = true;

	JMFormBase GetForm()
	{
		return m_TabForm;
	}

	Widget GetPanel()
	{
		return m_TabPanel;
	}

	//! This tab's index in its form's tab strip. The form assigns it when the tab is
	//! registered (RegisterTab, or AddTab( ..., controller )) - a tab never picks its own number, so
	//! two mods adding tabs to the same form can never collide. -1 until registered.
	int GetTabId()
	{
		return m_TabId;
	}

	bool IsEnabled()
	{
		return m_TabEnabled;
	}

	bool IsFocused()
	{
		return m_TabFocused;
	}

	void SetEnabled( bool enabled )
	{
		m_TabEnabled = enabled;
	}

	void SetFocused( bool focused )
	{
		if ( m_TabFocused == focused )
			return;

		m_TabFocused = focused;

		if ( focused )
			OnFocus();
		else
			OnUnfocus();
	}

	//! Called by JMFormBase.RegisterTab; not needed in a subclass constructor.
	void AttachToForm( JMFormBase form, int tabId )
	{
		m_TabForm = form;
		m_TabId = tabId;
	}

	//! Build this tab's widgets into `panel`. Called once.
	void OnCreate( Widget panel )
	{
		m_TabPanel = panel;
	}

	//! Repaint from current state. Only called while the tab is focused.
	void OnUpdate()
	{
	}

	void OnFocus()
	{
	}

	void OnUnfocus()
	{
	}

	void OnResize( float w, float h )
	{
	}

	void OnDestruct()
	{
	}
}
